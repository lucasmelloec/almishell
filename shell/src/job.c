#include <job.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

struct job *init_job() {
    struct job *j = (struct job *) malloc(sizeof(struct job));

    j->background = 'f';
    j->first_process = NULL;

    return j;
}

void delete_job(struct job *j) {
    struct process_node *current = j->first_process, *next = NULL;

    if(current) {
        free(current->p->argv[0]); /* Free command line memory */

        while(current->next) {
            next = current->next;
            free(current->p->argv); /* Free token location memory */
            free(current->p);
            free(current);
            current = next;
        }

        free(current);
    }

    free(j);
}

void put_job_in_foreground(struct shell_info *s, struct job *j, int cont) {
    int ret_status;

    /* Give control access to the shell terminal to the job */
    tcsetpgrp(s->terminal, j->first_process->p->pid);

    /* TODO: If job is being continued */

    waitpid(j->first_process->p->pid, &ret_status, 0);

    /* Return access to the terminal to the shell */
    tcsetpgrp(s->terminal, s->pgid);

    /* Restore shell terminal modes */
    tcsetattr(s->terminal, TCSADRAIN, &s->tmodes);
}

void run_job(struct shell_info *s, struct job *j, int fg)
{
    struct process *p = j->first_process->p;
    pid_t pid;
    int ret_status;

    /* TODO: Extend for jobs with multiple processes */
    pid = fork();

    if(pid == 0) {
        run_process(s, p, fg);
    }
    else if(pid < 0){
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else {
        p->pid = pid;
        if(s->interactive) {
            /* TODO: Handle process group for whole jobs with multiple processes */
            setpgid(pid, pid);
        }
    }

    /* TODO: Handle pipes */
    if(!s->interactive)
        waitpid(pid, &ret_status, 0);
    else if(fg)
        put_job_in_foreground(s, j, 0);
    /* TODO: Handle background */

}