#ifndef CLONEABLE_H
#define CLONEABLE_H

#include "core.h"

Type CloneableT = {"Cloneable", 0};

struct _Cloneable_VTBL {
    Object (*clone)(void *cloneable);
};

struct _Cloneable {
    struct _Cloneable_VTBL *vptr;
};

typedef struct _Cloneable *Cloneable;


#endif