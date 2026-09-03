// instruction and operators :
//  instruction : The instruction is a command that tells the computer to perform a specific operation. It consists of an opcode, which identifies the operation to be performed, and zero or more operands, which specify the data on which the operation is to be performed.

// types of instruction :
// 1.Type declaration instructions: These instructions declare the type of a variable or a function. For example, int x; declares a variable x of type int.
// example :
// int x; // declares a variable x of type int.
// float y; // declares a variable y of type float.
// char c; // declares a variable c of type char.
// void f(); // declares a function f that takes no arguments and returns void.
// int g(int); // declares

// type declaration rules :
// |------------------------------|--------------------------|
// |        type                  |      description         |
// |------------------------------|--------------------------|
// | int                          | integer                  |
// | float                        | floating-point number    |
// | double                       | double-precision number  |
// | char                         | character                |
// | void                         | no value                 |
// |------------------------------|--------------------------|

// rules 1 : The type declaration instructions must be followed by a semicolon (;).
// example : int x; // correct
//           intx;   // incorrect,

// rules 2 : The type declaration instructions can be combined with an assignment to initialize the variable.
// example : int x = 10; // declares a variable x of type int and initializes it to 10.
//           float y = 3.14; // declares a variable y of type float and initializes it to 3.14.

// rules 3 : The type declaration instructions can be combined with a function definition to declare a function.
// example : void f() { // declares a function f that takes no arguments and returns void.
//              function body
//           }

// rules 4 : The type declaration instructions can be combined with a function declaration to declare a function.
// example : int g(int); // declares a function g that takes an int argument and returns an int.

// rule 5: declare variable before use it.
// example : int x = 10; // declares a variable x of type int and initializes it to 10.
//           printf("%d", x); // prints the value of x.

// rule 6 : we can declare same type of variable in one line.
//  example : int x, y = 10, z; // declares three variables x, y, and z of type int and initializes y to 10.

// rule 7 : we can declare multiple variables of different types in one line.
// example : int x = 10, y; // declares a variable x of type int and initializes it to 10, and declares a variable y of type int.

// rule 8 : we can declare same value for  all declared variables.
// example : int x = 10, y = 10, z = 10; // declares three variables x, y, and z of type int and initializes them to 10.
// example : int x = 10

#include <stdio.h>
int main()
{
    int a = 22;
    int b = 33;
    int oldAge = 22;
    int newAge = 33;
}

// 2. Arithmetic instructions: These instructions perform arithmetic operations such as addition, subtraction, multiplication, and division on values or variables.
// operand : operand is a value or variable on which an operation is performed. For example, in the expression x + y, x and y are operands, and + is the operator.

//        a     +     b
//        |     |     |
//      operand  |   operand
//        |     |     |
//      value   |   value
//           operator

#include <stdio.h>
int main()
{
    int a, b, sum;
    a = 10;
    b = 220;
    sum = a + b;
    printf(" the sum of the a and b is %d ", sum);
}

// in arithmetic operator we have the operator types :
// 1. Addition (+): Adds two values or variables together.
// 2. Subtraction (-): Subtracts one value or variable from another.
// 3. Multiplication (*): Multiplies two values or variables together.
// 4. Division (/): Divides one value or variable by another.
// 5. Modulus (%): Returns the remainder of dividing one value or variable by another. the modulator operator work only with integer values.
// 6. Increment (++): Increases the value of a variable by one.
// 7. Decrement (--): Decreases the value of a variable by one.
// 8. Assignment (=): Assigns a value to a variable.
// 9. Compound assignment (+=, -=, *=, /=, %=): Performs an arithmetic operation and assigns the result to a variable.

//  |--------------------------|----------------------|
//  |        valid             |      invalid         |
//  |--------------------------|----------------------|
//  | a = b + c;               |  b + c = a           |
//  | a = b * c;               |  a = bc              |
//  | a = b / c;               |  a = b ↑ c           |
//  |--------------------------|----------------------|

//  |--------------------------|----------------------|
//  |    expression            |      value           |
//  |--------------------------|----------------------|
//  |         +                |  addition            |
//  |         -                |  subtraction         |
//  |         *                |  multiplication       |
//  |         /                |  division            |
//  |         %                |  modulus             |
//  |         ++               |  increment           |
//  |         --               |  decrement           |
//  |         =                |  assignment          |
//  |         +=               |  compound assignment |
//  |         -=               |  compound assignment |
//  |         *=               |  compound assignment |
//  |         /=               |  compound assignment |
//  |         %=               |  compound assignment |
//  |--------------------------|----------------------|

#include <stdio.h>
int main()
{
    int a = 10;
    int b = 20;
    int sum = a + b;
    int sub = a - b;
    int mul = a * b;
    int div = a / b;
    int mod = a % b;
    int inc = a++;
    int dec = b--;
    int equal = (a == 30);
    int compound = a += 10;
    printf(" the sum of the a and b is %d \n", sum);
    printf(" the sub of the a and b is %d \n", sub);
    printf(" the mul of the a and b is %d \n", mul);
    printf(" the div of the a and b is %d \n", div);
    printf(" the mod of the a and b is %d \n", mod);
    printf(" the inc of the a and b is %d \n", inc);
    printf(" the dec of the a and b is %d \n", dec);
    printf(" the equal of the a and b is %d \n", equal);
    printf(" the compound of the a and b is %d \n", compound);
}

