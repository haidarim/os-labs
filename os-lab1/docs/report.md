# Lab Report
## Specifications
Our shell starts of by checking if the input line contains any of the built-in commands `cd` or `exit`. If it is `cd` we check if there are any arguments, if there is none then `cd` changes directory to "HOME", if there is one argument we change directory to it. If there are more than one argument we do nothing and give the message that there are too many arguments.
For commands such as `ls`, `date`, and `who`, and other basic commands we use the function invoke_cmd. 

Invoke_cmd will fork the process and let the child check whether the input contains one or more commands. If there is one command it will run execvp with the command and arguments given. If the parsed input contains more then one command then invoke_cmd will call invoke_piped_cmd. In invoke_piped_cmd a pipe will be created and then the process will once again be forked. The child will redirect its output into the pipe using dup2. It will then check if there is a next-next command, if there is it will recursively call the function again. If there is no next-next command it will execute the command using execvp. The parent process will use dup2 to make sure it is able to read from the pipe. The parent will then execute the current command.

The first fork-call is responsible for I/O-redirection and for execution of command in program[0], if there are multiple programs i.e. multiple commands then the second fork will take care of execution of those commands and the communication between them. In this lab we used ordinary piping. For reading from an existing file we use `open()` function with the `O_RONLY` parameter. Writing the output into output file i.e. rstdout, is done by `creat` which writes to the file if it exists otherwise will create a file as rstdout and then write into it. 

For background commands we check the cmd_list->background variable. If it is set to 1, we ignore the SIGINT signal and we do not use waitpid(...) to wait for the child. We also add the pid for the background process into a linked list which we use to keep track of all background processes. This is then used for cleanup when it comes to zombies. 

When a background process/ or any process is done, it sends a `SIGCHILD` signal. When this signal is recieved, `check_if_background` is being trigged as signal handler. This function uses `waitpid(pid, &state, WNOHANG)` for cleanup. We use WNOHANG to find that processes have died. If the waitpid returns 0 for the pid then we just continue since the process hasn't stopped working yet. Otherwise it is removed by waitpid and we remove it from our linked list to stop keeping track of it. The advantage with `WNOHANG` as parameter is that the waitpid function does not block other jobs (processes) i.e. waiting being non-blocking. 


For the ctrl-c, we basically receive the signal and instead of killing our outer shell we write "\n> ". For background processes, we also ignore the ctrl-c signal with SIG_IGN. 
## Order of Implementation and Challenges
We started with task 1. We naively began with implementing checks to see what was written, which we soon found out was wrong. The problem being of course that we would have to write an unending amount of commands to handle everything.

Then we took the incoming line and filled an array with the command and its arguments.  By using this array we could use the 0th element for the command itself and then the rest as arguments for the execvp function. Later we changed it to have a pointer-pointer to the pgmlist instead of creating a new array. Functionally the same, just removed one unecessary step.

Afterwards we implemented background tasks. At first we thought that we had to look at the command line sent to see if `&` was a symbol sent but after a while we tested sending some commands using the `&` and using the print_cmd function we saw that the cmd_list variable was set automatically by parse. Then we used this variable to simply have the parent process not wait for its child in the event of background being set to 1. This then led to zombies but we didn't focus on that until a bit later. 

Then we implemented the built in functions for `exit` and `cd`, these commands are done by `exit(...)` and `chdir`respectively. We fixed a case for the CD command where no input is provided, then it sends the user to HOME instead of doing nothing. 

By now we felt the need to extract the code that did the forking and was going to do the piping. We then ran into some problems. First of, we exited completely. For the exiting we found that we were using execvp wrong. Or rather, we did it at the wrong place, leading to execvp taking over our process and then exiting. This was fixed by forking an extra time to have an inner and outer process so that execvp only killed the inner. We also got stuck after sending commmands which stopped after we closed our pipes correctly and in the correct places. 

Our redirects worked with one command. But it broke down when using multiple pipes in one line. Our theory was that we redirected at the wrong time. Meaning that commands like "ls | grep par | wc -l > words" were redirected immediately so the piped commands didn't get anything as input. This was fixed by redirecting only if the commands were the first, last, or only one we got. 

As next step, we started to try to fix zombies. We left zombies when running background processes and we created zombies immediately when running single commands. Such as `sleep 60 &`. We got rid of zombies by using the process mentioned in the specifications. However we still had zombies when doing simple commands, only they were removed soon after. So we needed to stop the zombies from being created in the first place. 

We got the tip from a lab assistant that we were using fork one too many times due to a misunderstanding on how to handle ctrl-c. 
We thought we needed an extra layer of fork to make it so that ctrl-c didn't stop our shell that was the outer process, but we simply missed the easy solution
which was to handle the ctrl-c with signal handling. 

After rewriting our code, not completely from scratch but a lot of it, we found out that we had been thinking correctly about how many forks we needed to have. Our problem was that we had missed that since we did recursive calls, our base case was doing one extra fork. When in reality we were only supposed to fork if we hade a pipe. What this means is that if we sent a command such as "sleep 60" our shell would do: base shell->fork once to allow execvp execution without closing->fork once more for command. When in reality the behavior we wanted was: base shell->fork once to allow execvp execution without closing and execute. The rewriting allowed us to see this and clean up the code in some places but was a lot of time spent on a bug which could have been fixed quite easily. 

In the rewriting, the main thing we did was remove the base case to its own function and remove the fork from the run_cmds function that gets the command list from the main function. The total amount of forks in the code are the same, but where and when they are executed is different. 

## Feedback for improving lab
It could maybe been a bit more clear how much work parse actually did. It took us a while before we understood that it was already able to recognize the symbols for pipe, background and redirection. 
It was quite a lot in the beginning and the lab felt a bit daunting at first, mostly due to a lack of experience and knowledge in the beginning. But when we had learned the basic functions and how for example piping was to be used, it became easier to be able to just think of the logic of how the program was supposed to work instead of having to think about how the concepts worked. Otherwise, quite fun and interesting. Felt like a responsive lab in that you can see your changes immediately and not have to wait for some result to be simulated or something like that.

