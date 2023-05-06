# CanyonLang

CanyonLang is a strongly, statically typed; object oriented language.

A Canyon library is a collection of independent packages, such that any one package may be imported without needing dependencies from the rest of the library.

A Canyon package is a collection of modules.

A Canyon module is a single source code file. It should contain a primary class with the same name as the file. It may additionally contain child classes, nested classes, and enumerated types.

Let's consider the example of the Canyon Standard Library. This library contains the Language Package. This package contains the String Module. You may just `import canyon.lang.String` to import only the String class. You may also `import canyon.lang.*` to import the entire Language Package. Furthermore, you may `import canyon.*` to import the entire Standard Library.

## Syntax

### Keywords

- abstract
  - Modifier to a `class` marking it as abstract. The class may not contain a constructor, and a default constructor will not be created.
- assert <!-- TODO Maybe -->
- bool
- break
- byte
  - An 8 bit integer
- case
- catch
- char
  - Shorthand for `unsigned byte`
- class
- continue
- default
- do
- double
- else
- enum
- examines
  - Specifies a child class that treats its parent's private fields as <!-- TODO package protected? --> instead. A class may only examine one other class (single inheritance). Examining classes may be extended, with the conversion from private to PACKAGE PROTECTED????? maintained with all further subclasses. You may only examine classes within the same package.
- extends
  - Specifies a child class. A class may only extend one other class. Interfaces may only extend interfaces (can be multiple, comma-separated interfaces); abstract classes may only extend an abstract class; concrete classes may extend either an abstract or a concrete class. Unless specified to extend another class, classes contain an implicit `extends Object`
- final
  - Same as Java
- finally
- float
- for
- if
- implements
  - Specifies that a class implements the given interface. Can be used for either concrete or abstract classes. Classes may implement multiple comma-separated interfaces.
- import
- in
  - Used alongside for to create a foreach loop
  - For example, `for (Object item in list)`
- instanceof <!-- TODO or equivalent -->
- int
  - A 32 bit integer
- interface
  - Defines a set of method signatures that an implementing class must provide an implementation of
  - Makes a member package-visible (essentially the inverse of C++ friend)
- long
  - A 64 bit integer
- native
  - Externs a C function
- new
- package
- private
- protected
- public
- return
- sealed
- short
  - A 16 bit integer
- static
- super
- switch
- this
- throw
- throws
- transient
- try
- unsigned
  - Specifies that a primitive type is unsigned. Primitives, with the exception of `char`, are signed by default.
- void
- volatile
- while

## Classes

(note that the inner and outer classes implicitly examine each other but do not directly inherit from each other)

### Public classes (may be a nested class or a standalone class)

Public classes are defined as `public class Foo`. Public classes may be referenced from any package.

| Field Modifier | This Class | Child Classes, same pkg | Other Classes, same pkg | Child Classes, diff pkg | Other Classes, diff pkg |
|:--------------:|:----------:|:-----------------------:|:-----------------------:|:-----------------------:|:-----------------------:|
|    `public`    |     ✅      |            ✅            |            ✅            |            ✅            |            ✅            |
|  `protected`   |     ✅      |            ✅            |            ✅            |            ✅            |            ❌            |
|  `package(d)`  |     ✅      |            ✅            |            ✅            |            ❌            |            ❌            |
|   `private`    |     ✅      |            ❌            |            ❌            |            ❌            |            ❌            |

### Package classes (may be a nested class or a standalone class)

`package class`es may not be referenced outside the current package

| Field Modifiers | Sealed | This Class | Examining Classes | Child Classes | Other Classes |
|:---------------:|:------:|:----------:|:-----------------:|:-------------:|:-------------:|
|    `public`     |   No   |     ✅      |         ✅         |       ✅       |       ✅       |
|    `public`     |  Yes   |     ✅      |         ✅         |       ✅       |       ✅       |
|    `package`    |   No   |     ✅      |         ✅         |       ✅       |       ✅       |
|    `package`    |  Yes   |     ✅      |         ✅         |       ✅       |       ✅       |
|   `protected`   |   No   |     ✅      |         ✅         |       ✅       |       ✅       |
|   `protected`   |  Yes   |     ✅      |         ✅         |       ✅       |       ❌       |
|    `private`    |   No   |     ✅      |         ✅         |       ❌       |       ❌       |
|    `private`    |  Yes   |     ✅      |         ❌         |       ❌       |       ❌       |

### Protected classes (may *ONLY* be a nested class)

### Private classes (may *ONLY* be a nested class)

### Attributes

### Methods

#### Constructors
