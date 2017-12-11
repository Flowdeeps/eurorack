// dumptables.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DUMPTABLES true

#include "exp2.h"
#include "sin.h"

void dumptable(int32_t *table, int c) {
	for (int i = 0; i < c; i++) {
    	printf("%d", exp2tab[i]);
    	if (i < c-1) {
    		printf(",");
    	}
    	if (i % 10 == 9) {
    		printf("\n");
    	}
    }
}

int main(int argc, char **argv)
{
    Exp2::init();
    Tanh::init();
    Sin::init();

    printf("const int32_t exp2tab[] = {\n");
    dumptable(&exp2tab[0],EXP2_N_SAMPLES << 1);
    printf("};\n");

    printf("const int32_t tanhtab[] = {\n");
    dumptable(&tanhtab[0],TANH_N_SAMPLES << 1);
    printf("};\n");

    printf("const int32_t sintab[] = {\n");
    dumptable(&sintab[0], SIN_N_SAMPLES << 1);
    printf("};\n");
}