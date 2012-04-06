/*
Written by Sam Keller, Simon Montague, and Will Herold

Written on 11/01/2011

This program sets up our interpreter, which interprets the parse tree given from our parser
*/

#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"
#include <math.h>

//initEnv initializes a given evironment by initializing its linked list and setting its
// parent to be NULL
void initEnv(Environment* env){
    init(env->contents);
    env->parent=NULL;
}

// this function creates an environment and sets the parent to the given one. 
Environment* createEnvironment(Environment* parentEnv){
    Environment* newEnv = malloc(sizeof(Environment));
    newEnv->parent = parentEnv;
    newEnv->contents = createList();
    init(newEnv->contents);
    
    //Put it in the globalList
    Value* memValue = malloc(sizeof(Value));
    memValue->type = environmentType;
    memValue->envValue = newEnv;
    memoryInsert(globalList, memValue);
    return newEnv;
}

// envLookup intakes a given symbol and an environment and looks for that symbol within
// the environment
Value* envLookup(char* symbol, Environment* env){
    ConsCell* current = (env->contents)->head;
    //While we still have contents in the environment to look through:
    while(current != NULL){   
        // if the symvalue of our current is equal to the given symbol
        if(strcmp(current->car->bindingValue->name, symbol) == 0){
            // return the value from the binding 
            Value* returnThing = current->car->bindingValue->value;
            return returnThing;
        }
        current = current->cdr;
    }
    
    
    //if we didn't find it and there's a parent environment, check there
    if(env->parent !=NULL){
        return envLookup(symbol, env->parent);
    }
    
    //If after everything we dont find a value, return a null value
    Value* blank = createValue();
    blank->type = blankType;
    blank->blankValue = 0;
    return blank;
}

// apply applies a given function to a bunch of arguments
Value *apply(Value *function, Value *actualArgs){
    // if the function is a primitive type, just return the result of using the function on the 
    // arguments
    if (function->type == primitiveType){
        return function->primitiveValue(actualArgs);
        
    // if the given value is a closure type, then it's a user-defined function
    } else if (function->type == closureType){
        Value *formalArgs = function->closureValue->params;
        
        Environment* frame = createEnvironment(function->closureValue->environment);
        
        ConsCell* paramPtr = formalArgs->cons;
        ConsCell* actualArgsPtr = actualArgs->cons;
        // while we're going through the parameters for the function:
        // bind the actual values to the parameters in the current environment.
        while(paramPtr != NULL){
            Binding* newBinding = malloc(sizeof(Binding));
            newBinding->name = paramPtr->car->symValue;
            newBinding->value = actualArgsPtr->car;
            Value* newValue = createValue();
            newValue->type = bindingType;
            newValue->bindingValue = newBinding;
            // insert this new value (containing the binding) into the environment
            insert(frame->contents, newValue);
            paramPtr = paramPtr->cdr;
            actualArgsPtr= actualArgsPtr->cdr;
        }
        
        // return the evaluation of that function with those values
        return eval(function->closureValue->parseTree, frame);
    // if something's gone wrong... return a blank value.
    } else{
        Value* blank = createValue();
        blank->type = blankType;
        blank->blankValue = 0;
        return blank;
    }
}

// evalDefine binds the first argument to the rest of the arguments given, which defines
// the first value 
void evalDefine(Value* args, Environment* env){
    Binding* newBinding = malloc(sizeof(Binding));
    newBinding->name = ((args->cons)->car)->symValue;
    newBinding->value = eval(cdr(args)->cons->car, env); 
    Value* newValue = createValue();
    newValue->type = bindingType;
    newValue->bindingValue = newBinding;
    // insert this new value (containing the binding) into the environment
    insert(env->contents, newValue);
    
}

// setEvalDefine is just like evalDefine but it works for set-- this means it calls eval
// in the root environment.
void setEvalDefine(Value* args, Environment* currentEnv, Environment* rootEnv){
    Binding* newBinding = malloc(sizeof(Binding));
    newBinding->name = ((args->cons)->car)->symValue;
    newBinding->value = eval(cdr(args)->cons->car, rootEnv); 
    Value* newValue = createValue();
    newValue->type = bindingType;
    newValue->bindingValue = newBinding;
    // insert this new value (containing the binding) into the current environment
    insert(currentEnv->contents, newValue);
}


