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
// #include "PluginParam.h"
// #include "PluginData.h"
// #include "PluginFx.h"
// #include "SysexComm.h"
#include "EngineMkI.h"

struct ProcessorVoice {
    int midi_note;
    int velocity;
    bool keydown;
    bool sustained;
    bool live;
    Dx7Note *dx7_note;
};

enum DexedEngineResolution {
    DEXED_ENGINE_MODERN,
    DEXED_ENGINE_MARKI
};

//==============================================================================
/**
*/
class DexedAudioProcessor
{
    static const int MAX_ACTIVE_NOTES = 1;
    ProcessorVoice voices[MAX_ACTIVE_NOTES];
    int currentNote;

    // The original DX7 had one single LFO. Later units had an LFO per note.
    Lfo lfo;

    bool sustain;
    bool monoMode;
    
    // Extra buffering for when GetSamples wants a buffer not a multiple of N
    float extra_buf[N];
    int extra_buf_size;

    int currentProgram;
    
    /**
     * The last time the state was save, to be able to bypass a VST host bug.
     */
    long lastStateSave;
    
    /**
     * Plugin fx (the filter)
     */
    //PluginFx fx;

    /**
     * This flag is used in the audio thread to know if the voice has changed
     * and needs to be updated.
     */
    bool refreshVoice;
    bool normalizeDxVelocity;
    bool sendSysexChange;
    
  //  void processMidiMessage(const MidiMessage *msg);
    void keydown(uint8_t pitch, uint8_t velo);
    void keyup(uint8_t pitch);
    
    /**
     * this is called from the Audio thread to tell
     * to update the UI / hostdata 
     */
   // void handleAsyncUpdate();
    void initCtrl();

	//MidiMessage* nextMidi,*midiMsg;
	//bool hasMidiMessage;
    //int midiEventPos;
	//bool getNextEvent(MidiBuffer::Iterator* iter,const int samplePos);
    
    //void handleIncomingMidiMessage(MidiInput* source, const MidiMessage& message);
    uint32_t engineType;
    
    FmCore engineMsfa;
    EngineMkI engineMkI;
    
    char clipboard[161];
    char clipboardContent;
    
    void resolvAppDir();
    
    void unpackOpSwitch(char packOpValue);
    void packOpSwitch();
    
public :
    // in MIDI units (0x4000 is neutral)
    Controllers controllers;
    // StringArray programNames;
    uint8_t data[161];

    // SysexComm sysexComm;
    VoiceStatus voiceStatus;
    
    float vuSignal;
    bool showKeyboard;
    int getEngineType();
    void setEngineType(int rs);
    
    OperatorCtrl opCtrl[6];
    CtrlDX pitchEgRate[4];
    CtrlDX pitchEgLevel[4];
    CtrlDX pitchModSens;
    CtrlDX algo;
    CtrlDX oscSync;
    CtrlDX feedback;
    CtrlDX lfoRate;
    CtrlDX lfoDelay;
    CtrlDX lfoAmpDepth;
    CtrlDX lfoPitchDepth;
    CtrlDX lfoWaveform;
    CtrlDX lfoSync;
    CtrlDX transpose;

    CtrlFloat fxCutoff;
    CtrlFloat fxReso;
    CtrlFloat output;
    //CtrlFloat tune;

    void setDxValue(int offset, int v);

    //==============================================================================
    DexedAudioProcessor();
    ~DexedAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    //void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void panic();
    bool isMonoMode() {
        return monoMode;
    }
    void setMonoMode(bool mode);
    
    void copyToClipboard(int srcOp);
    void pasteOpFromClipboard(int destOp);
    void pasteEnvFromClipboard(int destOp);
    void sendCurrentSysexProgram();
    void sendCurrentSysexCartridge();
    bool hasClipboardContent();
    
    //==============================================================================
    //AudioProcessorEditor* createEditor();
    bool hasEditor() const;
    void updateUI();
    bool peekVoiceStatus();
    void updateProgramFromSysex(const uint8_t *rawdata);
    void setupStartupCart();
    
    //==============================================================================
    const String getName() const;
    int getNumParameters();
    float getParameter (int index);
    void setParameter (int index, float newValue);
    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram(int index);
    const String getProgramName (int index);
    void changeProgramName(int index, const String& newName);
    void resetToInitVoice();
    
    // this is kept up to date with the midi messages that arrive, and the UI component
    // registers with it so it can represent the incoming messages
    void unbindUI();

    void loadPreference();
    void savePreference();
    
private:
    //==============================================================================
    // JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DexedAudioProcessor)

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
