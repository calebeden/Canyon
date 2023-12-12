#include <stdio.h>
void canyonMain();
int main(int argc, char **argv) {
    canyonMain();
    return 0;
}
void canyonMain() {
    int a;
    int z;
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
    return;
}
