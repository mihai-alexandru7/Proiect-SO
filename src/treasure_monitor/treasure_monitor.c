#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "../../include/parser.h"

#define BUFFER_SIZE 4096
#define MAX_COMMAND_LENGTH 256
#define COMMAND_FILE "cmd.dat"

pid_t manager_pid = -1;
int pipe_fd[2];

// Executes the treasure_manager program
void execute_manager(char** manager_argv) {
  // Create the pipe before forking
  if (pipe(pipe_fd) == -1) {
    perror("Error creating the pipe");
    return;
  }
  // Create the child process (manager) using fork
  manager_pid = fork();
  if (manager_pid == -1) {
    perror("Error creating the manager process");
    if (close(pipe_fd[0]) == -1) {
      perror("Error closing the read end of the pipe");
    }
    if (close(pipe_fd[1]) == -1) {
      perror("Error closing the write end if the pipe");
    }
    return;
  }
  // Child process (manager)
  if (manager_pid == 0) {
    if (close(pipe_fd[0]) == -1) {
      perror("Error closing the read end of the pipe");
    }
    if (dup2(pipe_fd[1], 1) == -1) {
      perror("Error redirecting the standard output to the write end of the pipe");
    }
    if (close(pipe_fd[1]) == -1) {
      perror("Error closing the write end of the pipe");
    }
    execv(manager_argv[0], manager_argv);
    perror("Error executing the manager program");
    exit(EXIT_FAILURE);
  }
  // Parent process (monitor)
  printf("Manager process started with pid %d\n", manager_pid);
  fflush(stdout);
  // Send SIGUSR1 signal to the hub process
  if (kill(getppid(), SIGUSR1) == -1) {
      perror("Error sending SIGUSR1 to the hub process");
  }
  if (close(pipe_fd[1]) == -1) {
    perror("Error closing the write end of the pipe");
  }
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  while ((bytes_read = read(pipe_fd[0], &buffer, BUFFER_SIZE)) > 0) {
    // Send SIGUSR1 signal to the hub process
    if (kill(getppid(), SIGUSR1) == -1) {
      perror("Error sending SIGUSR1 to the hub process");
    }
    if (write(1, &buffer, bytes_read) == -1) {
      perror("Error writing the manager output to the standard output");
    }
  }
  if (bytes_read == -1) {
    perror("Error reading from the pipe");
  }
  if (close(pipe_fd[0]) == -1) {
    perror("Error closing the read end of the pipe");
  }
}

// Signal handler for SIGUSR1
void handle_sigusr1() {
  // Open the command file
  int cmd_fd = open(COMMAND_FILE, O_RDONLY);
  if (cmd_fd == -1) {
    perror("Error opening the command file");
    return;
  }
  // Read the command from the command file
  char cmd_buffer[MAX_COMMAND_LENGTH];
  ssize_t bytes_read = read(cmd_fd, cmd_buffer, MAX_COMMAND_LENGTH);
  // Close the command file
  if (close(cmd_fd) == -1) {
    perror("Error closing the command file");
  }
  // Remove the command file after closing
  if (unlink(COMMAND_FILE) == -1) {
    perror("Error removing the command file");
  }
  // Check if the command was read correctly
  if (bytes_read <= 0) {
    perror("Error reading from the command file");
    return;
  }
  cmd_buffer[bytes_read] = '\0';
  // Parse the command
  int manager_argc = 0;
  char** manager_argv = get_argv(cmd_buffer, &manager_argc);
  execute_manager(manager_argv);
  free_argv(manager_argv, manager_argc);
}

// Signal handler for SIGCHLD
void handle_sigchld() {
  // Parent process (monitor) reaps the child process (manager)
  int status;
  if (waitpid(manager_pid, &status, WNOHANG) == -1) {
    perror("Error waiting for manager process");
  } else {
    if (WIFEXITED(status)) {
      printf("Manager process ended with code %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
      printf("Manager process terminated by signal: %d\n", WTERMSIG(status));
    } else {
      printf("Manager process ended abnormally\n");
    }
  }
  fflush(stdout);
  // Send SIGUSR1 signal to the hub process
  if (kill(getppid(), SIGUSR1) == -1) {
    perror("Error sending SIGUSR1 to the hub process");
  }
  manager_pid = -1;
}

// Signal handler for SIGTERM
void handle_sigterm() {
  sleep(10); // 10 seconds delay
  printf("Exiting monitor...\n");
  // Send SIGUSR1 signal to the hub process
  if (kill(getppid(), SIGUSR1) == -1) {
    perror("Error sending SIGUSR1 to the hub process");
  } 
  exit(EXIT_SUCCESS);
}

// Dispatches signal handling to the appropriate handler
void monitor_handler(int signal) {
  switch (signal) {
    case SIGUSR1: {
      handle_sigusr1();
      break;
    }
    case SIGCHLD: {
      handle_sigchld();
      break;
    }
    case SIGTERM: {
      handle_sigterm();
      break;
    }
  }
}

int main() {
  printf("Monitor process started with pid %d\n", getpid());
  fflush(stdout);
  // Send SIGUSR1 signal to the hub process
  if (kill(getppid(), SIGUSR1) == -1) {
      perror("Error sending SIGUSR1 to the hub process");
  }
  // Set up signal handlers
  struct sigaction sa_monitor;
  sa_monitor.sa_handler = monitor_handler;
  sigemptyset(&sa_monitor.sa_mask);
  sa_monitor.sa_flags = SA_RESTART;
  if (sigaction(SIGUSR1, &sa_monitor, NULL) == -1) {
    perror("Error setting up SIGUSR1 handler");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGCHLD, &sa_monitor, NULL) == -1) {
    perror("Error setting up SIGCHLD handler");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGTERM, &sa_monitor, NULL) == -1) {
    perror("Error setting up SIGTERM handler");
    exit(EXIT_FAILURE);
  }
  while (1) {
    pause();
  }
  return EXIT_SUCCESS;
}
