#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define PROCESS_FAILURE 1
#define SYSCALL_FAILURE 2
#define CHECK_PROCESS(process) if(process < 0) exit(SYSCALL_FAILURE)

volatile sig_atomic_t done = 0;

/* Declarations of functions */
void sighandler(int signum);
void print_random_num();

/* MAIN */
int main() {
    pid_t gen, nsd;
    int pipe_fd[2];
    int exit_status_gen, exit_status_nsd, exit_code_gen, exit_code_nsd;

    // Create a pipe
    CHECK_PROCESS(pipe(pipe_fd));

    gen = fork(); // Create first child process GEN
    
    // Code executed by GEN process
    if(gen == 0 ) {
        CHECK_PROCESS(close(pipe_fd[0])); // Close unused read end of the pipe
        CHECK_PROCESS(dup2(pipe_fd[1], STDOUT_FILENO)); // Redirect stdout to the write end of the pipe
        CHECK_PROCESS(close(pipe_fd[1])); // Close the write end of the pipe
        CHECK_PROCESS(signal(SIGTERM, sighandler));
        while (!done) {
            // GEN process tasks go here
            print_random_num();
            fflush(stdout);
            sleep(1);
        }
        fprintf(stderr, "GEN TERMINATED\n");
        exit(0);
    } else if(gen < 0) {
        printf("ERROR\n");
        exit(SYSCALL_FAILURE);
    }

    nsd = fork(); // Create first child process NSD
    
    if (gen && nsd == 0) {
        // Code executed by NSD process
        CHECK_PROCESS(close(pipe_fd[1])); // Close unused write end of the pipe
        CHECK_PROCESS(dup2(pipe_fd[0], STDIN_FILENO)); // Redirect stdin to the read end of the pipe
        CHECK_PROCESS(close(pipe_fd[0])); // Close the read end of the pipe
        CHECK_PROCESS(execl("nsd", "nsd", NULL));
    } else if (nsd < 0) {
        printf("ERROR\n");
        exit(SYSCALL_FAILURE);
    }

    if(gen && nsd) {
        // Close both ends of the pipe in MAIN process as they are not needed here
        CHECK_PROCESS(close(pipe_fd[0]));
        CHECK_PROCESS(close(pipe_fd[1]));
        
        sleep(5);

        // Send a SIGTERM signal to the GEN process to terminate it
        CHECK_PROCESS(kill(gen, SIGTERM));

        // Wait for both child processes to complete
        CHECK_PROCESS(waitpid(gen, &exit_status_gen, 0)); // Wait for GEN process to terminate
        CHECK_PROCESS(waitpid(nsd, &exit_status_nsd, 0)); // Wait for NSD process to terminatewaitpid(gen, NULL, 0);

        if (WIFEXITED(exit_status_gen)) exit_code_gen = WEXITSTATUS(exit_status_gen);        

        if (WIFEXITED(exit_status_nsd)) exit_code_nsd = WEXITSTATUS(exit_status_nsd);

        if(exit_code_gen != 0 || exit_code_nsd != 0) {
            printf("ERROR\n");
            exit(PROCESS_FAILURE);
        }

        printf("OK\n");
        exit(0);
    }
}


/* Definitions of functions */
void sighandler(int signum) {
   done = 1;
}

void print_random_num() {
    int num1 = rand() % 4096;
    int num2 = rand() % 4096;
    printf("%d %d\n", num1, num2);
}
