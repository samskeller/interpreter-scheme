#include <stdio.h>
#include <stdlib.h>
#include "splitter.h"

LinkedList* parse(LinkedList* tokens, int* depth);

void printValue(Value* value);

enum ERROR_TYPE {
    dummyError, tooManyClosed, tooFewClosed
};


