#ifndef __VOCALIST_H__
#define __VOCALIST_H__

#include "wordlist.h"

#define NUM_VOCALIST_PATCHES NUM_BANKS

#include "sam/sam.h"

#define MODE_NORMAL 0
#define MODE_CRAZY 1
#define MODE_REALLY_CRAZY 2

typedef unsigned short uint16_t;

class Vocalist {
public:
  Vocalist() {

  }

  ~Vocalist() { }

  void init(int sampleRate, int samplesPerBlock);
  void set_shape(int shape);

  void Render(const uint8_t* sync_buffer, int16_t *output, int len);
  void set_gatestate(bool gs);
  void Strike() { }
  // void SetBank(unsigned char b);
  void SetWord(unsigned char b);

  void SetMode(int m) { mode = m; }
  void set_pitch(uint16_t braids_pitch) { 
    sam.SetPitch(82 + (64 - (braids_pitch >> 9)));
  }
  void set_parameters(uint16_t parameter1, uint16_t parameter2) {
    SetSpeed(72 + (parameter2 >> 8) - 64);
    SetWord(parameter1 >> 11);
  }

  void SetPitch(unsigned char pitch) { sam.SetPitch(pitch); }
  void SetMouth(unsigned char mouth) { sam.SetMouth(mouth); }
  void SetThroat(unsigned char throat) { sam.SetThroat(throat); }
  void SetSpeed(unsigned char speed) { sam.SetSpeed(speed); }

private:
  void Load();
  void LoadRando();

  SAM sam;
  int mode;
  unsigned char bank;
  unsigned char word;
  bool gatestate;
  bool risingEdge;
  bool playing;
  unsigned char rando[24];
};

#endif
