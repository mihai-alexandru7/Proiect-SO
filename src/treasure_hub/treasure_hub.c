#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include "../../include/parser.h"

#define BUFFER_SIZE 4096
#define MAX_COMMAND_LENGTH 256
#define COMMAND_FILE "cmd.dat"
#define HUNTS_DIRECTORY "hunts"
#define TREASURE_MONITOR_PATH "./bin/treasure_monitor"
#define TREASURE_MANAGER_PATH "./bin/treasure_manager"
#define CALCULATE_SCORE_PATH "./bin/calculate_score"

pid_t monitor_pid = -1;
pid_t calculate_score_pid = -1;
int monitor_running = 0;
int waiting_for_monitor_exit = 0;
int pipe_fd[2];

// Signal handler for SIGUSR1
void handle_sigusr1() {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  while ((bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE)) > 0) {
    if (write(1, buffer, bytes_read) == -1) {
      perror("Error writing the monitor output to the standard output");
      break;
    }
  }
  if (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
    perror("Error reading from the pipe");
  }
}

// Signal handler for SIGCHLD
void handle_sigchld() {
  // Parent process (hub) reaps the child process
  pid_t pid;
  int status;
  if ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (pid == monitor_pid) {
      if(WIFEXITED(status)) {
        printf("Monitor process %d ended with code %d\n", monitor_pid, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("Monitor process terminated by signal: %d\n", WTERMSIG(status));
      } else {
        printf("Monitor process %d ended abnormally\n", monitor_pid);
      }
      monitor_pid = -1;
      monitor_running = 0;
      waiting_for_monitor_exit = 0;
      if (close(pipe_fd[0]) == -1) {
        perror("Error closing the read end of the pipe");
      }
    } else if (pid == calculate_score_pid) {
      if (WIFEXITED(status)) {
        printf("Calculate_score process %d ended with code %d\n", pid, WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        printf("Calculate_score process terminated by signal: %d\n", WTERMSIG(status));
      } else {
        printf("Calculate_score process %d ended abnormally\n", pid);
      }
    }
  }
}

// Dispatches signal handling to the appropriate handler
void hub_handler(int signal) {
  switch (signal) {
    case SIGUSR1: {
      handle_sigusr1();
      break;
    }
    case SIGCHLD: {
      handle_sigchld();
      break;
    }
  }
}

// Starts the monitor process
void start_monitor() {
  if (waiting_for_monitor_exit) {
    printf("Cannot execute the command: Monitor is currently shutting down\n");
    return;
  }
  if (monitor_running) {
    printf("Monitor is already running\n");
    return;
  }
  // Create the pipe before forking
  if (pipe(pipe_fd) == -1) {
    perror("Error creating the pipe");
    return;
  }
  if (fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK) == -1) {
    perror("Error setting the pipe to non-blocking mode");
  }
  // Create the child process (monitor) using fork
  monitor_pid = fork();
  if (monitor_pid == -1) {
    perror("Error creating the monitor process");
    if (close(pipe_fd[0]) == -1) {
      perror("Error closing the read end of the pipe");
    }
    if (close(pipe_fd[1]) == -1) {
      perror("Error closing the write end if the pipe");
    }
    return;
  }
  // Child process (monitor)
  if (monitor_pid == 0) {
    if (close(pipe_fd[0]) == -1) {
      perror("Error closing the read end of the pipe");
    }
    if (dup2(pipe_fd[1], 1) == -1) {
      perror("Error redirecting the standard output to the write end of the pipe");
    }
    if (close(pipe_fd[1]) == -1) {
      perror("Error closing the write end of the pipe");
    }
    execl(TREASURE_MONITOR_PATH, TREASURE_MONITOR_PATH, NULL);
    perror("Error executing the monitor program");
    exit(EXIT_FAILURE);
  }
  // Parent process (hub)
  monitor_running = 1;
  waiting_for_monitor_exit = 0;
  if (close(pipe_fd[1]) == -1) {
    perror("Error closing the write end of the pipe");
  }
}

