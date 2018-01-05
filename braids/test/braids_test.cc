// Copyright 2012 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "braids/renaissance.h"
#include "braids/vocalist/vocalist.h"
#include "braids/quantizer.h"
#include "stmlib/test/wav_writer.h"
#include "stmlib/utils/dsp.h"

using namespace braids;
using namespace stmlib;

const uint32_t kSampleRate = 96000;
const uint16_t kAudioBlockSize = 24;

void TestRenaissance() {
  Renaissance osc;

  WavWriter wav_writer(1, kSampleRate, 64*4);
  wav_writer.Open("oscillator.wav");

  osc.init(96000, kAudioBlockSize);

  for (int shape = 0; shape < 30; shape++) {
    printf("Shape %d\n", shape);
    osc.set_shape(shape);
    int n = 20;
    for (uint32_t i = 0; i < kSampleRate * 4 / kAudioBlockSize; ++i) {
      if ((i % 3000) == 1900) {
        osc.set_gatestate(false);
      }
      if ((i % 3000) == 0) {
        osc.set_pitch((n << 7));
        n+=12;
        osc.Strike();
        osc.set_gatestate(true);
      }
      osc.set_pitch((n << 7) + rand() % 20);
      int16_t buffer[kAudioBlockSize];
      uint8_t sync_buffer[kAudioBlockSize];
      uint16_t tri = i/2 % 65535;
      uint16_t tri2 = (i/3) % 65535;
      uint16_t ramp = i * 150;
      tri = tri > 32767 ? 65535 - tri : tri;
      tri2 = tri2 > 32767 ? 65535 - tri2 : tri2;
      osc.set_parameters(tri, tri2);
      memset(sync_buffer, 0, sizeof(sync_buffer));
      //sync_buffer[0] = (i % 32) == 0 ? 1 : 0;
      osc.Render(sync_buffer, buffer, kAudioBlockSize);
      wav_writer.WriteFrames(buffer, kAudioBlockSize);
    }
  }
}

void TestVocalist() {
  Renaissance osc;

  WavWriter wav_writer(1, kSampleRate, 64*4);
  wav_writer.Open("oscillator.wav");

  osc.init(96000, kAudioBlockSize);
  osc.set_shape(0);

  for (int word = 0; word < 16; word++) {
    printf("word %d\n", word);
    
    osc.vocalist.SetWord(word);

    osc.Strike();
    osc.set_gatestate(true);

    for (uint32_t i = 0; i < 40000/kAudioBlockSize; ++i) {
      int16_t buffer[kAudioBlockSize];
      uint8_t sync_buffer[kAudioBlockSize];
     
      memset(sync_buffer, 0, sizeof(sync_buffer));
      //sync_buffer[0] = (i % 32) == 0 ? 1 : 0;
      osc.Render(sync_buffer, buffer, kAudioBlockSize);
      wav_writer.WriteFrames(buffer, kAudioBlockSize);
    }

    osc.set_gatestate(false);
  }
}

void TestQuantizer() {
  Quantizer q;
  q.Init();
  for (int16_t i = 0; i < 16384; ++i) {
    int32_t pitch = i;
    printf("%d quantized to %d\n", i, q.Process(i, 60 << 7));
  }
}

int main(void) {
  TestVocalist();
}
