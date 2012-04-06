/* 
Written by Sam Keller, Simon Montague, and Will Herold

Written on 10/25/2011

This is our main function that tests our parser
*/

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int main(){
        //make a linked list
    LinkedList* tokens = malloc(sizeof(LinkedList));
    LinkedList* tokenized = malloc(sizeof(LinkedList));
    LinkedList* parseTree = malloc(sizeof(LinkedList));
    LinkedList* leftoverTokens = NULL;
    int depth = 0;
    int depthCopy = 0;
    char *expression = malloc(256 * sizeof(char));
    
    // go through the expression and tokenize it!
    while (fgets(expression, 256, stdin)){
        tokenized = tokenize(expression);
        tokenized = ReverseIt(tokenized);
        tokens = append(leftoverTokens, tokenized);        
        parseTree = parse(tokens, &depth); 
        parseTree = ReverseIt(parseTree);
        
        //if the depth is less than zero, return a closed email
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
            ConsCell* current = parseTree->head;
            depthCopy = 0;
            while (current != NULL){
	           printValue(current->car);
	            current = current->cdr;
	        }
	        leftoverTokens = NULL;
	        
      }   
    }
    // if, at the end, we have too few closed parens, return an error.
    if (depthCopy > 0) {
      printf("syntax error: too few closed parens\n");   // Too few close parens at end of input.
      enum ERROR_TYPE closedError = tooFewClosed;
      return closedError;
   }
  
    printf("\n"); //This is just for asthetics in the terminal. 
    free(expression);
    return 0;
}