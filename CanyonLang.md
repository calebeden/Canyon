# CanyonLang Specification v1.0.0

## Defining units of code

The following definitions are somewhat arbitrary, but are defined at this point so that they may be used in examples throughout the language spec. In reality, the compiler does not distinguish between packages and libraries, but they are useful concepts for the programmer.

A Canyon module is a single source code file. Its contents may define classes, functions, and enumerated types.

A Canyon package is a collection of modules designed to work together and are often heavily interdependent on each other.

A Canyon library is a collection of mostly independent packages bundled together for a common purpose.

For example, let's consider the Canyon Standard Library. This library contains all of the builtin code that comes with the compiler. It contains many packages, including the Language Package, which defines types that are critical to the language, and is the only package that is imported by default. Inside the Language Package is the String module, which defines the class `String`. If this package were not imported by the compiler by default, you could say `import canyon.lang.String;` to import only the definitions found in the String module, defined in the file `String.canyon`. If you wanted a specific definition, you could say `from canyon.lang.String import stringFunction;` to just import `stringFunction()`. Again assuming the package were not already imported by the compiler, you could write `import canyon.lang.*;` to import all modules from the Language Package. Furthermore, you may `import canyon.*` to import the entire Standard Library. When referencing something defined in another package, unless it is imported using a `from` statement, it must be referenced in the format `package.thing`, where 'thing' is defined in 'package.'

## Language Syntax

### Keywords

- abstract
  - Modifier to a `class` marking it as abstract. The class may not contain a constructor, and a default constructor will not be created. Abstract classes have at least one abstract method, and may or may not define concrete methods.
  - `abstract class Button {}`
- assert
  - The expression following this keyword is evaluated to a `bool` type. If the answer is false, throws an exception.
  - `assert (myVal == 5);`
- bool
  - Primitive data type representing a boolean value, that is, true or false.
  - `bool x = true;`
- break
  - Used to break out of a loop or switch statement, or in other words goto the end of the block.
  - `break;`
- byte
  - An 8 bit integer
  - `byte x = 5;`
- case
  - A value to match against inside a switch statement
  - `case 3: {}`
- catch
  - If an exception is thrown in the preceeding `try` block, will run the enclosed code instead
  - `catch (Exception e) {}`
- char
  - Shorthand for `unsigned byte`
  - `char x = 'a';`
- class
  - Defines an object class. Classes may be abstract or concrete depending on the absence or presence of the `abstract` keyword
  - class `BigButton {}`
- const
  - Defines a variable or attribute which cannot be reassigned after its initial assignment. Constant objects may have their memory modified, although they may not be reassigned to point to a different object.
  - `const bool true = 1;`
- continue
  - Used to goto the next loop iteration
  - `continue;`
- default
  - The case to goto in a switch statement if no other cases match
  - `default: {}`
- do
  - Execute the following block of code. Must be followed by a `while` and a condition to evaluate between loop iterations
  - `do {} while (x < 100);`
- double
  - A double precision float
  - `double x = 3.14159;`
- else
  - The code to run if the preceeding `if` condition is false. Can be chained together to form `else if`
  - `if (x % 3 == 0) {} else if (x % 3 == 1) {} else {}`
- enum
  - Defines an enumerated type
  - `enum day {SUN, MON, TUE, WED, THU, FRI, SAT}`
- extends
  - Specifies a child class. A class may only extend one other class. Interfaces may only extend interfaces (can be multiple, comma-separated interfaces); abstract classes may only extend an abstract class; concrete classes may extend either an abstract or a concrete class. Unless specified to extend another class, classes contain an implicit `extends Object`
  - `class BigButton extends Button {}`
- final
  - Final methods may not be overridden by a child class. Final classes may not be extended
  - `final void addOne() {}`
  - `final class Button {}`
- finally
  - Executes at the end of a try/catch block, regardless of which block the program ended up in
  - `try {} catch (Exception e) {} finally {}`
- float
  - A single precision floating point value
  - `float x = 3.14;`
- for
  - Defines a for or foreach loop
  - `for (int i = 0; i < 10; i++) {}`
- from
  - Imports just a specific definition, rather than the entire module
  - `from myLib.buttons.Button import Button;`
- if
  - Defines a code block to run if a certain condition is true
  - `if (x %2 == 0) {}`