// Stops the monitor process
void stop_monitor() {
  if (waiting_for_monitor_exit) {
    printf("Cannot execute the command: Monitor is currently shutting down\n");
    return;
  }
  if (!monitor_running) {
    printf("Monitor isn't running\n");
    return;
  }
  // Send SIGTERM signal to the monitor process
  if (kill(monitor_pid, SIGTERM) == -1) {
    perror("Error sending SIGTERM to the monitor process");
    return;
  }
  waiting_for_monitor_exit = 1;
}

// Sends a command to the running monitor process
void send_command_to_monitor(char** cmd_args) {
  if (waiting_for_monitor_exit) {
    printf("Cannot execute the command: Monitor is currently shutting down\n");
    return;
  }
  if (!monitor_running) {
    printf("Monitor isn't running\n");
    return;
  }
  // Open the command file
  int cmd_fd = open(COMMAND_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (cmd_fd == -1) {
    perror("Error opening the command file");
    return;
  }
  // Format the command
  char cmd_buffer[MAX_COMMAND_LENGTH];
  if (cmd_args[1] && cmd_args[2]) {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s --%s %s %s", TREASURE_MANAGER_PATH, cmd_args[0], cmd_args[1], cmd_args[2]);
  } else if (cmd_args[1]) {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s --%s %s", TREASURE_MANAGER_PATH, cmd_args[0], cmd_args[1]);
  } else {
    snprintf(cmd_buffer, sizeof(cmd_buffer), "%s --%s", TREASURE_MANAGER_PATH, cmd_args[0]);
  }
  // Write to the command file
  ssize_t bytes_written = write(cmd_fd, cmd_buffer, strlen(cmd_buffer));
  // Close the command file
  if (close(cmd_fd) == -1) {
    perror("Error closing the command file");
  }
  // Check if the command was written correctly
  if (bytes_written == -1) {
    perror("Error writing to the command file");
    if (unlink(COMMAND_FILE) == -1) {
      perror("Error removing the command file");
    }
    return;
  }
  // Send SIGUSR1 signal to the monitor process
  if (kill(monitor_pid, SIGUSR1) == -1) {
    perror("Error sending SIGUSR1 signal to the monitor process");
  }
}

//
void calculate_score() {
  // Open the hunts directory
  DIR* dir = opendir(HUNTS_DIRECTORY);
  if (!dir) {
    perror("Error opening the hunts directory");
    return;
  }
  // Calculate user scores for every hunt
  struct dirent* entry;
  while ((entry = readdir(dir))) {
    // Skip "." and ".." entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    // Create the pipe before forking
    int calculate_score_pipe_fd[2];
    if (pipe(calculate_score_pipe_fd) == -1) {
      perror("Error creating the pipe");
      continue;
    }
    // Create the child process (calculate_score)
    calculate_score_pid = fork();
    if (calculate_score_pid == -1) {
      perror("Error creating the calculate_score process");
      if (close(calculate_score_pipe_fd[0])) {
        perror("Error closing the read end of the pipe");
      }
      if (close(calculate_score_pipe_fd[1])) {
        perror("Error closing the write end of the pipe");
      }
      continue;
    }
    // Child process (calculate_score)
    if (calculate_score_pid == 0) {
      if (close(calculate_score_pipe_fd[0]) == -1) {
        perror("Error closing the read end of the pipe");
      }
      if (dup2(calculate_score_pipe_fd[1], 1) == -1) {
        perror("Error redirecting the standard output to the pipe");
      }
      if (close(calculate_score_pipe_fd[1]) == -1) {
        perror("Error closing the write end of the pipe");
      }
      execl(CALCULATE_SCORE_PATH, CALCULATE_SCORE_PATH, entry->d_name, NULL);
      perror("Error executing the calculate_score program");
      exit(EXIT_FAILURE);
    }
    // Parent process (hub)
    printf("Calculate_score process started with pid %d\n", calculate_score_pid);
    if (close(calculate_score_pipe_fd[1]) == -1) {
      perror("Error closing the write end of the pipe");
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(calculate_score_pipe_fd[0], &buffer, BUFFER_SIZE)) > 0) {
      if (write(1, &buffer, bytes_read) == -1) {
        perror("Error writing the calculate_score output to the standard output");
      }
    }
    if (bytes_read == -1) {
      perror("Error reading from the pipe");
    }
    if (close(calculate_score_pipe_fd[0]) == -1) {
      perror("Error closing the read end of the pipe");
    }
  }
  // Close the hunts directory
  if (closedir(dir) == -1) {
    perror("Error closing the hunts directory");
  }
}

