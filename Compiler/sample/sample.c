#include <stdio.h>
int function2();
void canyonMain();
int main(int argc, char **argv) {
    canyonMain();
    return 0;
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
    printf("%d\n",2);
    x = 3;
    printf("%d\n",x);
    x = 4;
    printf("%d\n",x);
    y = 5;
    printf("%d\n",y);
    z = 6;
    printf("%d\n",z);
    a = 1;
    printf("%d\n",a);
    printf("%d\n",x);
    printf("%d\n",y);
    printf("%d\n",z);
    x = y = z = 10;
    printf("%d\n",x);
    printf("%d\n",y);
    printf("%d\n",z);
    printf("%d\n",(1+1));
    add = (x+y);
    printf("%d\n",add);
    printf("%d\n",(2-1));
    sub = (x-5);
    printf("%d\n",sub);
    printf("%d\n",((2-1)+5));
    printf("%d\n",(2*2));
    mult = (x*y);
    printf("%d\n",mult);
    printf("%d\n",(4/2));
    div = (x/2);
    printf("%d\n",div);
    printf("%d\n",(15 % 4));
    mod = (x % y);
    printf("%d\n",mod);
    printf("%d\n",((((2*3)/4) % 5)*2));
    printf("%d\n",((2*3)+(6*7)));
    printf("%d\n",((1+2)*3));
    sep = ((1+5)/3);
    printf("%d\n",sep);
    printf("%d\n",function2());
    return;
}
