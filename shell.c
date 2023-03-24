/*
Christian Johnson
February 28, 2023
Program to emulate a shell using various functions in C.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <signal.h>

/* Random command length */
#define MAX_LEN 100

/* Number of command history is suppsoed to store */
#define MAX_HIST 10 

/* 2D Array to store the user-entered commands */
char history[MAX_HIST][MAX_LEN]; 
int count = 0; 

void hist_arr(char *command) 
{ 
  for (int i = 0; i < MAX_HIST - 1; i++) 
  {
    strcpy(history[i], history[i+1]);
  }
    
  strcpy(history[MAX_HIST - 1], command);
    
  if (count < MAX_HIST) 
  {
    count++;
  }
}


/* USR1 signal handler */
void signal_handler(int sig) 
{
  FILE *audit_file = fopen("audit.log", "w"); 
  if (audit_file == NULL) 
  {
    perror("Failed to open audit.log");
    exit(1);
  }

  int start = (count > MAX_HIST) ? (count % MAX_HIST) : 0;
  for (int i = start; i < count; i++) 
  {
    fprintf(audit_file, "%s\n", history[i]);
  }

  fclose(audit_file); 
    
  exit(0); 
}




int main(int argc, char *argv[]) 
{
  char *args[MAX_LEN/2 + 1]; // command arguments
  int running = 1; 
  char command[MAX_LEN]; 
  
  char *token = strtok(command, " ");

  /* Setting up the signal handler */
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGUSR1, &sa, NULL) == -1) 
  {
    printf("Signal Handler error. Sorry. \n");
    exit(1);
  }  
  
  while (running) 
  {
    printf("$ ");
    fflush(stdout);

        
    fgets(command, MAX_LEN, stdin);
    command[strcspn(command, "\n")] = '\0'; /* Remove newline character from the input */

    /* Handle the history command */   
    if (strcmp(command, "history") == 0) 
    {
      printf("Command History:\n");
      int i;
      int num_commands = count < MAX_HIST ? count : MAX_HIST;
      for (i = 0; i < num_commands; i++) 
      {
        printf("%d. %s\n", i+1, history[i]);
      }
        continue; //Restart loop to read user input
    }

    /* Update the history with the latest command */
    hist_arr(command);

    /* Parsing */
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) 
    {
      args[i] = token;
      i++;
      token = strtok(NULL, " ");
    }
    args[i] = NULL; // Set last argument to NULL
    strncpy(history[count % MAX_HIST], command, MAX_LEN);
    count++;

    int background = 0;
    if (strcmp(args[i-1], "&") == 0) 
    {
      background = 1;
      args[i-1] = NULL; 
    }


    /* Exit command */
    if (strcmp(args[0], "exit") == 0) 
    {
      running = 0;
      continue; 
    }
    
    /* Env */
    if (strcmp(args[0], "env") == 0) 
    {
      printf("User: %s (%d), Group: %s (%d)\n", getlogin(), getuid(), getgrgid(getgid())->gr_name, getgid());
      continue;
    }

    /* Cd and all its variants */
    if (strcmp(args[0], "cd") == 0) 
    {
      if (args[1] == NULL) 
      { 
        chdir(getenv("HOME"));
      } else 
      {
        if (chdir(args[1]) != 0) 
        { 
          printf("cd: no such file or directory: %s\n", args[1]);
        }
      }
      continue;
    }
      
    args[i] = NULL; 
      
    /* Forking and exec functions for command execution */
    pid_t pid = fork();
    /* Child process */
    if (pid == 0) 
    { 
      // Handling for < and > for redirection
      int input_redir = -1;
      int output_redir = -1;
      for (i = 0; args[i] != NULL; i++) 
      {
        if (strcmp(args[i], "<") == 0) 
        {
          input_redir = i;
        } else if (strcmp(args[i], ">") == 0) 
        {
          output_redir = i;
        }
      }

      /* If input redirection is needed */
      if (input_redir != -1) 
      {
        /* Open the input file and redirect stdin to it */
        int input_fd = open(args[input_redir + 1], O_RDONLY);
        if (input_fd == -1) 
        {
          perror("open");
          exit(1);
        }
        if (dup2(input_fd, STDIN_FILENO) == -1) 
        {
          perror("dup2");
          exit(1);
        }
        close(input_fd);
        args[input_redir] = NULL;
      }

      /* If output redirection is needed */
      if (output_redir != -1) 
      {
        /* Open the output file and redirect stdout to it */
        int output_fd = open(args[output_redir + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1) 
        {
          perror("open");
          exit(1);
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1) 
        {
          perror("dup2");
          exit(1);
        }
        close(output_fd);
        args[output_redir] = NULL;
      }

      execvp(args[0], args);
      printf("Command not found\n");
      exit(1);
    } else if (pid > 0) 
    { 
      if (!background) 
      {
        wait(NULL); 
      }
    } else 
    { 
      printf("Fork error. Sorry. \n");
      exit(1);
    }
  }

  return 0;

}
