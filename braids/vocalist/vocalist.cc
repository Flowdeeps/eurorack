#include <stdio.h>
#include <stdlib.h>

#include "vocalist.h"
#include "wordlist.h"

void Vocalist::SetBank(unsigned char b) {
  bank = b;
}

void Vocalist::SetWord(unsigned char w) {
  word = w;
}

void Vocalist::Load() {
  playing = false;
  if (mode == MODE_NORMAL) {
    sam.LoadNextWord(
      &data[wordpos[bank][word]],
      &data[wordpos[bank][word] + wordlen[bank][word]],
      &data[wordpos[bank][word] + (wordlen[bank][word] << 1)],
      wordlen[bank][word]
    );
    sam.InitFrameProcessor();
    sam.PrepareFrames();
  } else {
    LoadRando();
  }
}

void Vocalist::LoadRando() {
  for (int i = 0; i < 24; i++) {
    rando[i] = rand() % 256;
  }
  sam.LoadNextWord(&rando[0], &rando[8], &rando[16], 8);
  sam.InitFrameProcessor();
  sam.PrepareFrames();
}

void Vocalist::FillBuffer(int16_t *output, int bufferLen) {
  unsigned char buffer[6];
  int len = bufferLen >> 2;

  if (risingEdge) {
    Load();
    risingEdge = false;
    playing = true;
  }

  int written = 0;
  if (playing) {
    written = sam.FillBufferFromFrame(len, &buffer[0]);

    if (written < len) {
      if (mode == MODE_NORMAL) {
        playing = false;
        SetWord((word + 1) % NUM_WORDS);
      } else {
        LoadRando();
      }
    }
  }

  for (int i = written; i < len; i++) {
    buffer[i] = 0x80;
  }

  for (int i = 0; i < 6; i+=1) {
    int idx = i << 2;
    int16_t value = (((int16_t) buffer[i])-127) << 8;

    output[idx] = value;
    output[idx+1] = value;
    output[idx+2] = value;
    output[idx+3] = value; 
  }
}

void Vocalist::set_gatestate(bool gs) {
  if (!gatestate && gs) {
    risingEdge = true;
  }
  gatestate = gs;
}