int main() {
  printf("Hub process started with pid %d\n", getpid());
  // Set up signal handlers
  struct sigaction sa_hub;
  sa_hub.sa_handler = hub_handler;
  sigemptyset(&sa_hub.sa_mask);
  sa_hub.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &sa_hub, NULL) == -1) {
    perror("Error setting up SIGUSR1 handler");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGCHLD, &sa_hub, NULL) == -1) {
    perror("Error setting up SIGCHLD handler");
    exit(EXIT_FAILURE);
  }
  printf("Type 'help' for a list of available commands\n");
  char cmd[MAX_COMMAND_LENGTH];
  while (1) {
    if (!fgets(cmd, sizeof(cmd), stdin)) {
      perror("Error reading the command from the standard input");
      break;
    }
    cmd[strcspn(cmd, "\n")] = '\0';
    if (strlen(cmd) == 0) {
      continue;
    }
    // Parse the command
    int cmd_argc = 0;
    char** cmd_argv = get_argv(cmd, &cmd_argc);
    if (cmd_argc > 3) {
      printf("Too many arguments for command '%s'\n", cmd_argv[0]);
      printf("Type 'help' for a list of available commands\n");
      continue;
    }
    if (strcmp(cmd_argv[0], "help") == 0) {
      printf("Available commands:\n");
      printf("'exit' to exit the treasure hub\n");
      printf("'start_monitor' to start the monitor process\n");
      printf("'stop_monitor' to stop the monitor process\n");
      printf("'view_treasure <hunt_id> <treasure_id>' to ask the monitor to view a treasure\n");
      printf("'list_hunts' to ask the monitor to list all the hunts\n");
      printf("'list_treasures <hunt_id>' to ask the monitor to list all the treasures in a hunt\n");
      printf("'calculate_score' to calculate the scores of users in each existing hunt\n");
    } else if (strcmp(cmd_argv[0], "exit") == 0) {
      if (cmd_argc > 1) {
        printf("'exit' takes no arguments\n");
      	continue;
      }
      if (waiting_for_monitor_exit) {
        printf("Monitor is shutting down\n");
      } else if (monitor_running) {
	      printf("Monitor is still running. Please use 'stop_monitor' and wait for it to terminate before using 'exit'\n");
      } else {
	      printf("Exiting hub...\n");
        free_argv(cmd_argv, cmd_argc);
        break;
      }
    } else if (strcmp(cmd_argv[0], "start_monitor") == 0) {
      if (cmd_argc > 1) {
        printf("'start_monitor' takes no arguments\n");
        continue;
      }
      start_monitor();
    } else if (strcmp(cmd_argv[0], "stop_monitor") == 0) {
      if (cmd_argc > 1) {
        printf("'stop_monitor' takes no arguments\n");
        continue;
      }
      stop_monitor();
    } else if (strcmp(cmd_argv[0], "view_treasure") == 0) {
      if (!cmd_argv[1] || !cmd_argv[2]) {
	      printf("'view_treasure' requires <hunt_id> and <treasure_id>\n");
	      continue;
      }
      send_command_to_monitor(cmd_argv);
    } else if (strcmp(cmd_argv[0], "list_treasures") == 0) {
      if (cmd_argc > 2) {
	      printf("'list_treasures' takes only one argument <hunt_id>\n");
	      continue;
      }
      if (!cmd_argv[1]) {
	      printf("'list_treasures' requires a <hunt_id>\n");
	      continue;
      }
      send_command_to_monitor(cmd_argv);
    } else if (strcmp(cmd_argv[0], "list_hunts") == 0) {
      if (cmd_argc > 1) {
	      printf("'list_hunts' takes no arguments\n");
	      continue;
      }
      send_command_to_monitor(cmd_argv);
    } else if (strcmp(cmd_argv[0], "calculate_score") == 0) {
      calculate_score();
    } else {
      printf("Unknown command: '%s'\n", cmd_argv[0]);
      printf("Type 'help' for a list of available commands\n");
    }
    free_argv(cmd_argv, cmd_argc);
  }
  return EXIT_SUCCESS;
}