// type conversion : Type conversion is the process of converting a value from one data type to another. It is also known as type casting. In C, type conversion can be done implicitly or explicitly.

// int op int  : the result is int
// int op float: the result is float
// float op float : the result is float
// float op int : the result is float
// char op char : the result is char
// char op int : the result is int
// int op char : the result is int
// float op char : the result is float
// char op float : the result is float

// 1. Implicit type conversion: Implicit type conversion is done automatically by the compiler. It occurs when values of different data types are combined in an expression. The compiler converts the values to a common data type before performing the operation.
// example : int a = 10;
//           float b = 3.14;
//           float c = a + b; // implicit conversion of a to float

// 2. Explicit type conversion: Explicit type conversion is done by the programmer using type casting. It involves specifying the desired data type in parentheses before the value to be converted.
// example : int a = 10;
//           float b = 3.14;
//           int c = (int)b; // explicit conversion of b to int

// operator precedence : Operator precedence determines the order in which operators are evaluated in an expression. Operators with higher precedence are evaluated before operators with lower precedence. If two operators have the same precedence, the associativity of the operators determines the order of evaluation.
//            precedence  operator
//            1           ()
//            2           ++, --, +, -, !
//            3           *, /, %
//            4           +, -
//            5           <, <=, >, >=
//            6           ==, !=
//            7           &&
//            8           ||
//            9           =, +=, -=, *=, /=, %=
//            10          ,

// example
#include <stdio.h>
int main()
{
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;
    int e = 50;
    int f = 60;
    int g = 70;
    int h = 80;
    int i = 90;
    int j = 100;
    int k = 110;
    int l = 120;
    int m = 130;
    int n = 140;
    int o = 150;
    int p = 160;
    int q = 170;
    int r = 180;
    int s = 190;
    int t = 200;
    int u = 210;
    int v = 220;
    int w = 230;
    int x = 240;
    int y = 250;
    int z = 260;
    int sum = a + b * c / d % e + f - g * h / i % j + k - l * m / n % o + p - q * r / s % t + u - v * w / x % y + z;
    printf(" the sum of the a and b is %d \n", sum);
}
// q - solve following equation :
// 1. 5 * 2 - 2 * 3
// 2. 5 * 2 / 2 * 3
// 3. 5 * (2 / 2) * 3
// 4. 5 + 2 / 2 * 3

#include <stdio.h>
int main()
{
    int a = 5, b = 2, c = 3, result1, result2, result3, result4;
    result1 = a * b - b * c;
    printf(" the result is %d \n", result1);
    result2 = a * b / b * c;
    printf(" the result is %d \n", result2);
    result3 = a * (b / b) * c;
    printf(" the result is %d \n", result3);
    result4 = a + b / b * c;
    printf(" the result is %d \n", result4);
}

// 3. Control instructions: These instructions control the flow of execution in a program. They include conditional statements (if, else, switch), loops (for, while, do-while), and function calls.

// types of the control instructions :
// a. sequence control instruction : These instructions execute a series of statements in the order in which they appear in the program.
// example
#include <stdio.h>
int main()
{
    int a = 10;
    int b = 20;
    int sum = a + b;
    printf(" the sum of the a and b is %d ", sum);
}
// in the above example the sequence of the instruction is the first declare the variable a and b and then add the a and b and then print the sum of the a and b.

// b. decision control instruction : These instructions make decisions based on the value of a condition. They include if, else, and switch statements.
// c. loop control instruction : These instructions repeat a series of statements until a condition is met. They include for, while, and do-while loops.
// d. case control instruction : These instructions select one of several possible actions based on the value of a variable. They include switch statements.

// 4. Data movement instructions: These instructions move data between memory locations or registers. They include load and store instructions, which move data between memory and registers, and move instructions, which move data between registers.
// 5. Input/output instructions: These instructions read input from a device (such as a keyboard or mouse) or write output to a device (such as a display or printer).
// 6. Transfer instructions: These instructions transfer control from one part of a program to another. They include jump instructions, which transfer control to a specific location in the program, and call and return instructions, which transfer control to and from a function.
// 7. Logical instructions: These instructions perform logical operations such as AND, OR, and NOT on values or variables.

//  operators : Operators are symbols that represent an operation that can be performed on one or more operands. They are used to perform arithmetic, logical, and other operations on values or variables.

// types of operators :
// 1. Arithmetic operators: Arithmetic operators perform arithmetic operations such as addition, subtraction, multiplication, and division on values or variables.
// example : int a = 10;
//           int b = 20;
//           int sum = a + b; // addition
//           int sub = a - b; // subtraction
//           int mul = a * b; // multiplication
//           int div = a / b; // division
//           int mod = a % b; // modulus
//           int inc = a++; // increment
//           int dec = b--; // decrement

