// dumptables.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DUMPTABLES true

#include "exp2.cc"
#include "sin.cc"
#include "freqlut.cc"

// ripped out of MkEngine
static const uint16_t SINLOG_BITDEPTH = 10;
static const uint16_t SINLOG_TABLESIZE = 1<<SINLOG_BITDEPTH;
static uint16_t sinLogTable[SINLOG_TABLESIZE];

static const uint16_t SINEXP_BITDEPTH = 10;
static const uint16_t SINEXP_TABLESIZE = 1<<SINEXP_BITDEPTH;
static uint16_t sinExpTable[SINEXP_TABLESIZE];

void calctables() {
    float bitReso = SINLOG_TABLESIZE;

    for(int i=0;i<SINLOG_TABLESIZE;i++) {
        float x1 = sin(((0.5+i)/bitReso) * M_PI/2.0);
        sinLogTable[i] = round(-1024 * log2(x1));
    }
    
    bitReso = SINEXP_TABLESIZE;
    for(int i=0;i<SINEXP_TABLESIZE;i++) {
        float x1 = (pow(2, float(i)/bitReso)-1) * 4096;
        sinExpTable[i] = round(x1);
    }
}

void dumptable32(int32_t *table, int c) {
	for (int i = 0; i < c; i++) {
    	printf("%d", table[i]);
    	if (i < c-1) {
    		printf(",");
    	}
    	if (i % 10 == 9) {
    		printf("\n");
    	}
    }
}

void dumptableu16(uint16_t *table, int c) {
    for (int i = 0; i < c; i++) {
        printf("%hu", table[i]);
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
    calctables();

    int32_t sampleRate = 48000;
    Freqlut::init(sampleRate);

    printf("#include <math.h>\n#include \"synth.h\"\n");

    printf("const int32_t exp2tab[] = {\n");
    dumptable32(&exp2tab[0],EXP2_N_SAMPLES << 1);
    printf("};\n");

    printf("const int32_t tanhtab[] = {\n");
    dumptable32(&tanhtab[0],TANH_N_SAMPLES << 1);
    printf("};\n");

    printf("const int32_t sintab[] = {\n");
    dumptable32(&sintab[0], SIN_N_SAMPLES << 1);
    printf("};\n");

    printf("const int32_t lut[] = {\n");
    dumptable32(&lut[0], N_SAMPLES + 1);
    printf("};\n");

    printf("const uint16_t sinLogTable[] = {\n");
    dumptableu16(&sinLogTable[0], SINLOG_TABLESIZE);
    printf("};\n");

    printf("const uint16_t sinExpTable[] = {\n");
    dumptableu16(&sinExpTable[0], SINEXP_TABLESIZE);
    printf("};\n");

}