/*
Written by Sam Keller, Simon Montague, and Will Herold

Written on 10/25/2011

This program sets up our parser, which makes a tree out of the tokenized tokens
*/

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

//return a linked list called parser
LinkedList* parse(LinkedList* tokens, int* depth){
    LinkedList* stack = createList();
    init(stack);
    ConsCell* current = tokens->head;
    //while we still have tokens to step through
    while(current != NULL){
        // if current is an open paren
        if((*current).car->type == 5){
            (*depth)++;
        }
        // if current is a closed paren
        if((*current).car->type == 6){
            // if the depth has already gone below 0, we don't want to do any of that stuff
            if((*depth) <= 0){
                 (*depth) = (*depth) - 1;
            }
            if((*depth) > 0){
                (*depth) = (*depth) - 1;
                
                LinkedList* child = createList();
                init(child);
                //this would insert the open parenthesis into the parse tree...
                //but we don't want that
                //insert(child, current->car);
                
                // while we are still inside a set of parens, add the contents to the child list
                while(((stack->head)->car)->type != 5){
                    insert(child, (*stack).head->car);
                    pop(stack);
                }
                //This would add the closed parentheses to the parse tree...
                //but we don't want that
               //insert(child, (*stack).head->car); 
                pop(stack);
                Value* childValue = createValue();
                enum TOKEN_TYPE newCellType = consType;
                childValue->type = newCellType;
                childValue->cons = child->head;
                insert(stack,childValue);
            }
            

        }else{
            insert(stack, current->car);
        }
        current = current->cdr;  
    }
    return stack;
}

// printValue prints out the value
void printValue(Value* value){
    //check for all of the different cases 
    if(value->type == booleanType) {
        if(value->boolValue == 1) {
            printf("#t ");
        } else {
            printf("#f ");
        }
    }
    if(value->type == integerType) {
        printf("%d ",value->intValue);
    }
    if(value->type == floatType) {
        printf("%f ",value->fltValue);
    }
    if(value->type == stringType) {
        printf("%s ",value->strValue);
    }
    if(value->type == symbolType) {
        printf("%s ",value->symValue);
    } //only getting part of the symbol
    if(value->type == openType) {
        printf("%c ",value->openValue);
    }
    if(value->type == closedType) {
        printf("%c ",value->closedValue);
    }
    if(value->type == consType){
		printf("(");
        ConsCell* starterCell = value->cons;
        while(starterCell != NULL){
            printValue(starterCell->car);
            starterCell = starterCell->cdr;
        }
		printf(")");
    }
	if(value->type == closureType){
		printf("#<procedure> ");
	}
}