// 2. Relational operators: Relational operators compare two values or variables and return a Boolean value (true or false) based on the comparison.
// example :
//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          ==            |  equal to                      |
//  |          !=            |  not equal to                  |
//  |          <             |  less than                     |
//  |          >             |  greater than                  |
//  |          <=            |  less than or equal to         |
//  |          >=            |  greater than or equal to      |
//  |------------------------|--------------------------------|

//  q - write a program to compare two numbers.
#include <stdio.h>
int main()
{
    int a = 20, b = 30;
    if (a == b)
    {
        printf(" the %d and %d both are equal ", a, b);
    }
    else if (a != b)
    {
        printf(" the %d and %d both are not equal", a, b);
    }
    else if (a < b)
    {
        printf(" the %d is less than %d ", a, b);
    }
    else if (a > b)
    {
        printf(" the %d is greater than %d ", a, b);
    }
    else if (a <= b)
    {
        printf(" the %d is less than or equal to %d ", a, b);
    }
    else if (a >= b)
    {
        printf(" the %d is greater than or equal to %d ", a, b);
    }
    return 0;
}

// 3. Logical operators: Logical operators perform logical operations such as AND, OR, and NOT on Boolean values.

//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          &&            |  logical AND                   |
//  |          ||            |  logical OR                    |
//  |          !             |  logical NOT                   |
//  |------------------------|--------------------------------|
// logic table 
// and :
// |-----------|-------------|------------------|
// |    a      |     b       |      a && b      |
// |-----------|-------------|------------------|
// |    true   |    true     |      true        |
// |    true   |    false    |      false       |
// |    false  |    true     |      false       |
// |    false  |    false    |      false       |
// |-----------|-------------|------------------|

// or :
// |-----------|-------------|------------------|
// |    a      |     b       |      a || b      |
// |-----------|-------------|------------------|
// |    true   |    true     |      true        |
// |    true   |    false    |      true        |
// |    false  |    true     |      true        |
// |    false  |    false    |      false       |
// |-----------|-------------|------------------|

// not :
// |-----------|-------------|
// |    a      |     !a      |
// |-----------|-------------|
// |    true   |    false    |
// |    false  |    true     |
// |-----------|-------------|




#include <stdio.h>
int main()
{
    int a = 10, b = 20;
    if (a == 10 && b == 20)
    {
        printf("the a and b both are equal");
    }
    else if (a == 10 || b != 30)
    {

        printf(" the a is equal to 10 and b is not equal to 30");
    }
    else if (a != 20 || b == 20)
    {
        printf(" the a is not equal to 20 and b is equal to 20");
    }
    else if (a != 20 && b != 30)
    {
        printf(" the a is not equal to 20 and b is not equal to 30");
    }
    else if (a != b)
    {
        printf(" the a is not equal to b ");
    }
}

// write a program to check the grade of the student
#include <stdio.h>
int main()
{

    int marks, grade;
    printf(" enter the marks of the student : ");
    scanf("%d", &marks);
    if (marks >= 90 && marks <= 100)
    {
        printf(" you have a+ grade ");
    }
    else if (marks > 80 && marks <=90)
    {
        printf(" you have a grade ");
    }
    else if (marks > 70 && marks <=80)
    {
        printf(" you have b+ grade ");
    }
    else if (marks > 60 && marks <=70)
    {
        printf(" you have b grade ");
    }
    else if (marks > 50 && marks <=60)
    {
        printf(" you have c grade ");
    }
    else 
    {
        printf(" you have failed in the exam ");
    }
}

// 4. Assignment operators: Assignment operators assign a value to a variable.

//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          =             |  assignment                    |
//  |          +=            |  addition assignment           |
//  |          -=            |  subtraction assignment        |
//  |          *=            |  multiplication assignment     |
//  |          /=            |  division assignment           |
//  |          %=            |  modulus assignment            |
//  |------------------------|--------------------------------|

// 5. Increment and decrement operators: Increment and decrement operators increase or decrease the value of a variable by one.
// |------------------------|--------------------------------|
// |        operator        |      description               |
// |------------------------|--------------------------------|
// |          ++            |  increment                     |
// |          --            |  decrement                     |
// |------------------------|--------------------------------|

// 6. Conditional operator: The conditional operator (?:) is a ternary operator that evaluates a condition and returns one of two values based on the result of the evaluation.

//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          ? :           |  conditional                   |
//  |------------------------|--------------------------------|

// 7. Bitwise operators: Bitwise operators perform bitwise operations on values or variables.
//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          &             |  bitwise AND                   |
//  |          |             |  bitwise OR                    |
//  |          ^             |  bitwise XOR                   |
//  |          ~             |  bitwise NOT                   |
//  |          <<            |  left shift                    |
//  |          >>            |  right shift                   |
//  |------------------------|--------------------------------|

// 8. Shift operators: Shift operators shift the bits of a value to the left or right by a specified number of positions.
//  |------------------------|--------------------------------|
//  |        operator        |      description               |
//  |------------------------|--------------------------------|
//  |          <<            |  left shift                    |
//  |          >>            |  right shift                   |
//  |------------------------|--------------------------------|
