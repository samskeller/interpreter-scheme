/*
Written by Sam Keller, Simon Montague, and Will Herold

Written on 11/01/2011

This code includes our basic test of our interpreter, as defined in intepreter.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

LinkedList* globalList;

int main(int argc, char *argv[]){
    globalList = malloc(sizeof(LinkedList));
    init(globalList);
    printf("-----------------------------------------------------------\n");
    printf("---------------Interactive Scheme Interpreter--------------\n");
    printf("----Written by: Will Herold, Simon Montague, Sam Keller----\n");
    printf("-----------------------------------------------------------\n");
    LinkedList* tokens = createList();
    init(tokens);
    LinkedList* tokenized = createList();
    init(tokenized);
    LinkedList* parseTree = createList();
    init(parseTree);
    LinkedList* leftoverTokens = NULL;
    
    int depth = 0;
    int depthCopy = 0;
    char *expression = malloc(256 * sizeof(char));
    
    //Top Environment
    Environment* topEnv = createEnvironment(NULL);
    
    
    //Create the primitive functions+bindings
    char* lessThanEqualsFunc = "<=";
    char* addFunc = "+";
    char* greaterThanEqualsFunc = ">=";
    char* subtractFunc = "-";
	char* equalFunc = "=";
	char* carFunc = "car";
	char* cdrFunc = "cdr";
	char* divideFunc = "/";
	char* consFunc = "cons";
	char* multiplyFunc = "*";
	char* nullCheck = "null?";
	char* greaterThanFunc = ">";
	char* lessThanFunc = "<";
	bind(equalFunc, makePrimitiveValue((*equals)), topEnv);
    bind(subtractFunc,makePrimitiveValue((*subtract)), topEnv);
    bind(addFunc, makePrimitiveValue((*add)), topEnv);
    bind(lessThanEqualsFunc, makePrimitiveValue((*lessThanEquals)), topEnv);
    bind(greaterThanEqualsFunc, makePrimitiveValue((*greaterThanEquals)), topEnv);
    bind(carFunc, makePrimitiveValue((*car)), topEnv);
	bind(cdrFunc, makePrimitiveValue((*schemeCdr)), topEnv);
    bind(divideFunc, makePrimitiveValue((*divide)), topEnv);
	bind(consFunc, makePrimitiveValue((*cons)), topEnv);
	bind(multiplyFunc, makePrimitiveValue((*multiply)), topEnv);
    bind(nullCheck, makePrimitiveValue((*isNull)), topEnv);
    bind(greaterThanFunc, makePrimitiveValue((*greaterThan)), topEnv);
    bind(lessThanFunc, makePrimitiveValue((*lessThan)), topEnv);
    
    //Load the libraries from the command line
    if(argc > 1){
        int i = 1;
        while(i< argc){
            Value* fakeList = createValue();
            fakeList->type = consType;
            ConsCell* fakeCons = malloc(sizeof(ConsCell));
            Value* dumbValue = malloc(sizeof(Value));
            dumbValue->type = stringType;
            dumbValue->strValue = argv[i];
            fakeCons->car = dumbValue;
            fakeList->cons = fakeCons;
            load(fakeList, topEnv);
            
            Value* memValue = malloc(sizeof(Value));
            memValue->type = consType;
            memValue->cons = fakeCons;
            memoryInsert(globalList, memValue);
            free(dumbValue);
            i++;
        }
    }
    

    int exit = 1;
    
    // this while loop makes our interactive window work.
    while(exit == 1){
        // go through the expression and tokenize it!
            fputs("> ", stdout);
            fflush(stdout);
            fgets(expression, 256, stdin);
            if(strcmp(expression, "(exit)\n") == 0){
                exit = 0;
            }else {
                tokenized = tokenize(expression);
                if(tokenized->head != NULL){ 
                    tokenized = ReverseIt(tokenized);
                    tokens = append(leftoverTokens, tokenized);        
                    parseTree = parse(tokens, &depth); 
                    parseTree = ReverseIt(parseTree);
                    //if the depth is less than zero, return a closed error 
                    if (depth < 0) {
                         printf("syntax error: too many closed parens\n");   // Too many close parentheses.
                         enum ERROR_TYPE closedError = tooManyClosed;
                        return closedError;
                    // if the depth is greater than zero, keep going
                    } else if (depth > 0) {
                        // There are more open parens than close parens, so these tokens are saved as leftovers. 
                        leftoverTokens = tokens;
                        depthCopy = depth;
                        depth = 0; 
                    }else {
                        depthCopy = 0;
                        //cleanupList(leftoverTokens);
                        leftoverTokens = NULL;
                        //consCell* current = parseTree->head;
                        //The interpreter part starts here
                        ConsCell* current = parseTree->head;
                        while(current != NULL){
                            Value* finalParse = current->car;//THIS IS THE VALUE FOR THE PARSE TREE
                            // evalute the parse tree! and then print it and free some things.
                            Value* evaluation = eval(finalParse, topEnv);
                            if(evaluation->type != blankType){
                                printValue(evaluation);
                                printf("\n");
                            }
                            current = current->cdr;
                        }
                    }
                }//end of if(tokenized != NULL)
            }
     }
    
    //Cleanup memory!
    free(expression);   
    globalCleanup(globalList);
    free(globalList);
    
    // if, at the end, we have too few closed parens, return an error.
	if (depthCopy > 0) {
      printf("syntax error: too few closed parens\n");   // Too few close parens at end of input.
      enum ERROR_TYPE closedError = tooFewClosed;
      return closedError;
   }

    
    return 0;   
}