// letRecEvalDefine also works like evalDefine but for letRec. It checks to see if looking up
// the symbol returns a blank type-- if it does, it's already been declared so it returns an error.
// If it returns something else, then make the new binding in the given environment
void letRecEvalDefine(Value* args, Environment* env){
    Value* checkIfInEnv = envLookup(args->cons->car->symValue, env);
    if(checkIfInEnv->type != blankType){
        printf("ERROR: Duplicate identifier %s\n",args->cons->car->symValue);
		exit(EXIT_FAILURE);
    }else{
        Binding* newBinding = malloc(sizeof(Binding));
        newBinding->name = ((args->cons)->car)->symValue;
        newBinding->value = eval(cdr(args)->cons->car, env); 
        Value* newValue = createValue();
        newValue->type = bindingType;
        newValue->bindingValue = newBinding;
        // insert this new value (containing the binding) into the environment
        insert(env->contents, newValue);
    }
}

// this is the eval define but for when we use let. we pass in a sub environment
// too and insert the new bindings into the sub environment
void letEvalDefine(Value* args, Environment* env, Environment* subEnv){
    Value* checkIfInEnv = envLookup(args->cons->car->symValue, subEnv);
    if(checkIfInEnv->type != blankType){
        printf("ERROR: Duplicate identifier %s\n",args->cons->car->symValue);
		exit(EXIT_FAILURE);
    }else{
        Binding* newBinding = malloc(sizeof(Binding));
        newBinding->name = ((args->cons)->car)->symValue;
        newBinding->value = eval(cdr(args)->cons->car, env); 
        Value* newValue = createValue();
        newValue->type = bindingType;
        newValue->bindingValue = newBinding;
        // insert this new value (containing the binding) into the sub-environment
        insert(subEnv->contents, newValue);
    }
    
}

// letStarEvalDefine 
void letStarEvalDefine(Value* args, Environment* env, Environment* subEnv){
    Binding* newBinding = malloc(sizeof(Binding));
    newBinding->name = ((args->cons)->car)->symValue;
    newBinding->value = eval(cdr(args)->cons->car, env); 
    Value* newValue = createValue();
    newValue->type = bindingType;
    newValue->bindingValue = newBinding;
    // insert this new value (containing the binding) into the environment
    insert(subEnv->contents, newValue);    
}



