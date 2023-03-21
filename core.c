#include "core.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/

static struct _Object_VTBL Object_vtbl = OBJECT_VTBL;

Type ObjectT = {"Object", sizeof(struct _Object)};

Object Object_new() {
    Object new = malloc(sizeof(*new));
    new->type = &ObjectT;
    new->vptr = &Object_vtbl;
}

struct _Object Object_super(Type *childT, struct _Object_VTBL *vptr) {
    return (struct _Object){childT, vptr};
}

bool Object_is_type(Object this, Type *t) {
    // TODO: Function vs method
    return this->type == t;
}

Integer Object_hash(void *object) {
    // TODO: uint64_t when access to Linux
    return Integer_new((uint64_t) object);
}

bool Object_equals(void *object, Object other) {
    // Objects are considered equivalent if they point to the same address
    return object == other;
}

String Object_to_string(void *object) {
    // TODO : JAVA getClass().getName() + '@' + Integer.toHexString(hashCode())
    Object this = (Object) object;
    char *str = malloc(strlen(this->type->name) + 100);
    strcpy(str, this->type->name);
    strcpy(str, " at ");
    Integer address = Integer_new((uint64_t) this);
    strcpy(str, String_to_bytes(((Object) address)->vptr->to_string((Object) address)));
    return String_new2(str);
}

/******************************************************************************/

static struct _Object_VTBL Integer_Object_vtbl = OBJECT_VTBL;

static struct _Integer_VTBL Integer_vtbl = INTEGER_VTBL;

Type IntegerT = {"Integer", sizeof(struct _Integer)};

Integer Integer_new(int64_t val) {
    Integer new = malloc(sizeof(*new));
    new->super = Object_super(&IntegerT, &Integer_Object_vtbl);
    new->val = val;
    return new;
}

struct _Integer Integer_super(Type *childT, int64_t val) {
    // TODO: Allow child classes to override Object methods
    return (struct _Integer){Object_super(childT, &Integer_Object_vtbl), &Integer_vtbl,
          val};
}

/******************************************************************************/

#define Object_hash String_hash
#define Object_equals String_equals
#define Object_to_string String_to_string
static struct _Object_VTBL String_Object_vtbl = OBJECT_VTBL;
#undef Object_hash
#undef Object_equals
#undef Object_to_string
static struct _String_VTBL String_vtbl = STRING_VTBL;

Type StringT = {"String", sizeof(struct _String)};

String String_new() {
    String new = malloc(sizeof(*new));
    new->super = Object_super(&StringT, &String_Object_vtbl);
    new->val = "";
    new->len = 0;
    return new;
}

struct _String String_super(Type *childT) {
    // TODO: Allow child classes to override Object methods
    return (
          struct _String){Object_super(childT, &String_Object_vtbl), &String_vtbl, "", 0};
}

String String_new2(char *value) {
    String new = malloc(sizeof(*new));
    new->super = Object_super(&StringT, &String_Object_vtbl);
    new->val = value;
    new->len = strlen(value);
    return new;
}

struct _String String_super2(Type *childT, char *value) {
    // TODO: Allow child classes to override Object methods
    return (struct _String){Object_super(childT, &String_Object_vtbl), &String_vtbl,
          value, strlen(value)};
}

char *String_to_bytes(void *string) {
    String this = (String) string;
    return this->val;
}

Integer String_hash(void *string) {
    // TODO
    return Integer_new(0);
}

bool String_equals(void *string, Object other) {
    String this = (String) string;
    if (!Object_is_type(other, &StringT)) {
        // TODO: Exceptions
        return false;
    }
    String otherString = (String) other;
    if (this->len != otherString->len) {
        return false;
    }
    return strcmp(this->val, otherString->val) == 0;
}

String String_to_string(void *string) {
    puts("In string to string");
    return (String) string;
}

size_t String_len(void *string) {
    String this = (String) string;
    return this->len;
}

/******************************************************************************/

void print(Object this) {
    puts("Here");
    printf("%s", String_to_bytes(this->vptr->to_string(this)));
}