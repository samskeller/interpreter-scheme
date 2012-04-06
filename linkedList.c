/*
linkedList.c

Written by Sam Keller, Will Herold, and Simon Montague

This code initializes a given linked list, inserts values into it, and prints it. 
We also have a reverse function that reverses the order of the list since our code
will automatically make a backwards list.
*/

#include <stdio.h>
#include <stdlib.h>
#include "linkedList.h"


// set up the linked list, set its head to Null
void init(LinkedList *list) 
{
  list->head = NULL;
}


//Insert into our global linked list for memory cleanup later
void memoryInsert(LinkedList* myList, Value* dummyValue){
    ConsCell* newCell = malloc(sizeof(ConsCell));
    newCell->car = dummyValue;
    newCell->cdr = myList->head;
    myList->head = newCell;
}

//insert a given value into a linked list.
void insert(LinkedList *list, Value *value){
  // make a cons cell
  ConsCell *cell = malloc(sizeof(struct __ConsCell));
  cell->car = value;
  //set the cdr of the cons cell to be whatever the head used to be
  cell->cdr = list->head;  
  //reset the head of the linked list
  list->head = cell;
  Value* memoryValue = malloc(sizeof(Value));
  memoryValue->type = consType;
  memoryValue->cons = cell;
  memoryInsert(globalList, memoryValue);
}

//print the tokens inside of a linked list.
void printTokens(LinkedList *list) {
  ConsCell *current = list->head;
  while (current != NULL){
  // all of these if statements are accounting for the different types of things
  //that we could encounter in our scheme code. The type number corresponds to the 
  //what the type is given in the enum TOKEN_TYPE, which is located in linkedList.h
    if((*current).car->type == 0) {
        if((*current).car->boolValue == 1) {
            printf("#t:boolean\n");
        } else if ((*current).car->boolValue == 0) {
            printf("#f:boolean\n");
        }else{
        //if it is not one of these, then we have a case where we are using # improperly
        printf("Our implementation doesn't deal with other things after # symbols. So don't do that!\n");
        }
        
    }
    if((*current).car->type == 1) {
        printf("%d:integer\n",(*current).car->intValue);
    }
    if((*current).car->type == 2) {
        printf("%f:float\n",(*current).car->fltValue);
    }
    if((*current).car->type == 3) {
        printf("%s:string\n",(*current).car->strValue);
    }
    if((*current).car->type == 4) {
        printf("%s:symbol\n",(*current).car->symValue);
    } //only getting part of the symbol
    if((*current).car->type == 5) {
        printf("%c:open\n",(*current).car->openValue);
    }
    if((*current).car->type == 6) {
        printf("%c:closed\n",(*current).car->closedValue);
    }
    current = current->cdr;
  }

}
//this function reverses a linked list
LinkedList *ReverseIt(LinkedList *list) {
    //make a new linked list
    LinkedList *reverseList = createList();
    init(reverseList);
    ConsCell *current = (*list).head;
    // go through the old linked list and insert each value into our new one. this will
    //reversee the order for our new one.
    while(current != NULL){
        insert(reverseList, (*current).car);
        current = current->cdr;
    }
    return reverseList;
}

// the pop function pops the first value off of a given linked list
Value* pop(LinkedList *list){
    // reset some pointers
    ConsCell* currentValue = list->head;
    Value* popVal = currentValue->car;
    ConsCell* newHead = currentValue->cdr;
    list->head = newHead;
    return popVal;
}

// the append function appends one linked list to another
LinkedList* append(LinkedList* leftOver, LinkedList* mainList){
    if(leftOver == NULL){
        return mainList;
    }else{
        // set up all of the pointers
        ConsCell* currentCons = leftOver->head;
        while(currentCons->cdr != NULL){
            currentCons = currentCons->cdr;
        }
        currentCons->cdr = mainList->head;
        mainList->head = leftOver->head;
        return mainList;
    }
}

// createValue is just a simple way for us to create a new Value and add it to the global mem list
Value* createValue(){
    Value* value = malloc(sizeof(Value));
    Value* memValue =malloc(sizeof(Value));
    memValue->type = valueType;
    memValue->valueValue = value;
    memoryInsert(globalList, memValue);    
    return value;
}

//Create a Linked List and also add it to the global memory list
LinkedList* createList(){
    LinkedList* newList = malloc(sizeof(LinkedList));
    Value* memValue = malloc(sizeof(Value));
    memValue->type = listType;
    memValue->listValue = newList;
    memoryInsert(globalList, memValue);
    return newList;
}


//------------------------
//MEMORY CLEANUP FUNCTIONS
//------------------------
//Take a value, free what it points to (could be many different things) and then free the value
void freeValue(Value* value){
    //Free what the value points to - NOTE: there are some things (linked lists, etc) that we do
    //                                elsewhere so we did not put them in here. See globalCleanup.
    
    
    if(value->type == stringType){
        free(value->strValue);
    }else if(value->type == symbolType){
        free(value->symValue);
    }else if(value->type == closureType){
        free(value->closureValue);
    }else if(value->type == bindingType){
        free(value->bindingValue);
    }

    //Free the value itself
    free(value);
}

void globalCleanup(LinkedList* globalList){
    ConsCell* current = globalList->head;
    //Free the contents of the values stored in the global list's cons cells
    while(current != NULL){
        if(current->car->type == consType){
            if(current->car->cons != NULL){
                free(current->car->cons);
            }
        }else if(current->car->type == valueType){
            //If its a value, we first need to free what the value points to, then the value itself
            freeValue(current->car->valueValue);
        }else if(current->car->type == listType){
            free(current->car->listValue);
        }else if(current->car->type == environmentType){
            free(current->car->envValue);
        }else{
            printf("Unknown type \n");
        }
        
        current = current->cdr;
    }
    
    //Free the values themselves
    ConsCell* current2 = globalList->head;
    while(current2 != NULL){
        free(current2->car);
        current2 = current2->cdr;
    }
    
    //Free the cons cells
    ConsCell* current3 = globalList->head;
    ConsCell* previous = globalList->head;
    while(current3 != NULL){
        current3 = current3->cdr;
        free(previous);
        previous = current3;
    }
}
