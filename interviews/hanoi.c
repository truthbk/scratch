#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

void hanoi(uint32_t n, char src, char dst, char aux){
    if(n==1) {
        fprintf(stdout, "move disc from %c to %c\n", src, dst);
        return;
    }

    hanoi(n-1, src, aux, dst);
    fprintf(stdout, "move disc from %c to %c\n", src, dst);
    hanoi(n-1, aux, dst, src);
    return;
}


int main(int argc, char ** argv) {
    uint32_t n =3;
    hanoi(n, 'A', 'C', 'B');
    return 0;
}
