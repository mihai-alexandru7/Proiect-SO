#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 3
#define COMMAND_FILE "monitor_command.txt"
#define TREASURE_MANAGER_PATH "./bin/treasure_manager"

pid_t monitor_pid = -1;
int monitor_running = 0;
int waiting_for_monitor_exit = 0;

// Monitor process signal handling
void monitor_handler(int signal) {
  // Monitor received command signal from hub
  if (signal == SIGUSR1) {
    // Open the command file
    int cmd_fd = open(COMMAND_FILE, O_RDONLY);
    if (cmd_fd == -1) {
      perror("MONITOR: Error opening the command file");
      return;
    }
    // Read the command from the command file
    char cmd_buffer[MAX_COMMAND_LENGTH];
    ssize_t bytes_read = read(cmd_fd, cmd_buffer, sizeof(cmd_buffer) - 1);
    // Close the command file
    if (close(cmd_fd) == -1) {
      perror("MONITOR: Error closing the command file");
    }
    // Remove the command file after closing
    if (unlink(COMMAND_FILE) == -1) {
      perror("MONITOR: Error removing the command file");
    }
    // Check if the command was read corectly
    if (bytes_read <= 0) {
      perror("MONITOR: Error reading from the command file");
      return;
    }
    cmd_buffer[bytes_read] = '\0';
    // Parse the command into args
    char* args[MAX_ARGS + 2];
    int i = 0;
    args[i++] = TREASURE_MANAGER_PATH;
    char *arg = strtok(cmd_buffer, ";");
    while(arg != NULL && i < (MAX_ARGS + 1)) {
      args[i++] = arg;
      arg = strtok(NULL, ";");
    }
    args[i] = NULL;
    if (i < 2) {
      fprintf(stderr, "Invalid command format received in monitor process\n");
      return;
    }
    // Create and execute the manager process
    pid_t manager_pid = fork();
    if (manager_pid == -1) {
      perror("MONITOR: Error creating the manager process");
      return;
    }
    // Child process (manager)
    if (manager_pid == 0) {
      execvp(args[0], args);
      perror("MONITOR: Error executing the manager process");
      exit(EXIT_FAILURE);
    }
    // Parent process (monitor) waits for child process (manager)
    printf("MONITOR: Manager process %d started successfully\n", manager_pid);
    int status;
    if (waitpid(manager_pid, &status, 0) >  0) {
      if(WIFEXITED(status)) {
        printf("MONITOR: Manager process %d ended with code %d\n", manager_pid, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("MONITOR: Manager process terminated by signal: %d\n", WTERMSIG(status));
      } else {
        printf("MONITOR: Manager process %d ended abnormally\n", manager_pid);
      }
    }
  } else if (signal == SIGTERM) {
    // Monitor received termination signal from hub
    printf("MONITOR: Received termination signal (SIGTERM)\n");
    printf("MONITOR: Exiting...\n");
    usleep(10000000); // 10 seconds delay
    exit(EXIT_SUCCESS);
  }
}

// Start the monitor process
void start_monitor() {
  if (monitor_running) {
    printf("HUB: Monitor is already running\n");
    return;
  }
  if (waiting_for_monitor_exit) {
    printf("HUB: Cannot start monitor, previous instance is still shutting down\n");
    return;
  }
  // Create child process (monitor)
  monitor_pid = fork();
  if (monitor_pid == -1) {
    perror("HUB: Error creating the monitor process");
    return;
  }
  // Child process (monitor)
  if (monitor_pid == 0) {
    struct sigaction sa_monitor;
    sa_monitor.sa_handler = monitor_handler;
    sigemptyset(&sa_monitor.sa_mask);
    sa_monitor.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa_monitor, NULL) == -1) {
      perror("MONITOR: Error setting up SIGUSR1 handler");
      exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa_monitor, NULL) == -1) {
      perror("MONITOR: Error setting up SIGTERM handler");
      exit(EXIT_FAILURE);
    }
    while (1) {
      pause();
    }
    exit(EXIT_SUCCESS);
  }
  // Parent process (hub)
  monitor_running = 1;
  waiting_for_monitor_exit = 0;
  printf("HUB: Monitor process %d started successfully\n", monitor_pid);
}

// Stop the monitor process
void stop_monitor() {
  if (!monitor_running) {
    printf("HUB: Monitor isn't running\n");
    return;
  }
  if (waiting_for_monitor_exit) {
    printf("HUB: Already waiting for monitor %d to exit\n", monitor_pid);
    return;
  }
  // Send SIGTERM to monitor
  if (kill(monitor_pid, SIGTERM) == -1) {
    perror("HUB: Error sending SIGTERM to monitor");
    return;
  }
  waiting_for_monitor_exit = 1;
  printf("HUB: Waiting for monitor process to terminate...\n");
}

