#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "list.h"

/*
 * The tokenify() function from lab 3, with one modification,
 * for you to use.
 *
 * The modification is that it also takes a second parameter,
 * which is the set of characters on which to "split" the input.
 * In lab 3, this was always " \t\n", but you can call tokenify
 * to split on other characters, too, with the implementation below.
 * Hint: you can split on ";" in order to separate the commands on
 * a single command line.
 */
char** tokenify(const char *s, const char *sep) {
    char *word = NULL;

    // make a copy of the input string, because strtok
    // likes to mangle strings.
    char *s1 = strdup(s);

    // find out exactly how many tokens we have
    int words = 0;
    for (word = strtok(s1, sep);
         word;
         word = strtok(NULL, sep)) words++ ;
    free(s1);

    s1 = strdup(s);

    // allocate the array of char *'s, with one additional
    char **array = (char **)malloc(sizeof(char *)*(words+1));
    int i = 0;
    for (word = strtok(s1, sep);
         word;
         word = strtok(NULL, sep)) {
        array[i] = strdup(word);
        i++;
    }
    array[i] = NULL;
    free(s1);
    return array;
}

void free_tokens(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        free(tokens[i]); // free each string
        i++;
    }
    free(tokens); // then free the array
}

bool isbackground(char ** cmd){
    bool flag = false;
    int i=0;
    while (cmd[i]!=NULL){
        i=i+1;
    }
    int last_char_index = strlen(cmd[i-1])-1;
    if (strcmp(cmd[i-1],"&")==0) { // & as a solitary token
        flag = true;
        cmd[i-1] = NULL;
    }
    else if(cmd[i-1][last_char_index]=='&') { // & attached to last token
        flag = true;
        cmd[i-1][last_char_index] = '\0';
    }
    return flag;
}
int run_command(char ** cmd, bool isback){
	pid_t pid = fork();
    int status = 0;
	if (pid == -1){
		printf("Fatal error.\n");
        exit(1);
	}
	else if(pid == 0){
		if (execv(cmd[0], cmd) < 0) {
            printf("Could not run process\n");
			exit(-1);
		}
	}
	else if (pid > 0){
        if (isback) {
            waitpid(pid, &status, WNOHANG);
        }
        else{
            waitpid(pid, &status, WUNTRACED);
        }
	}
    return pid;
}
int main(int argc, char **argv) {
    struct proclist *plist = proclist_new(); // you'll need this eventually

    struct procnode * current;
    char ** commands;
    char ** currentcmd;
    char buffer[1024];
    int pid;
    int finished;
    bool isback;
    int index;
    // main loop for the shell
    while(true) {
        current = plist->head;
        while (current != NULL) {
            finished = waitpid(current->pid, NULL, WNOHANG);
            if (finished != 0) {
                proclist_remove(plist, current->pid);
            }
            current = current->next;
        }
        printf("prompt>");
        fflush(stdout);
        if (fgets(buffer, 1024, stdin)==NULL) {
            proclist_free(plist);
            if(commands!=NULL) {
                free_tokens(commands);
		    }
            exit(0);
            break;
        }
        buffer[strlen(buffer)-1]='\0';
        commands = tokenify(buffer, ";");
        index = 0;
        while (commands[index] != NULL){
            currentcmd = tokenify(commands[index]," \t");
            isback = isbackground(currentcmd);
            if ((strcmp(currentcmd[0],"exit")==0) && (currentcmd[1]==NULL)){
                proclist_free(plist);
                free_tokens(commands);
            	free_tokens(currentcmd);
                exit(0);
            }
            else if (strcmp(currentcmd[0],"status")==0 && (currentcmd[1]==NULL)){
                proclist_print(plist);
            }
            else if (strcmp(currentcmd[0], "kill" )==0){
                pid = atoi(currentcmd[1]);
                current = proclist_find(plist, pid);
                if (current != NULL) {
                    kill(pid, SIGKILL);
                    proclist_remove(plist, pid);
                }
                free(current);
            }
            else {
                pid = run_command(currentcmd, isback);
                if (isback) {
                    proclist_add(plist, pid, currentcmd[0]);
                }
            }
            index=index+1;
            if(currentcmd!=NULL) {
                free_tokens(currentcmd);
		    }
        }
       // free_tokens(currentcmd);
	 free_tokens(commands);
    }
    free_tokens(commands);

    proclist_free(plist); // clean up the process list
    return EXIT_SUCCESS;
}