// eval is our main function. It takes in a parse tree and evaluates it!
Value* eval(Value* expr, Environment* env){
    //If the expression is just an integer, boolean or string, return it
    if(expr->type == integerType ^ expr->type == booleanType ^ expr->type == stringType ^ expr->type == floatType){
        return expr;
	
	//Otherwise, if the expression is a symbol, try to look it up in the environment, and return either that or just the symbol if it isnt defined somewhere.
	}else if(expr->type == symbolType){
		//If the expression is just a symbol, we want to see if it has been defined
		//in the environment, and if it has return its value there, otherwise just
		//return it as is.
		if(envLookup(expr->symValue, env)->type != blankType){
			return envLookup(expr->symValue, env);
		}else{
		    printf("ERROR: Symbol %s undefined\n",expr->symValue);
			Value* blank = createValue();
            blank->type = blankType;
            blank->blankValue = 0;
            return blank;
		}
	
    // If the expression is a list (probably a function), do this
    }else if(expr->cons->cdr !=NULL){
        Value* operator = expr->cons->car;
        Value* args = cdr(expr);
		
		if(operator->type == consType){
            Value* evaledOperator = eval(operator, env);
            //Evaluate the arguments in case they contain functions as well
            //NOTE: If there are no fcns in args, then args won't change even
            //		though we run evalArgs on it
            Value* evaldArgs = evalArgs(args, env); 
            return apply(evaledOperator, evaldArgs);
		}
		
        else if(operator->type == symbolType){ //If the operator is a symbol
            if(strcmp(operator->symValue, "define") == 0){ // implement DEFINE
                //Create a new binding and store it in the enviroment list
                evalDefine(args, env);
                Value* blank = createValue();
                blank->type = blankType;
                blank->blankValue = 0;
                return blank;
            } 
            else if(strcmp(operator->symValue, "if") == 0) { // implement IF
                //If nothing comes after the IF (i.e no test or args), return an error
				if(args->cons ==NULL){
					printf("Invalid syntax for IF statement");
                    Value* invalid = createValue();
                    invalid->type = errorType;
                    invalid->errorValue = 0;
                    return invalid;
				}
				//Create a value of our test
				Value* test = args->cons->car;
                
				//If there is only the test and no actions to perform after, return an error
                if(args->cons->cdr == NULL){
                    printf("Invalid syntax for IF statement");
                    Value* invalid = createValue();
                    invalid->type = errorType;
                    invalid->errorValue = 0;
                    return invalid;
                }
                // if the test is true, return the first thing, if it's false, return the second thing
                if(eval(test, env)->intValue == 1) {
                    Value* consequent = args->cons->cdr->car;
					
					//Create a value which will contain the evaluated consequent
					Value* consqEval = createValue();
					consqEval = eval(consequent, env);
					return consqEval; 
                   }
                else {
					//If the alternative exists, and the test calls for us to return it, return it
                    if(args->cons->cdr->cdr != NULL){
						Value* alternative = args->cons->cdr->cdr->car;
					
						//Create a value which will contain the evaluated alternative
						Value* altEval = createValue();
						altEval = eval(alternative, env);
						return altEval; 
                    }
                    // technically this isn't an error, but we want to return a value that will never
                    // get used
                    else {
                        Value* invalid = createValue();
                        invalid->type = errorType;
                        invalid->errorValue = 0;
                        return invalid;
                    }
                }
            }//End of where we define if
            
            
            else if(strcmp(operator->symValue, "quote") == 0) { // implement QUOTE
				if(args->cons->car->type != consType){//If the args of quote are not contained in a scheme list (surrounded by parenthesis) 
					printf("SYNTAX ERROR: Something's wrong with quote\n");
					Value* blank = createValue();
                    blank->type = blankType;
                    blank->blankValue = 0;
                    return blank;
				}else{
					return args->cons->car;
				}
            }
            
            // load simply calls our load function
            else if(strcmp(operator->symValue, "load") == 0) { // implement LOAD
                load(args, env);
                Value* blank = createValue();
                blank->type = blankType;
                blank->blankValue = 0;
                return blank;
            }
            
            // letrec creates a new subenvironment and then defines the arguments in this new
            // sub environment
			else if(strcmp(operator->symValue, "letrec") == 0) { // implement LETREC
				//In this case, args will contain a list of definitions and then the rest of the expression
				Value* defns = args->cons->car; //Value containing list of definitions
				if(args->cons->cdr != NULL){
                    Value* body = args->cons->cdr->car; //Value containing a pointer to the rest of the program's body
                    Environment* subEnv = createEnvironment(env);
                    ConsCell* definition = defns->cons;
                    while(definition != NULL){
                        Value* defArgs = createValue();
                        defArgs->type = consType;
                        defArgs->cons = definition->car->cons;
                        letRecEvalDefine(defArgs, subEnv);
                        definition= definition->cdr;
                        //free(defArgs);
                    }
                    return eval(body, subEnv);
                }else{
                   printf("SYNTAX ERROR: LetRec - Bad Syntax\n");
					Value* blank = createValue();
                    blank->type = blankType;
                    blank->blankValue = 0;
                    return blank;
                }
            }
            
            else if(strcmp(operator->symValue, "let") == 0) { // implement LET 
				//In this case, args will contain a list of definitions and then the rest of the expression
				// the only difference with letrec is that we call letEvalDefine instead of evalDefine
				Value* defns = args->cons->car; //Value containing list of definitions
	            if(args->cons->cdr != NULL){
                    Value* body = args->cons->cdr->car; //Value containing a pointer to the rest of the program's body
                    Environment* subEnv = createEnvironment(env);
                    ConsCell* definition = defns->cons;
                    while(definition != NULL){
                        Value* defArgs = createValue();
                        defArgs->type = consType;
                        defArgs->cons = definition->car->cons;
                        letEvalDefine(defArgs, env, subEnv);
                        definition= definition->cdr;
                        //free(defArgs);
                    }
                
                    return eval(body, subEnv);

                }else{
                   printf("SYNTAX ERROR: Let - Bad Syntax\n");
					Value* blank = createValue();
                    blank->type = blankType;
                    blank->blankValue = 0;
                    return blank;
                }
            }
            else if(strcmp(operator->symValue, "let*") == 0) { // implement LET*
                Value* defns = args->cons->car; //Value containing list of definitions
	            if(args->cons->cdr != NULL){
                    Value* body = args->cons->cdr->car; //Value containing a pointer to the rest of the program's body
                    Environment* currentEnv = env;
                    ConsCell* definition = defns->cons;
                    while(definition != NULL){
                        Environment* subEnv = createEnvironment(currentEnv);
                        Value* defArgs = createValue();
                        defArgs->type = consType;
                        defArgs->cons = definition->car->cons;
                        letStarEvalDefine(defArgs, currentEnv, subEnv);
                        definition= definition->cdr;
                        //free(defArgs);
                        currentEnv = subEnv;
                    }
                    return eval(body, currentEnv);

                }else{
                   printf("SYNTAX ERROR: Let* - Bad Syntax\n");
					Value* blank = createValue();
                    blank->type = blankType;
                    blank->blankValue = 0;
                    return blank;
                }
            
            }
            else if(strcmp(operator->symValue, "lambda") == 0) { // implement Lambda
                // here we make a new Closure and set up the parameters and the parseTree from the
                // list of arguments. Then we make a value and return this value
                Value* variables = args->cons->car;
                Value* body = args->cons->cdr->car;
                Closure* newClosure = malloc(sizeof(Closure));
                newClosure->params = variables;
                newClosure->parseTree = body;
                newClosure->environment = env;
                Value* closureContainer = createValue();
                closureContainer->type = closureType;
                closureContainer->closureValue = newClosure;
                
                return closureContainer;
            
            }
            
            // if we hit "and", we basically step through all of the arguments and check to see
            // if they're true or not. If a single one of them returns a 0, meaning it's false,
            // then we set our intValue of our return value to be 0. Otherwise return 1
            else if(strcmp(operator->symValue, "and") == 0) { // implement AND
                ConsCell* current = args->cons;
                Value* result = createValue();
                result->type = integerType;
                while(current != NULL){
                    Value* evaluated = eval(current->car, env);
                    if(evaluated->intValue == 0){
                        result->intValue = 0;
                        return result;
                    }
                    current = current->cdr;
                }
                result->intValue = 1;
                return result;
            }
            
            // the or works very similarly to and except that it will always return 
            // an int value of 0 unless it comes across an argument that returns true.
            else if(strcmp(operator->symValue, "or") == 0) { // implement AND
                ConsCell* current = args->cons;
                Value* result = createValue();
                
                result->type = integerType;
                while(current != NULL){
                    Value* evaluated = eval(current->car, env);
                    if(evaluated->intValue == 1){
                        result->intValue = 1;
                        return result;
                    }
                    current = current->cdr;
                }
                result->intValue = 0;
                return result;
            }
            
            // if we hit "cond", step through each of the if statements we're given. If
            // any of them return 1 while we're stepping through, we evaluate the next thing in
            // that if statement.
            else if(strcmp(operator->symValue, "cond") == 0) { // implement COND
                ConsCell* current = args->cons;
                Value* currentEval;
                Value* evaluated = createValue();
                while(current->cdr != NULL){
                    currentEval = eval(current->car->cons->car, env);
                    if(currentEval->intValue == 1){
                        evaluated = eval(current->car->cons->cdr->car, env);
                        return evaluated;
                    }
                    current = current->cdr;
                }
                // If we're at the last item, check to see if it's an else. If it is, do whatever
                // comes next. Otherwise, check it like normal.
                if(strcmp(current->car->cons->car->symValue, "else") == 0){
                    evaluated = eval(current->car->cons->cdr->car, env);
                    return evaluated;
                } else{
                    currentEval = eval(current->car->cons->car, env);
                    if(currentEval->intValue == 1){
                        evaluated = eval(current->car->cons->cdr->car, env);
                        return evaluated;
                    }
                }
                // if none of the statements were true, return a blank type cause nothing happens
                Value* blank = createValue();
                blank->type = blankType;
                blank->blankValue = 0;
                return blank;
            }
            // if we hit "set!", then we want to ogo through every environment and set the arguments
            // to whatever they're supposed to be. This makes the variable given bound in every
            // possible environment so no matter where we look, we'll find this binding
            else if(strcmp(operator->symValue, "set!") == 0) { // implement set!
                Environment* currentEnv = env;
                Environment* subEnv = env;
                evalDefine(args, env);
                while(currentEnv != NULL) {
                    setEvalDefine(args, currentEnv, subEnv);
                    currentEnv = currentEnv->parent;
                }
                Value* blank = createValue();
                blank->type = blankType;
                blank->blankValue = 0;
                return blank;
            }
            else {
				//Evaluate the arguments in case they contain functions as well
				//NOTE: If there are no fcns in args, then args won't change even
				//		though we run evalArgs on it
				Value* evaledArgs = evalArgs(args, env); 
				Value* evaledOperator = eval(operator, env);
				return apply(evaledOperator, evaledArgs);
            }
        }
    
    }else{
    // implement Literals (and assigned variables)
        // if it's a symbol, look it up
        if(expr->cons->car->type == symbolType){
            Value* lookup = envLookup(expr->cons->car->symValue, env);
            if(lookup->type == closureType){
                Value* emptyArgs = createValue();
                emptyArgs->type = consType;
                emptyArgs->cons = NULL;
                return apply(lookup, emptyArgs);
            }else if(lookup->type != blankType){
                return lookup;
            }else{
                printf("ERROR: The symbol %s is not yet defined\n", expr->cons->car->symValue);
                Value* blank = createValue();
                blank->type = blankType;
                blank->blankValue = 0;
                return blank;
            }
        //If it is just a function in the list
       }else if(expr->cons->car->type == consType){
            Value* emptyArgs = createValue();
            emptyArgs->type = consType;
            emptyArgs->cons = NULL;
            return apply(eval(expr->cons->car, env), emptyArgs);            
       // if it's something like an integer, just return that integer
       }else{  
            return expr;
        }
    }
}



