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
    playing = false;
    bank = 0;
    word = 0;
    risingEdge = 0;
    mode = MODE_NORMAL;
    SetPitch(80);
    SetSpeed(72);
    SetThroat(128);
    SetMouth(128);
    sam.InitFrameProcessor();
    sam.EnableSingmode();
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
  void set_pitch(uint16_t braids_pitch) { sam.SetPitch(braids_pitch >> 8); }
  void set_parameters(uint16_t parameter1, uint16_t parameter2) {
    SetSpeed(parameter1 >> 7);
    SetThroat(parameter2 >> 7);
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
