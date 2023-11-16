/*
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file(s)
 * you will need to modify the CMakeLists.txt to compile
 * your additional file(s).
 *
 * Add appropriate comments in your code to make it
 * easier for us while grading your assignment.
 *
 * Using assert statements in your code is a great way to catch errors early and make debugging easier.
 * Think of them as mini self-checks that ensure your program behaves as expected.
 * By setting up these guardrails, you're creating a more robust and maintainable solution.
 * So go ahead, sprinkle some asserts in your code; they're your friends in disguise!
 *
 * All the best!
 */
#include <assert.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// The <unistd.h> header is your gateway to the OS's process management facilities.
#include <unistd.h>
#include <fcntl.h>
#include "parse.h"

#include "LinkedList.h"

static void run_cmds(Command *);
static void print_cmd(Command *cmd);
static void print_pgm(Pgm *p);
void stripwhite(char *);
static void invoke_cmd(Pgm *pgm, Command *cmd_list);
static void invoke_piped_cmd(Pgm *pgm, Command *cmd_list);

static LinkedList *list;
static int num_process_bg = 0;

volatile sig_atomic_t sigint_received = 0;

/*signal handler for incomming SIGINT*/
void sig_handler(int sig){
  printf("\n> ");
}

/*signal handler for incomming SIGCHILD*/
void check_if_background(int sig){
  if(sig == SIGCHLD)
  {
    for(int k = 0 ; k<num_process_bg; k++){
      
      if(l_size(list)>0)
      {
        int pid = l_get(list, k);
        int state;
        /*check the state but not block*/
        if(( waitpid(pid, &state, WNOHANG)) == 0)
        {
          continue;
        }else
        {
          int p = remove_element(list, k);
          if(p = pid)
          {
            num_process_bg--;
          }
        }
      }     
    }
  }
}
int main(void)
{
  signal(SIGINT, sig_handler);

  // Creates a list that will keep track of all background processes
  list = createLinkedList();
 
  signal(SIGCHLD, check_if_background);
  for (;;)
  {
    
    char *line;
    line = readline("> ");
   
    // If EOF encountered, exit shell
    if (!line)
    {
      break;
    }

    // Remove leading and trailing whitespace from the line
    stripwhite(line);

    // If stripped line not blank
    if (*line)
    {
      add_history(line);

      Command cmd;
      if (parse(line, &cmd) == 1)
      {
        run_cmds(&cmd);
      }
      else
      {
        printf("Parse ERROR\n");
      }
    }

    // Clear memory
    free(line);

  
  }
  // Frees up the memory used by the list
  free(list);
  return 0;
}


// Runs built-in commands and calls additional functions for other commands  
static void run_cmds(Command *cmd_list)
{
  char **args = cmd_list->pgm->pgmlist;
  Pgm *pgm = cmd_list->pgm;

  print_cmd(cmd_list);
  // Implementation of the cd command
  if (strcmp(args[0], "cd") == 0) {
    if(!args[1])
    {
      if(chdir(getenv("HOME")) != 0) {
        perror("lsh");
      }
    } else if (args[2] != NULL) {
      fprintf(stderr, "lsh: too many arguments to \"cd\"\n");
    } else {
      if (chdir(args[1]) != 0) {
        perror("lsh");
      }
    }
    return;
  // Implementation of the exit command
  } else if (strcmp(args[0], "exit") == 0) {
    exit(0);
  // If there is no match with any built-in command. Run invoke_cmd.
  } else {
    invoke_cmd(pgm, cmd_list);
  }
}