- implements
  - Specifies that a class implements the given interface. Can be used for either concrete or abstract classes. Classes may implement multiple comma-separated interfaces.
  - `class DraggableButton implements Clickable, Dragable`
- import
  - Imports the specified library, package, or module. Each `.` character represents going a layer deeper into the file system heirarchy, like a `/` typically does in the terminal. The import is relative to the cwd where the compiler is executed.
  - `import myLib.buttons;`
- in
  - Used alongside for to create a foreach loop
  - `for (Object item in list) {}`
- instanceof <!-- TODO or equivalent -->
- int
  - A 32 bit integer
  - `int x = 5;`
- interface
  - Defines a set of method signatures that an implementing class must provide an implementation of
  - `interface Clickable {}`
- is
  - Evaluates whether two references point to the same instance of an object
  - `if (button1 is button2) {}`
- long
  - A 64 bit integer
  - `long x = 5L;`
- native
  - Externs a C function that the compiler will assume is linked once the Canyon compiler outputs the C translation of the CanyonLang
  - `native void foo(int x);`
- new
  - Allocates an object on the heap and calls its constructor
  - `new Button();`
- private
  - Defines an attribute, method, or inner class which cannot be accessed by another class or function, including child classes.
  - `private int x;`
- protected
  - Defines an attribute, method, or inner class which may be accessed by child classes, but not other classes or functions.
  - `protected int y;`
- public
  - Defines an attribute, method, or inner class which can be accessed anywhere.
  - `public int z;`
- return
  - Returns from the current function or method, with the following expression evaluated as the return value
  - `return 0;`
- short
  - A 16 bit integer
  - `short x = 7;`
- static
  - A modifier which states that something belongs only to the context in which it is defined. Static attributes and methods inside a class belong to the class, rather than any particular object. Static classes, functions, and variables may not be imported into another module.
  - `static int x = 4;`
- super
  - Refers to the parent class. Can be used to call the original version of an overloaded function or to reference a protected or public attribute that shares a name with one in the current class. The no argument super constructor is inserted automatically unless the child constructor specifically calls a different constructor from the parent class.
  - `super(a, b, c);`
- switch
  - Creates a switch statement
  - `switch (x) {}`
- this
  - Inside a method, refers to the object that the method is acting upon
  - `this.x;`
- throw
  - Used to cause an exception to occur
  - `throw new Exception();`
- throws
  - States that a function throws an error which the caller must catch
  - `void foo() throws Exception {}`
- transient
  - Designates that a particular attribute should not be saved when serializing and object
  - `transient int x;`
- try
  - Attempts to run the contents of the following code block. If an exception is thrown, jumps to the mandator catch block(s)
  - `try {} catch {}`
- unsigned
  - Specifies that a primitive type is unsigned. Primitives, with the exception of `char`, are signed by default.
  - `unsigned int x = 4;`
- void
  - Specifies that a function or method does not return a value
  - `void foo() {}`
- volatile
  - Signals to not optimize accesses to a variable because it may be accessed by different threads.
  - `volatile int x;`
- while
  - A loop to run as long as a condition is met
  - `while (x < 5) {}`

### Functions and methods

Functions and methods look nearly identical, with the key difference being that methods are defined inside a class and functions are not. They may each take parameters and return a single value or reference or void. Non-static methods will get a default `this` parameter with the static type defined to be the current class. Function and method signatures may be defined using the following rules:

1. Abstract
    - Only allowed in an abstract class. Defines an interface-like method which must be implemented by any concrete child classes.
1. Access Modifier
    - Defines the visibility of the method (`public`, `private`, `protected`). Disallowed for use with functions.
1. Static, native, final
    - `native` declares a compiled function written in a language such as C or C++. No body should be provided, since this should be resolved by the linker. Equivalent to C `extern` keyword. Note that `native` and `abstract` are mutually exclusive
    - `static` states that the method is a class method (not receiving a `this` object), and will be resolved using static typing at runtime. It is used on functions to limit their scope to within the current module, disallowing it to be imported or called in any other modules.
      - `static` functions may not be declared `native`, but it is possible for `static` methods to be `native`.
    - `final` declares a method that may not be redefined/overridden by a child class. It is invalid to use on a function.
1. Return type (including const, unsigned)
1. Function/method name
1. Parameter list
1. Exceptions
1. Body