// makePrimitiveValue makes a primitive function like addition or subtraction
Value *makePrimitiveValue(Value* (*f)(Value *)) {
   Value *resultValue = createValue();
   resultValue->type = primitiveType;
   resultValue->primitiveValue = (*f);
   return resultValue;
}

// the bind functions binds a given symbol to a given function in our environment
void bind(char* sym, Value* func, Environment* env) {
    Binding* newBinding = malloc(sizeof(Binding));
    newBinding->name = sym;
    newBinding->value = func;
    Value* envValue = createValue();
    envValue->type = bindingType;
    envValue->bindingValue = newBinding;
    // insert this new binding into the environment
    insert(env->contents, envValue);
}

// add is our primitive function for adding
Value* add(Value* args){
    float sum = 0;
    ConsCell* current = args->cons;
    // add every element to the current sum
    while(current != NULL){
        // don't know whether it's an int or a float
        if(current->car->type == integerType){
            sum = sum + current->car->intValue;
        } else if (current->car->type == floatType){
            sum = sum + current->car->fltValue;
        }
        current= current->cdr;
    }
    // if the total could be an int, make it into one
    if(fmod(sum,1.0) == 0){
        int newSum = sum;
        Value* result = createValue();
        enum TOKEN_TYPE resultType = integerType;
        result->type = resultType;
        result->intValue = newSum;
        return result;
    } else{
        Value* result = createValue();
        enum TOKEN_TYPE resultType = floatType;
        result->type = resultType;
        result->fltValue = sum;
        return result;
    }
}

