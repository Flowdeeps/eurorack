/**
 *
 * Copyright (c) 2014-2015 Pascal Gauthier.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <fstream>
using namespace ::std;

/**
 * This function normalize data that comes from corrupted sysex.
 * It used to avoid engine crashing upon extrem values
 */
char normparm(char value, char max, int id) {
    if ( value <= max && value >= 0 )
        return value;
    
    // if this is beyond the max, we expect a 0-255 range, normalize this
    // to the expected return value; and this value as a random data.
    
    value = abs(value);
    
    char v = ((float)value)/255 * max;

    return v;
}

char sysexChecksum(const char *sysex, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; sum -= sysex[i++]);
    return sum & 0x7F;
}

void unpackProgram(char* voiceData, unsigned char *unpackPgm, int idx) {
    char *bulk = (char *)voiceData + 6 + (idx * 128);
    
    for (int op = 0; op < 6; op++) {
        // eg rate and level, brk pt, depth, scaling
        
        for(int i=0; i<11; i++) {
            unpackPgm[op * 21 + i] = normparm(bulk[op * 17 + i], 99, i);
        }
        
        memcpy(unpackPgm + op * 21, bulk + op * 17, 11);
        char leftrightcurves = bulk[op * 17 + 11];
        unpackPgm[op * 21 + 11] = leftrightcurves & 3;
        unpackPgm[op * 21 + 12] = (leftrightcurves >> 2) & 3;
        char detune_rs = bulk[op * 17 + 12];
        unpackPgm[op * 21 + 13] = detune_rs & 7;
        char kvs_ams = bulk[op * 17 + 13];
        unpackPgm[op * 21 + 14] = kvs_ams & 3;
        unpackPgm[op * 21 + 15] = kvs_ams >> 2;
        unpackPgm[op * 21 + 16] = bulk[op * 17 + 14];  // output level
        char fcoarse_mode = bulk[op * 17 + 15];
        unpackPgm[op * 21 + 17] = fcoarse_mode & 1;
        unpackPgm[op * 21 + 18] = fcoarse_mode >> 1;
        unpackPgm[op * 21 + 19] = bulk[op * 17 + 16];  // fine freq
        unpackPgm[op * 21 + 20] = detune_rs >> 3;
    }
    
    for (int i=0; i<8; i++)  {
        unpackPgm[126+i] = normparm(bulk[102+i], 99, 126+i);
    }
    unpackPgm[134] = normparm(bulk[110], 31, 134);
    
    char oks_fb = bulk[111];
    unpackPgm[135] = oks_fb & 7;
    unpackPgm[136] = oks_fb >> 3;
    memcpy(unpackPgm + 137, bulk + 112, 4);  // lfo
    char lpms_lfw_lks = bulk[116];
    unpackPgm[141] = lpms_lfw_lks & 1;
    unpackPgm[142] = (lpms_lfw_lks >> 1) & 7;
    unpackPgm[143] = lpms_lfw_lks >> 4;
    memcpy(unpackPgm + 144, bulk + 117, 11);  // transpose, name
    unpackPgm[155] = 1;  // operator on/off
    unpackPgm[156] = 1;
    unpackPgm[157] = 1;
    unpackPgm[158] = 1;
    unpackPgm[159] = 1;
    unpackPgm[160] = 1;
}

// void resetToInitVoice() {
//     const char init_voice[] =
//       { 99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
//         99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
//         99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
//         99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
//         99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
//         99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 99, 0, 1, 0, 7,
//         99, 99, 99, 99, 50, 50, 50, 50, 0, 0, 1, 35, 0, 0, 0, 1, 0, 3, 24,
//         73, 78, 73, 84, 32, 86, 79, 73, 67, 69 };
    
//     for(int i=0;i<sizeof(init_voice);i++) {
//         data[i] = init_voice[i];
//     }

// }

int main(int argc, char **argv) {
    char *buf = 0;
    unsigned char pgm[161];
    long size;
    if (argc != 2) {
        error(-1, 0, "Usage: %s <patches.syx>\n\n", argv[0]);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        error(-1, 0, "Couldn't open file");
    }

    if (fseek(fp, 0, SEEK_END) == 0) {
        size = ftell(fp);
        buf = (char *) malloc(size);
        fseek(fp, 0L, SEEK_SET);

        size_t newLen = fread(buf, sizeof(char), size, fp);
    }
    int i;
    for (i = 0; 6 + (i * 128) < size; i++) {
        unpackProgram(buf, &pgm[0], i);
        printf("const unsigned char pgm%d[] = {\n", i+32);
        for (int j = 0; j < 161; j++) {
            printf("%d", pgm[j]);
            if (j != 160) {
                printf(", ");
            }
            if (j % 21 == 20) {
                printf("\n");
            }
        }
        printf("};\n\n");
    }

    printf("const unsigned char *voices[] = {");

    for (int j = 0; j < i; j++) {
        printf("pgm%d", j+32);
        if (j < i-1) {
            printf(", ");
        }
        if (j % 8 == 7) {
            printf("\n");
        }
    }

    fclose(fp);
}