// Function that invokes singular commands with execvp and sends piped cmds to invoke_piped_cmd
static void invoke_cmd(Pgm *pgm, Command *cmd_list) {
  
  // Forks the process to make sure the shell doesn't terminate.
  int pid = fork();
  if (pid == -1) {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  // Child process
  if (pid == 0) {
    // If parse contains rstdout it will redirect the output of the process to writo to a file
    if(cmd_list->rstdout) {
      int fd = creat(cmd_list->rstdout, S_IRUSR | S_IWUSR);
      dup2(fd, STDOUT_FILENO); 
    }
    // If the parse contains multiple piped commands it will call invoke_piped_cmd 
    if(pgm->next) {
      invoke_piped_cmd(pgm, cmd_list);
    } else {
      // If parse contains rstdin it will redirect the input of the process to read from a file
      if(cmd_list->rstdin) {
      int fd = open(cmd_list->rstdin, O_RDONLY);
      dup2(fd, STDIN_FILENO); 
    }
    // if commands is flagged as background process it will ignore SIGINT (ctrl-c)
      if(cmd_list->background != 0) {
        signal(SIGINT, SIG_IGN);
      }
      // Will execute the command. This will only happens if there is only one command, all other execution takes place in invoke_piped_command
      if (execvp(pgm->pgmlist[0], pgm->pgmlist) == -1) {
        perror("lsh");
        exit(EXIT_FAILURE);
      }
    }
  // Parent process
  } else {
    // The parent process waits for child, but if it the background flag is up it will save the background process in a list and then continue on.
    if(!cmd_list->background) {
      waitpid(pid, NULL, 0);
    } else {
      add(list, num_process_bg++, pid);
    }
    return 0;

  }
}

void get_args(char *args[], char **line){
  int i = 0;
  while((*(line+i))!= NULL){
    args[i++] = *(line+i);
  }
}

/*
This function is invoked by invoke_cmd when there are multiple commands
this function recursively creates pipes between new processes which run commands.   
*/
static void invoke_piped_cmd(Pgm *pgm, Command *cmd_list) {

  int fd[2];
  int pid;

  // Creates a pipe
  if(pipe(fd) == -1) {
    perror("pipe");
    return;
  }

  // Forks the process
  pid = fork();

  if(pid < 0) {
    perror("fork failed");
    return;

  // Child process (or child-child process)
  } else if (pid == 0) {
    // Makes sure that the output of the process will go into the writeend of the pipe
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);

    // Changes to next command, since the parent will check the current one.
    pgm = pgm->next;

    // If there is a next one after that, then run the function recursively.  
    if(pgm->next != NULL) {
      invoke_piped_cmd(pgm, cmd_list);
    }
    // Otherwise the command will be executed 
    else {
      // If it's a background process it will ignore CTRL-C
       if(cmd_list->background != 0) {
        signal(SIGINT, SIG_IGN);
      }
      if(execvp(pgm->pgmlist[0], pgm->pgmlist) == -1) {
        perror("lsh");
      }
    }
  }
  // Parent process 
  else {
    // Make sure the process will read from the readend of the pipe
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);

    // Ignores CTRL-C if its a background process
    if(cmd_list->background != 0) {
        signal(SIGINT, SIG_IGN);
    }
    // Executes command
    if(execvp(pgm->pgmlist[0], pgm->pgmlist) == -1) {
        perror("lsh");
  }
  printf("runing piped cmd");
}
}


/*
 * Print a Command structure as returned by parse on stdout.
 *
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
static void print_cmd(Command *cmd_list)
{
  printf("------------------------------\n");
  printf("Parse OK\n");
  printf("stdin:      %s\n", cmd_list->rstdin ? cmd_list->rstdin : "<none>");
  printf("stdout:     %s\n", cmd_list->rstdout ? cmd_list->rstdout : "<none>");
  printf("background: %s\n", cmd_list->background ? "true" : "false");
  printf("Pgms:\n");
  print_pgm(cmd_list->pgm);
  printf("------------------------------\n");
}

/* Print a (linked) list of Pgm:s.
 *
 * Helper function, no need to change. Might be useful to study as inpsiration.
 */
static void print_pgm(Pgm *p)
{
  if (p == NULL)
  {
    return;
  }
  else
  {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    print_pgm(p->next);
    printf("            * [ ");
    while (*pl)
    {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}


/* Strip whitespace from the start and end of a string.
 *
 * Helper function, no need to change.
 */
void stripwhite(char *string)
{
  size_t i = 0;

  while (isspace(string[i]))
  {
    i++;
  }

  if (i)
  {
    memmove(string, string + i, strlen(string + i) + 1);
  }

  i = strlen(string) - 1;
  while (i > 0 && isspace(string[i]))
  {
    i--;
  }

  string[++i] = '\0';
}