// lessThan is our primitive function for determining if one number is less than another
Value* lessThanEquals(Value* args) {
    // we're assuming we only have two values given
    float x;
    float y;
    if(args->cons->car->type == integerType){
        x = args->cons->car->intValue;
    } else if (args->cons->car->type == floatType){
        x = args->cons->car->fltValue;
    }
    if(args->cons->cdr->car->type == integerType){
        y = args->cons->cdr->car->intValue;
    } else if (args->cons->cdr->car->type == floatType){
        y = args->cons->cdr->car->fltValue;
    }
    Value* result = createValue();
    result->type = booleanType;

    if(x <= y) {
        result->boolValue = 1;
        return result; }
    else {
        result->boolValue = 0;
        return result; }
}

// greater than is our primitive function for determining if one number is greater than another
Value* greaterThanEquals(Value* args) {
    // we're assuming we only have two values given
    float x;
    float y;
    if(args->cons->car->type == integerType){
        x = args->cons->car->intValue;
    } else if (args->cons->car->type == floatType){
        x = args->cons->car->fltValue;
    }
    if(args->cons->cdr->car->type == integerType){
        y = args->cons->cdr->car->intValue;
    } else if (args->cons->cdr->car->type == floatType){
        y = args->cons->cdr->car->fltValue;
    }
    Value* result = createValue();
    result->type = booleanType;

    if(x >= y) {
        result->boolValue = 1;
        return result; }
    else {
        result->boolValue = 0;
        return result; }
}

