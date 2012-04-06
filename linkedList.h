/* linkedlist.h

This is our header file that contains the headers for a linked list.

Written by Sam Keller, Will Herold, and Simon Montague.
*/

#include <stdio.h>
#include <stdlib.h>

struct __LinkedList* globalList;



// these are all of our different types of values. this stores the type and the value
typedef struct __Value {
    int type; /* Make sure type includes a consType or some such */
    union {
        int boolValue;
        int intValue;
        float fltValue;
        char* strValue;
        char* symValue;
        char openValue;
        char closedValue;
        struct __ConsCell* cons;
        double dblValue;
        struct __Closure* closureValue;
        /* primitiveValue is a pointer to a function that takes
         a Value* as input and returns a Value* as output.*/
        struct __Value* (*primitiveValue)(struct __Value*);
        struct __LinkedList* listValue;
        struct __Binding* bindingValue;
        int errorValue;
        int blankValue;
        struct __Value* valueValue;
        struct __environment* envValue;
    };
} Value;

//our cons cell has a car and a cdr.
typedef struct __ConsCell {
    struct __Value *car;
    struct __ConsCell *cdr;
} ConsCell;


// our linked list contains a pointer to a cons cell to start with.
typedef struct __LinkedList 
{
  ConsCell* head;
} LinkedList;

// all of our types of values
enum TOKEN_TYPE {
    booleanType, integerType, floatType, stringType, symbolType, openType, closedType, consType, 
    doubleType, closureType, primitiveType, bindingType, errorType, blankType, valueType, environmentType, listType
};


void init(LinkedList *list);

void insert(LinkedList *list, Value *value);
void memoryInsert(LinkedList* myList, Value* dummyValue);

void print(LinkedList *list);

LinkedList* ReverseIt(LinkedList *list);

Value* pop(LinkedList *list);

LinkedList* append(LinkedList *leftOver, LinkedList *mainList);
Value* createValue();
LinkedList* createList();

void freeValue(Value* value);

void globalCleanup(LinkedList* globalList);

