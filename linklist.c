#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// ---------------------------------------- Linked List ----------------------------------------------------

struct node {
    char* type;
    struct node *next;
};

struct node *head = NULL;

/* Code Reference: https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm */
struct node* insertFirst(char* type) {
    //create a link
    struct node *link = (struct node*) malloc(sizeof(struct node));
    link->type = type;
    //point it to old first node
    link->next = head;
    //point first to new first node
    head = link;
    //printf("%s ", my_head->type);
    return head;
}

//find a link with given type
int find(char* type) {
    //start from the first link
    struct node* current = head;
    //if list is empty
    if(head == NULL) {
        return 0;
    }
    //navigate through list
    while(strcmp(current->type, type) != 0) {
        //if it is last node
        if(current->next == NULL) {
            return 0;
        } else {
            //go to next link
            current = current->next;
        }
    }
    //if data found, return true
    return 1;
}

void printList() {

    struct node *ptr = head;
    //start from the beginning
    while(ptr != NULL) {
        printf("%s \n", ptr->type);
        ptr = ptr->next;
    }
}

//-----------------------------------------------------------------------------------------------------------

int main(int argc, char **argv){
    char a[] = "a";
    char b[] = "b";
    char c[] = "c";
    
    insertFirst(a);
    insertFirst(b);
    insertFirst(c);
    printList();
}