// Hub process signal handling
void hub_handler(int signal) {
  if (signal == SIGCHLD) {
    // Parent process (hub) waits for child process (monitor)
    int status;
    if (waitpid(monitor_pid, &status, 0) > 0) {
      if(WIFEXITED(status)) {
        printf("HUB: Monitor process %d ended with code %d\n", monitor_pid, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("HUB: Monitor process terminated by signal: %d\n", WTERMSIG(status));
      } else {
        printf("HUB: Monitor process %d ended abnormally\n", monitor_pid);
      }
    }
    // Reset monitor state
    monitor_pid = -1;
    monitor_running = 0;
    waiting_for_monitor_exit = 0;
  }
}

// Send a command to the running monitor process
void send_command_to_monitor(const char* cmd, const char* arg1, const char* arg2) {
  if (!monitor_running) {
    printf("HUB: Monitor isn't running\n");
    return;
  }
  if (waiting_for_monitor_exit) {
    printf("HUB: Monitor is shutting down\n");
    return;
  }
  // Open the command file
  int cmd_fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (cmd_fd == -1) {
    perror("HUB: Error opening the command file");
    return;
  }
  // Format the command
  char cmd_buffer[MAX_COMMAND_LENGTH];
  if (arg1 && arg2) {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "--%s;%s;%s", cmd, arg1, arg2);
  } else if (arg1) {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "--%s;%s", cmd, arg1);
  } else {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "--%s", cmd);
  }
  // Write to the command file
  ssize_t bytes_written = write(cmd_fd, cmd_buffer, strlen(cmd_buffer));
  // Close command file
  if (close(cmd_fd) == -1) {
    perror("HUB: Error closing the command file");
  }
  // Check if the command was written correctly
  if (bytes_written != (ssize_t)strlen(cmd_buffer)) {
    perror("HUB: Error writing to the command file");
    if (unlink(COMMAND_FILE) == -1) {
      perror("HUB: Error removing the command file");
    }
    return;
  }
  // Send SIGUSR1 signal to the monitor process
  if (kill(monitor_pid, SIGUSR1) == -1) {
    perror("HUB: Error sending SIGUSR1 signal to the monitor process");
  }
}

int main() {
  struct sigaction sa_hub;
  sa_hub.sa_handler = hub_handler;
  sigemptyset(&sa_hub.sa_mask);
  sa_hub.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa_hub, NULL) == -1) {
    perror("HUB: Error setting up SIGCHLD handler");
    exit(EXIT_FAILURE);
  }
  printf("Welcome to Treasure Hub!\n");
  printf("Type 'help' for a list of available commands\n");
  char cmd[MAX_COMMAND_LENGTH];
  while (1) {
    printf(">> ");
    fflush(stdout);
    if (!fgets(cmd, sizeof(cmd), stdin)) {
      perror("HUB: Error reading the command");
      break;
    }
    cmd[strcspn(cmd, "\n")] = '\0';
    if (strlen(cmd) == 0) {
      continue;
    }
    // Parse the command into args
    char* args[MAX_ARGS + 1];
    int argc = 0;
    char *arg = strtok(cmd, " ");
    while (arg != NULL && argc < (MAX_ARGS + 1)) {
      args[argc++] = arg;
      arg = strtok(NULL, " ");
    }
    args[argc] = NULL;
    char* command = args[0];
    char* hunt_id = (argc > 1) ? args[1] : NULL;
    char* treasure_id = (argc > 2) ? args[2] : NULL;
    if (argc > 3) {
      printf("HUB: Too many arguments for command '%s'\n", command);
      printf("Type 'help' for a list of available commands\n");
      continue;
    }
    if (strcmp(command, "help") == 0) {
      printf("Available commands:\n");
      printf("'exit' to exit the treasure hub\n");
      printf("'start_monitor' to start the monitor process\n");
      printf("'stop_monitor' to stop the monitor process\n");
      printf("'view_treasure <hunt_id> <treasure_id>' to ask the monitor to view a treasure\n");
      printf("'list_hunts' to ask the monitor to list all the hunts\n");
      printf("'list_treasures <hunt_id>' to ask the monitor to list all the treasures in a hunt\n");
    } else if (strcmp(command, "exit") == 0) {
      if (argc > 1) {
        printf("HUB: 'exit' takes no arguments\n");
        continue;
      }
      if (monitor_running) {
        printf("HUB: The monitor process is still running. Please use 'stop_monitor' and wait for it to terminate before using 'exit'\n");
      } else if (waiting_for_monitor_exit) {
        printf("HUB: The monitor process is shutting down. Please wait\n");
      } else {
        printf("HUB: Exiting...\n");
        break;
      }
    } else if (strcmp(command, "start_monitor") == 0) {
      if (argc > 1) {
        printf("HUB: 'start_monitor' takes no arguments\n");
        continue;
      }
      start_monitor();
    } else if (strcmp(command, "stop_monitor") == 0) {
      if (argc > 1) {
        printf("HUB: 'stop_monitor' takes no arguments\n");
        continue;
      }
      stop_monitor();
    } else if (strcmp(command, "view_treasure") == 0) {
      if (hunt_id == NULL || treasure_id == NULL) {
        printf("HUB: 'view_treasure' requires <hunt_id> and <treasure_id>\n");
        continue;
      }
      send_command_to_monitor(command, hunt_id, treasure_id);
    } else if (strcmp(command, "list_treasures") == 0) {
      if (argc > 2) {
        printf("HUB: 'list_treasures' takes only one argument <hunt_id>\n");
        continue;
      }
      if (hunt_id == NULL) {
        printf("HUB: 'list_treasures' requires a <hunt_id>\n");
        continue;
      }
      send_command_to_monitor(command, hunt_id, NULL);
    } else if (strcmp(command, "list_hunts") == 0) {
      if (argc > 1) {
        printf("HUB: 'list_hunts' takes no arguments\n");
        continue;
      }
      send_command_to_monitor(command, NULL, NULL);
    } else {
      printf("HUB: Unknown command: '%s'\n", command);
      printf("Type 'help' for a list of available commands\n");
    }
    usleep(2000000); // 2 seconds delay
  }
  return EXIT_SUCCESS;
}
