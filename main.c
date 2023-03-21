#include "core.h"

#include <stdio.h>
#include <stdlib.h>


int main() {
    Integer myInt = Integer_new(30);
    print((Object) myInt);
    Object o = malloc(sizeof(*o));
    Type t = {"Testing", 1};
    o->type = &t;
    // Object_print(o);
    print(o);
    return 0;
}