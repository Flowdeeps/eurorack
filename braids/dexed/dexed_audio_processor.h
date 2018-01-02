/**
 *
 * Copyright (c) 2013-2017 Pascal Gauthier.
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

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

// #include "../JuceLibraryCode/JuceHeader.h"

#include "controllers.h"
#include "dx7note.h"
#include "lfo.h"
#include "synth.h"
#include "fm_core.h"
// #include "PluginData.h"
// #include "PluginFx.h"
// #include "SysexComm.h"
#include "EngineMkI.h"

struct ProcessorVoice {
    int velocity;
    bool keydown;
    bool sustained;
    bool live;
    int16_t braids_pitch;
    Dx7Note dx7_note;
};

enum DexedEngineResolution {
    DEXED_ENGINE_MODERN,
    DEXED_ENGINE_MARKI
};

extern EngineMkI engineMkI;

//==============================================================================
/**
*/
class DexedAudioProcessor
{
    static const int MAX_ACTIVE_NOTES = 1;
    ProcessorVoice voices[MAX_ACTIVE_NOTES];
    int curShape;

    // The original DX7 had one single LFO. Later units had an LFO per note.
    Lfo lfo;
    
    void keydown();
    void keyup();
    void initCtrl();
        
    void resolvAppDir();
    
    void unpackOpSwitch(char packOpValue);
    void packOpSwitch();

    int16_t parameter_[2];
    int16_t pitch_;

    bool noteStart_, gatestate_;
    
public :
    // in MIDI units (0x4000 is neutral)
    Controllers controllers;
    const uint8_t *data;

    bool monoMode;

    // Extra buffering for when GetSamples wants a buffer not a multiple of N
    int16_t extra_buf[N];
    unsigned int extra_buf_size;

    // SysexComm sysexComm;
    VoiceStatus voiceStatus;
    
    void setDxValue(int offset, int v);

    //==============================================================================
    DexedAudioProcessor();
    ~DexedAudioProcessor();

    //==============================================================================
    void set_shape(int i);
    void reset();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    //void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void panic();
    
    void updateProgramFromSysex(const uint8_t *rawdata);
    void setupStartupCart();
    char *patchName();

    //================ Braids
    inline void set_pitch(int16_t pitch) {
        pitch_ = pitch;
    }

    inline int16_t pitch() const { return pitch_; }

    inline void set_parameters(
          int16_t parameter_1,
          int16_t parameter_2) {

     // controllers.modwheel_cc = parameter_1 >> 9;
     // controllers.breath_cc = parameter_2 >> 9;
     // controllers.refresh();
    }
  
  inline void Strike() {
    noteStart_ = true;
  }

  inline void set_gatestate(bool gatestate) {
    gatestate_ = gatestate;
  }
  
  void Render(const uint8_t* sync_buffer, int16_t* buffer, size_t size);
  
private:
    //==============================================================================
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DexedAudioProcessor)

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
