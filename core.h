#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct _Type {
    char *name;
    size_t size;
} Type;

typedef struct _Object *Object;
typedef struct _String *String;
typedef struct _Integer *Integer;

/******************************************************************************/

Type ObjectT;

struct _Object_VTBL {
    Integer (*hash)(void *object);
    bool (*equals)(void *object, Object other);
    String (*to_string)(void *object);
};

#define OBJECT_VTBL \
  { Object_hash, Object_equals, Object_to_string }

struct _Object {
    Type *type;
    struct _Object_VTBL *vptr;
};
typedef struct _Object *Object;

bool Object_is_type(Object this, Type *t); // TODO

struct _Object Object_super(Type *childT, struct _Object_VTBL *vptr);

Integer Object_hash(void *object);

bool Object_equals(void *object, Object other);

String Object_to_string(void *object);

void Object_print(Object this); // TODO

/******************************************************************************/

Type IntegerT;

struct _Integer_VTBL { };

#define INTEGER_VTBL \
  { }

struct _Integer {
    struct _Object super;
    struct _Integer_VTBL *vptr;
    int64_t val;
};
typedef struct _Integer *Integer;

Integer Integer_new(int64_t val);
struct _Integer Integer_super(Type *childT, int64_t val);

void Integer_print(Integer i); // TODO: Get rid of

/******************************************************************************/

Type StringT;

struct _String_VTBL {
    size_t (*len)(void *string);
    char *(*to_bytes)(void *string);
};

#define STRING_VTBL \
  { String_len, String_to_bytes }

struct _String {
    struct _Object super;
    struct _String_VTBL *vptr;
    char *val;
    size_t len;
};

String String_new();
struct _String String_super(Type *childT);
String String_new2(char *bytes);
struct _String String_super2(Type *childT, char *bytes);

Integer String_hash(void *string);
String String_to_string(void *string);
bool String_equals(void *string, Object other);

size_t String_len(void *string);
char *String_to_bytes(void *string);

/******************************************************************************/

void print(Object this);

#endif