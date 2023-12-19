#include <stdio.h>
void print2(int one,int two);
int function4(int x,int y);
int function3(int x);
int function2();
void canyonMain();
void print(int x);
int main(int argc, char **argv) {
    canyonMain();
    return 0;
}
void print2(int one,int two){
    print(one);
    print(two);
}
int function4(int x,int y){
    return (x+y);
}
int function3(int x){
    return x;
}
int function2(){
    return 100;
}
void canyonMain(){
    int sep;
    int div;
    int mult;
    int sub;
    int mod;
    int a;
    int z;
    int add;
    int y;
    int x;
    print(2);
    x=3;
    print(x);
    x=4;
    print(x);
    y=5;
    print(y);
    z=6;
    print(z);
    a=1;
    print(a);
    print(x);
    print(y);
    print(z);
    x=y=z=10;
    print(x);
    print(y);
    print(z);
    print((1+1));
    add=(x+y);
    print(add);
    print((2-1));
    sub=(x-5);
    print(sub);
    print(((2-1)+5));
    print((2*2));
    mult=(x*y);
    print(mult);
    print((4/2));
    div=(x/2);
    print(div);
    print((15%4));
    mod=(x%y);
    print(mod);
    print(((((2*3)/4)%5)*2));
    print(((2*3)+(6*7)));
    print(((1+2)*3));
    sep=((1+5)/3);
    print(sep);
    print(function2());
    print(function3(x));
    print(function4(1,3));
    print2((1+1),(2+2));
    return;
}
void print(int x) {
    printf("%d\n", x);
}
