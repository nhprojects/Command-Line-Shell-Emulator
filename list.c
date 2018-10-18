#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "list.h"

/*
 * Return a pointer to a newly-heap-allocated struct proclist.
 * Don't forget to initialize the contents of the struct!
 */
struct proclist* proclist_new() {
    struct proclist* new_list = malloc(sizeof(struct proclist));
    new_list -> head = NULL;
    new_list -> length = 0;
    return new_list;
}

/*
 * Completely free the memory consumed by a struct proclist,
 * including each node (and each cmd in each node, which should
 * also be heap-allocated), and the struct proclist itself.
 */
void proclist_free(struct proclist *head) {
    struct procnode * current = head->head;
    struct procnode * next = NULL;
    while (current!=NULL) {
        next = current->next;
        free(current->cmd);
        free(current);
        current = next;
    }
    free(head);
}

/*
 * Create a new heap-allocated struct procnode, initialize it
 * with the pid and cmd passed as parameters, and add it to the
 * struct proclist.
 *
 * The cmd string *must* be copied to a newly heap-allocated
 * location.  (Hint: use strdup.)
 */
void proclist_add(struct proclist* head, pid_t pid, char *cmd) {
    char * newCmd = strdup(cmd);
    struct procnode * nextPtr = malloc(sizeof(struct procnode));
    nextPtr->pid = pid;
    nextPtr->cmd = newCmd;
    nextPtr->next = NULL;
    struct procnode * current = head->head;
    if (current == NULL){
        head->head=nextPtr;
    }
    else{
        while (current->next!=NULL) {
            current = current->next;
        }
        current->next = nextPtr;
    }
    head->length += 1;
}

void procnodeToString(struct procnode * node){
    if (node!=NULL){
        printf("pid: %i, cmd: %s\n",node->pid, node->cmd);
    }
}

/*
 * Search the struct proclist for a struct procnode with a particular
 * process id (pid).  Return NULL if the pid isn't anywhere on the proclist,
 * or a pointer to the struct procnode for that pid.
 */
struct procnode* proclist_find(struct proclist* head, pid_t pid) {
    struct procnode * current;
    current= head->head;
    while (current!=NULL){
        if (current->pid==pid){
            return current;
        }
        current=current->next;
    }
    return NULL;
}

/*
 * Remove the procnode within the proclist that has a particular
 * pid.  If a procnode with the pid doesn't exist on the proclist, do
 * nothing.
 * When removing, don't forget to deallocate any heap memory used
 * by the struct procnode being removed (including the cmd, which should
 * have been heap-allocated).
 */
void proclist_remove(struct proclist *head, pid_t pid) {
    struct procnode * prev = head->head;
    struct procnode * temp;
    if (prev->pid==pid) {
        temp = head->head;
        head->head=prev->next;
        head->length-=1;
        free(temp->cmd);
        free(temp);
        return;
    }
    while (prev->next!=NULL){
        if (prev->next->pid==pid){
            temp = prev->next;
            prev->next = prev->next->next;
            head->length -= 1;
            free(temp->cmd);
            free(temp);
            return;
        }
        prev = prev->next;
    }
}
/*
 * Print some representation of the (active) processes on the proclist.
 * For example, if no processes are running, you can print something like:
 *

 Processes currently active: none

 *
 * If there are any processes active, you output might look like:

 Processes currently active:
        [30]: /bin/sleep
        [29]: /bin/sleep
        [28]: /bin/sleep

 */
 void proclist_print(struct proclist * head){
     char * prompt = "Processes currently active: ";
     if (head->head==NULL){
         printf("%s none\n", prompt);
         return;
     }
     struct procnode * current = head->head;
     printf("%s\n", prompt);
     procnodeToString(current);
     while (current->next!=NULL) {
         current = current->next;
         procnodeToString(current);
     }
 }
