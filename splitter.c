/*
splitter.c

Written by Sam Keller, Will Herold, and Simon Montague

This is the majority of code and it takes in the expression in Scheme and returns a
linked list with all of the pieces and their types
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splitter.h"

LinkedList *tokenize(char *expression){
  int i;
  i = 0;
  //create the linked list
  LinkedList *tokens = createList();
  init(tokens);
  //while the expression still has characters left
  while(i < strlen(expression)){
    char current = expression[i];
    
    if(current == '\n'){
        i = strlen(expression);
    }
    
    //if we've hit a string:
    else if(current == '"'){
      int stringLen = 1;
      int temp = i+1;
      //while we're still in the string:
      while(expression[temp] != '"'){ 
	    temp++;
	    stringLen++;
      }
      
      char* newString = malloc(sizeof(char)*(stringLen+1));
      newString[stringLen] = '\0';
      char* ip = &(expression[i]);
      //we use strncopy to make a newString that we can add to our linked list
      strncpy(newString, ip,stringLen+1);
      current = temp;
      //set its type, strValue, etc
      Value* tempValue = createValue();
      enum TOKEN_TYPE newCellType = stringType;
      tempValue->type = newCellType;
      tempValue->strValue = newString;
      // add it to the linked list
      insert(tokens, tempValue);
      //reset i to be where temp is, otherwise we'll be lost in the expression
      i = temp; 
      
    }
    //if we hit an open parenthesis
    else if(current == '('){
      Value* tempValue = createValue();
      enum TOKEN_TYPE newCellType = openType;
      tempValue->type = newCellType;
      char tempChar = '(';
      tempValue->openValue = tempChar;
      insert(tokens, tempValue);
    }
    //if we hit a closed parenthesis
    else if(current == ')'){
      Value* tempValue = createValue();
      enum TOKEN_TYPE newCellType = closedType;
      tempValue->type = newCellType;
      char tempChar = ')';
      tempValue->closedValue = tempChar;
      insert(tokens, tempValue);
    }
    //if we hit a pound symbol
    else if(current == '#'){
      Value* tempValue = createValue();
      enum TOKEN_TYPE newCellType = booleanType;
      tempValue->type = newCellType;
      int tempBool;
      //if it's true, return a 1
      if(expression[i+1] == 't'){
        tempBool = 1; 
        tempValue->boolValue = tempBool;
        insert(tokens, tempValue);
        i++;
        }
        //if it's false, return a 0
      else if(expression[i+1] == 'f') {
        tempBool = 0;
        tempValue->boolValue = tempBool;
        insert(tokens, tempValue);
        i++;
        }
        //otherwise, if the user is trying to input a vector or something, so put in 3
        //which means that we don't deal with that. Don't increment i since the # is not
        //connected to the symbol after it
      else{
        //Set tempBool = 3 which means that it is an improper use
        tempBool = 3; 
        }
    }
    
    //if we hit a number
    else if(isdigit(current) ^ (current == '-' & isdigit(expression[i+1]))){
        int isNegative = 0;
        if(current == '-'){
            isNegative = 1;
            i++;
        }
        int floatPlace = 0;
        int j = i;
        //go through to figure out if it's a float or an integer. do this with the
        //placedholder called floatPlace
        while(isdigit(expression[j])){
            if(expression[j+1] == '.'){
                floatPlace = 1;
                j++;
            }
        j++;
        }
        //if it's an integer:
        if(floatPlace == 0){
            int numLength = 0;
            while(isdigit(expression[i])){
                i++;
                numLength++;
            }
            //set it up and strncpy just like we did for a string
            char *newInteger = malloc(sizeof(char) * (numLength + 1));
            newInteger[numLength] = '\0';
            char* jp = &(expression[i - numLength]);
            strncpy(newInteger, jp, numLength);
            //atoi is a nifty function that converts a string to an integer
            int integer = atoi(newInteger);
            free(newInteger);
            if(isNegative == 1){
                integer = integer*(-1);
            }
            Value* tempValue = createValue();
            enum TOKEN_TYPE newCellType = integerType;
            tempValue->type = newCellType;
            tempValue->intValue = integer;
            insert(tokens, tempValue); 
            //go back one space for i otherwise we'll go too far
            i = i - 1;
        } else { // if it's a float:
        int numLength = 0;
        while(isdigit(expression[i]) | expression[i] == '.') {
            i++;
            numLength++;
        }
        char *newFloat = malloc(sizeof(char) * (numLength+1));
        newFloat[numLength] = '\0';
        char* jp = &(expression[i - numLength]);
        strncpy(newFloat, jp, numLength);
        //use the function atof to make a float. It seems that it's storing the right
        //number when we do this, but when we print it out, it does some funny rounding
        // and changes the last part of the float. Not sure why it does this but it
        //probably won't be a problem when we start doing real math.
        float ourFloat = atof(newFloat);
        free(newFloat);
        if(isNegative == 1){
                ourFloat = ourFloat*(-1.0);
            }
        Value* tempValue = createValue();
        enum TOKEN_TYPE newCellType = floatType;
        tempValue->type = newCellType;
        tempValue->fltValue = ourFloat;
        insert(tokens, tempValue);
        i = i - 1;
        }
    }
    
    //if it's a semi-colon, go to the end of the expression because the rest is a comment
    else if(current == ';'){
    i = strlen(expression);
    }
    
    //if it's a blank space, don't do anything!
    else if(current == ' '){
    // Do nothing!
    }
	
	else if(current == '\t'){
	//Do Nothing if it is a tab!
	}
    //if we've gotten here, it must be a symbol!
    else{
    int symLength = 0; 
    int testCheck = 0;
    //the symbol can include anything but a white space, an open parenthesis, or a closed
    //parenthesis
    while(expression[i] != ' ' ^ expression[i] != '(' ^ expression[i] != ')' & testCheck == 0) {
        //printf("expression %c\n",expression[i]);
        //printf("i = %d\n", i);
        if(expression[i] == '\n') {
        i = strlen(expression);
        testCheck = 1;
        } else {
        symLength++;
        i++; 
        }
    }
    if(i == strlen(expression)) {
        char *newSymbol = malloc(sizeof(char)*(symLength+1));
        char *sp = &(expression[i - 1 - symLength]);
        strncpy(newSymbol, sp, symLength);
        newSymbol[symLength] = '\0';
        Value* tempValue2 = createValue();
        enum TOKEN_TYPE newCellType = symbolType;
        tempValue2->type = newCellType;
        tempValue2->symValue = newSymbol;
        insert(tokens, tempValue2);
    } else{
        char *newSymbol = malloc(sizeof(char)*(symLength+1));
        char *sp = &(expression[i - symLength]);
        strncpy(newSymbol, sp, symLength);
        newSymbol[symLength] = '\0';
        Value* tempValue2 = createValue();
        enum TOKEN_TYPE newCellType = symbolType;
        tempValue2->type = newCellType;
        tempValue2->symValue = newSymbol;
        insert(tokens, tempValue2);
        i = i - 1;
    }
    }
    i++;
  }
  return tokens;
}

