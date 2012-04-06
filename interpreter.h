#include <stdio.h>
#include <stdlib.h>
#include "parser.h"


typedef struct __Binding {
    char* name;
    struct __Value* value;
}Binding;

typedef struct __environment {
    LinkedList* contents;
    struct __environment* parent;
}Environment;

typedef struct __Closure {
    Value* params;
    Value* parseTree;
    Environment* environment; //Enviroment pointer
}Closure;

Value* envLookup(char* symbol, Environment* env);
void evalDefine(Value* args, Environment* env);
void letRecEvalDefine(Value* args, Environment* env);
void evalDefine(Value* args, Environment* env);
void letStarEvalDefine(Value* args, Environment* env, Environment* subEnv);


void initEnv(Environment* env);

Environment* createEnvironment(Environment* parentEnv);


Value *makePrimitiveValue(Value* (*f)(Value *));

void bind(char* sym, Value* func, Environment* environ); 

Value *apply(Value *function, Value *actualArgs);
Value *eval(Value *expr, Environment *env);

//Evaluate the arguments of a function, returns a Value of type consCell that points to the first cons-cell in the list of evald args.
Value* evalArgs(Value* args, Environment* env);

//Math Functions
//Add takes in a value containing a list of numbers and adds them all together. 
//Returns a value with the answer
Value* add(Value* args);
Value* subtract(Value* args);
Value* lessThanEquals(Value* args);
Value* greaterThanEquals(Value* args);
Value* greaterThan(Value* args);
Value* lessThan(Value* args);
Value* equals(Value* args);
Value* cdr(Value* list);
Value* schemeCdr(Value* list);
Value* car(Value* list);
Value* cons(Value* args);
Value* divide(Value* args);
Value* multiply(Value* args);
Value* isNull(Value* args);

Value* load(Value* args, Environment* env);