Value* greaterThan(Value* args) {
    // we're assuming we only have two values given
    float x;
    float y;
    if(args->cons->car->type == integerType){
        x = args->cons->car->intValue;
    } else if (args->cons->car->type == floatType){
        x = args->cons->car->fltValue;
    }
    if(args->cons->cdr->car->type == integerType){
        y = args->cons->cdr->car->intValue;
    } else if (args->cons->cdr->car->type == floatType){
        y = args->cons->cdr->car->fltValue;
    }
    Value* result = createValue();
    result->type = booleanType;

    if(x > y) {
        result->boolValue = 1;
        return result; }
    else {
        result->boolValue = 0;
        return result; }
}

Value* lessThan(Value* args) {
    // we're assuming we only have two values given
    float x;
    float y;
    if(args->cons->car->type == integerType){
        x = args->cons->car->intValue;
    } else if (args->cons->car->type == floatType){
        x = args->cons->car->fltValue;
    }
    if(args->cons->cdr->car->type == integerType){
        y = args->cons->cdr->car->intValue;
    } else if (args->cons->cdr->car->type == floatType){
        y = args->cons->cdr->car->fltValue;
    }
    Value* result = createValue();
    result->type = booleanType;

    if(x < y) {
        result->boolValue = 1;
        return result; }
    else {
        result->boolValue = 0;
        return result; }
}

// subtract is our primitive function to subtract a bunch of numbers from one number
Value* subtract(Value* args){
    ConsCell* current = args->cons;
    float sum;
    // don't know whether it's a float or an int
    if(current->car->type == integerType){
        sum = current->car->intValue;
    } else if (current->car->type == floatType){
        sum = current->car->fltValue;
    }
    current = current->cdr;
    // add every element to the current sum
    while(current != NULL){
        if(current->car->type == integerType){
            sum = sum - current->car->intValue;
        } else if (current->car->type == floatType){
            sum = sum - current->car->fltValue;
        }
        current= current->cdr;
    }
    // if the total could be an int, make it into one
    if(fmod(sum,1.0) == 0){
        int newSum = sum;
        Value* result = createValue();
        enum TOKEN_TYPE resultType = integerType;
        result->type = resultType;
        result->intValue = newSum;
        return result;
    } else{
        Value* result = createValue();
        enum TOKEN_TYPE resultType = floatType;
        result->type = resultType;
        result->fltValue = sum;
        return result;
    }
}

// this is a cdr function written for use in the behind the scenes C code. There is a seperate one for use in evaluating scheme.
//The two are split mostly for ease of debugging since this version was not fully working on the scheme code and so much of the
//interpreter structure relied on this particular version of the function.
Value* cdr(Value* list){
    Value* cdrValue = createValue();
    cdrValue->type = consType;
    cdrValue->cons = list->cons->cdr;
    return cdrValue;
}

//This is the version of cdr that is used in the evaluation of scheme code. 
Value* schemeCdr(Value* list){
    Value* cdrValue = createValue();
	//printValue(list->cons->car);
    cdrValue->type = consType;
	//For some reason, the Value pointing to the first cons cell in the list is nested in another value. 
    cdrValue->cons = list->cons->car->cons->cdr;
    return cdrValue;
}

// returns the car of a list. 
Value* car(Value* list){
    if(list->cons->car != NULL){
	    Value* carValue = list->cons->car->cons->car;
	    return carValue;
	} else {
	    Value* blank = createValue();
        blank->type = blankType;
        blank->blankValue = 0;
        return blank;
	}
}

//Cons function. Takes the first item in args and adds it to the list supplied by the second argument.
Value* cons(Value* args){
	Value* arg1 = args->cons->car;
	Value* arg2 = args->cons->cdr->car;
	if(arg2->type == consType){ //The case where the second argument is a list, we want to add arg1 to that list
		LinkedList* returnList = createList();
		init(returnList);
		returnList->head = arg2->cons;
		insert(returnList, arg1);
		//Create and return a value holding this list
		Value* returnValue = createValue();
		returnValue->type = consType;
		returnValue->cons = returnList->head;
		//Now that we have the return value set up, free the linked list struct
		//free(returnList);
		return returnValue;
	} else {//if arg2 isnt a list
		LinkedList* returnList = createList();
		init(returnList);
		insert(returnList, arg1);
		insert(returnList, arg2);
		returnList = ReverseIt(returnList); //Reverse the list since it is currently backwards (because we insert at the front)
		//Create and return a value holding this list
		Value* returnValue = createValue();
		returnValue->type = consType;
		returnValue->cons = returnList->head;
		//Now that we have the return value set up, free the linked list struct
		//free(returnList);
		return returnValue;
	}
}

// returns the first number divided by all of the following numbers.
Value* divide(Value* args){
    ConsCell* current = args->cons;
    float total;
    if(current->car->type == integerType){
        total = current->car->intValue;
    }
    if(current->car->type == floatType){
        total = current->car->fltValue;
    }
    current = current->cdr;
    // divide every element from the first one
    while(current != NULL){
        if(current->car->type == integerType){
            total = total / current->car->intValue;
        }
        if(current->car->type == floatType){
            total = total / current->car->fltValue;
        }
        current= current->cdr;
    }
    // if the total could be an int, make it into one
    if(fmod(total,1.0) == 0){
        int newTotal = total;
        Value* result = createValue();
        enum TOKEN_TYPE resultType = integerType;
        result->type = resultType;
        result->intValue = newTotal;
        return result;
    } else{
        Value* result = createValue();
        enum TOKEN_TYPE resultType = floatType;
        result->type = resultType;
        result->fltValue = total;
        return result;
    }
}

// returns the first number multiplied by all of the following ones.
Value* multiply(Value* args){
    ConsCell* current = args->cons;
    float total;
    if(current->car->type == integerType){
        total = current->car->intValue;
    } else if(current->car->type == floatType){
        total = current->car->fltValue;
    }
    current = current->cdr;
    // multiply every element from the first one
    while(current != NULL){
        if(current->car->type == integerType){
            total = total * current->car->intValue;
        } else if(current->car->type == floatType){
            total = total * current->car->fltValue;
        }
        current = current->cdr;
    }
    // if the total could be an int, make it into one
    if(fmod(total,1.0) == 0){
        int newTotal = total;
        Value* result = createValue();
        enum TOKEN_TYPE resultType = integerType;
        result->type = resultType;
        result->intValue = newTotal;
        return result;
    } else{
        Value* result = createValue();
        enum TOKEN_TYPE resultType = floatType;
        result->type = resultType;
        result->fltValue = total;
        return result;
    }
    
}

// This function compares two numbers and returns 1 if they're the same, 0 if they're not
Value* equals(Value* args){ //Args will be a value containing a pointer to a cons cell. We are comparing the first two items in this "list"
	ConsCell* arg1 = args->cons;
	ConsCell* arg2 = arg1->cdr;
	Value* result = createValue(); //The value we will be returning
    result->type = booleanType;
    
    int outcome;
    // if they're both integers, compare them
    if(arg1->car->type == integerType & arg2->car->type == integerType){
        if(arg1->car->intValue == arg2->car->intValue){
            outcome = 1;
        } else{
            outcome = 0;
        }
    }
    // if one's a float and one's an int, compare, etc.
    if(arg1->car->type == floatType & arg2->car->type == integerType){
        if(arg1->car->fltValue == arg2->car->intValue){
            outcome = 1;
        } else{
            outcome = 0;
        }
    }
    
    //etc
    if(arg1->car->type == integerType & arg2->car->type == floatType){
        if(arg1->car->intValue == arg2->car->fltValue){
            outcome = 1;
        } else{
            outcome = 0;
        }
    }
    //etc
    if(arg1->car->type == floatType & arg2->car->type == floatType){
        if(arg1->car->fltValue == arg2->car->fltValue){
            outcome = 1;
        } else{
            outcome = 0;
        }
    }
    
    result->boolValue = outcome;

	return result;
}

//Checks to see if the arguments are Null
Value* isNull(Value* args){
    Value* result = createValue();
    result->type = booleanType;
    if(args->cons->car->cons == NULL){
        result->boolValue = 1;
        return result;
    }else{
        result->boolValue = 0;
        return result;
    }
}


// evalArgs is our function for evaluating a list of arguments
Value* evalArgs(Value* args, Environment* env){
	LinkedList* evalList = createList();
	init(evalList);
	//Set current to be the first cons cell in preperation for list traversal
	ConsCell* current = args->cons; 
	while(current != NULL) {
		Value* evaldArg = eval(current->car, env);
		insert(evalList, evaldArg);
		current = current->cdr;
	}
	
	//We need to reverse the list so that it is in the proper order
	LinkedList* reversedList = ReverseIt(evalList);
	
	Value* returnValue = createValue();
	returnValue->type = consType;
	returnValue->cons = reversedList->head;
	ConsCell* current1 = returnValue->cons;

	return returnValue;
}

Value* load(Value* args, Environment* env){
    // we pretty much just run the same code that's in our interpTest file
    FILE *fp;
    char* loadFile = args->cons->car->strValue;
    fp=fopen(loadFile, "r");
    
    LinkedList* loadTokens = createList();
    init(loadTokens);
    LinkedList* loadTokenized = createList();
    init(loadTokenized);
    LinkedList* loadParseTree = createList();
    init(loadParseTree);
    LinkedList* loadLeftoverTokens = NULL;
    int loadDepth = 0;
    int loadDepthCopy = 0;
    char* loadExpression = malloc(256 * sizeof(char));
    while (fgets(loadExpression, 256, fp)){
        loadTokenized = tokenize(loadExpression);
        if(loadTokenized->head != NULL){
            loadTokenized = ReverseIt(loadTokenized);
            loadTokens = append(loadLeftoverTokens, loadTokenized);        
            loadParseTree = parse(loadTokens, &loadDepth); 
            loadParseTree = ReverseIt(loadParseTree);
            
            //if the depth is less than zero, return a closed email
            if (loadDepth < 0) {
                 printf("syntax error: too many closed parens\n");   // Too many close parentheses.
                 enum ERROR_TYPE closedError = tooManyClosed;
                Value* error = createValue();
                error->type = errorType;
                error->errorValue = closedError;
                return error;
            // if the depth is greater than zero, keep going
            } else if (loadDepth > 0) {
                // There are more open parens than close parens, so these loadTokens are saved as leftovers. 
                loadLeftoverTokens = loadTokens;
                loadDepthCopy = loadDepth;
                loadDepth = 0; 
            }else {
                loadDepthCopy = 0;
                loadLeftoverTokens = NULL;
            //The interpreter part starts here
                ConsCell* current = loadParseTree->head;

                while(current != NULL){
                        Value* finalParse = current->car;//THIS IS THE VALUE FOR THE PARSE TREE
                        // evalute the parse tree! and then print it and free some things.
                        Value* evaluation = eval(finalParse, env);
                        if(evaluation->type != blankType){
                            printValue(evaluation);
                            printf("\n");
                        }
                        current = current->cdr;
                }
            }
        }        
    }
    free(loadExpression);
    
    Value* blank = createValue();
    blank->type = blankType;
    blank->blankValue = 0;
    return blank;
}

