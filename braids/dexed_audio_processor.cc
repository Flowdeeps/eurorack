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

#include <stdarg.h>
#include <bitset>

#include "PluginParam.h"
#include "dexed_audio_processor.h"

#include "Dexed.h"
#include "synth.h"
#include "freqlut.h"
#include "sin.h"
#include "exp2.h"
#include "env.h"
#include "pitchenv.h"
#include "aligned_buf.h"
#include "fm_op_kernel.h"

//==============================================================================
DexedAudioProcessor::DexedAudioProcessor() {
#ifdef DEBUG
    Logger *tmp = Logger::getCurrentLogger();
    if ( tmp == NULL ) {
        Logger::setCurrentLogger(FileLogger::createDateStampedLogger("Dexed", "DebugSession-", "log", "DexedAudioProcessor Created"));
    }
    TRACE("Hi");
#endif

    currentNote = -1;
        
    TRACE("controler %s", controllers.opSwitch);
    
    normalizeDxVelocity = false;
    showKeyboard = true;
    
    memset(&voiceStatus, 0, sizeof(VoiceStatus));
    setEngineType(DEXED_ENGINE_MARKI);
    
    controllers.values_[kControllerPitchRange] = 3;
    controllers.values_[kControllerPitchStep] = 0;
    controllers.masterTune = 0;

    //nextMidi = NULL;
    //midiMsg = NULL;

}

DexedAudioProcessor::~DexedAudioProcessor() {
    TRACE("Bye");
}

const char init_voice[] =
      { 99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
        99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
        99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
        99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
        99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 7,
        99, 99, 99, 99, 99, 99, 99, 00, 39, 0, 0, 0, 0, 0, 0, 0, 99, 0, 1, 0, 7,
        99, 99, 99, 99, 50, 50, 50, 50, 0, 0, 1, 35, 0, 0, 0, 1, 0, 3, 24,
        73, 78, 73, 84, 32, 86, 79, 73, 67, 69 };

//==============================================================================
void DexedAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    //Freqlut::init(sampleRate);
    Lfo::init(sampleRate);
    PitchEnv::init(sampleRate);
    Env::init_sr(sampleRate);
    //fx.init(sampleRate);
    
    for (int note = 0; note < MAX_ACTIVE_NOTES; ++note) {
        voices[note].keydown = false;
        voices[note].sustained = false;
        voices[note].live = false;
    }

    for(unsigned int i=0;i<sizeof(init_voice);i++) {
        data[i] = init_voice[i];
    }

    currentNote = 0;
    controllers.values_[kControllerPitch] = 0x2000;
    controllers.modwheel_cc = 0;
    controllers.foot_cc = 0;
    controllers.breath_cc = 0;
    controllers.aftertouch_cc = 0;

    extra_buf_size = 0;
    
    sustain = false;
    
    lfo.reset(data + 137);
}

void DexedAudioProcessor::releaseResources() {
    currentNote = -1;

    for (int note = 0; note < MAX_ACTIVE_NOTES; ++note) {
        voices[note].keydown = false;
        voices[note].sustained = false;
        voices[note].live = false;
    }
}

void DexedAudioProcessor::Render(const uint8_t* sync_buffer, int16_t* channelData, size_t numSamples) {
    unsigned int i;
    
    if ( refreshVoice ) {
        for(i=0;i < MAX_ACTIVE_NOTES;i++) {
            if ( voices[i].live )
                voices[i].dx7_note.update(data, voices[i].midi_note, voices[i].velocity);
        }
        lfo.reset(data + 137);
        refreshVoice = false;
    }

    if (noteStart_) {
        keydown(78,128);
        noteStart_ = false;
    }

    // todo apply params

    // flush first events
    for (i=0; i < numSamples && i < extra_buf_size; i++) {
        channelData[i] = extra_buf[i];
    }
    
    // remaining buffer is still to be processed
    if (extra_buf_size > numSamples) {
        for (unsigned int j = 0; j < extra_buf_size - numSamples; j++) {
            extra_buf[j] = extra_buf[j + numSamples];
        }
        extra_buf_size -= numSamples;
    } else {
        for (; i < numSamples; i += N) {
            int32_t audiobuf[N];
            //float sumbuf[N];
            
            for (int j = 0; j < N; ++j) {
                audiobuf[j] = 0;
                //sumbuf[j] = 0;
            }
            int32_t lfovalue = lfo.getsample();
            int32_t lfodelay = lfo.getdelay();
            
            for (int note = 0; note < MAX_ACTIVE_NOTES; ++note) {
                if (voices[note].live) {
                    voices[note].dx7_note.compute(&audiobuf[0], lfovalue, lfodelay, &controllers);
                    
                    // for (int j=0; j < N; ++j) {
                    //     int32_t val = audiobuf[j];
                        
                    //     val = val >> 4;
                    //     int clip_val = val < -(1 << 24) ? 0x8000 : val >= (1 << 24) ? 0x7fff : val >> 9;
                    //     float f = ((float) clip_val) / (float) 0x8000;
                    //     if( f > 1 ) f = 1;
                    //     if( f < -1 ) f = -1;
                    //     sumbuf[j] += f;
                    //     audiobuf[j] = 0;
                    // }
                }
            }
            
            // int jmax = numSamples - i;
            // for (int j = 0; j < N; ++j) {
            //     if (j < jmax) {
            //         channelData[i + j] = sumbuf[j];
            //     } else {
            //         extra_buf[j - jmax] = sumbuf[j];
            //     }
            // }
            int jmax = numSamples - i;
            for (int j = 0; j < N; ++j) {
                if (j < jmax) {
                    channelData[i + j] = audiobuf[j] >> 16;
                } else {
                    extra_buf[j - jmax] = audiobuf[j] >> 16;
                }
            }
        }
        extra_buf_size = i - numSamples;
    }
}

// void DexedAudioProcessor::processMidiMessage(const MidiMessage *msg) {
//     const uint8 *buf  = msg->getRawData();
//     uint8_t cmd = buf[0];

//     switch(cmd & 0xf0) {
//         case 0x80 :
//             keyup(buf[1]);
//         return;

//         case 0x90 :
//             keydown(buf[1], buf[2]);
//         return;
            
//         case 0xb0 : {
//             int ctrl = buf[1];
//             int value = buf[2];
            
//             switch(ctrl) {
//                 case 1:
//                     controllers.modwheel_cc = value;
//                     controllers.refresh();
//                     break;
//                 case 2:
//                     controllers.breath_cc = value;
//                     controllers.refresh();
//                     break;
//                 case 4:
//                     controllers.foot_cc = value;
//                     controllers.refresh();
//                     break;
//                 case 64:
//                     sustain = value > 63;
//                     if (!sustain) {
//                         for (int note = 0; note < MAX_ACTIVE_NOTES; note++) {
//                             if (voices[note].sustained && !voices[note].keydown) {
//                                 voices[note].dx7_note.keyup();
//                                 voices[note].sustained = false;
//                             }
//                         }
//                     }
//                     break;
//                 case 123:
//                     panic();
//                     break;
//             }
//         }
//         return;
            
//         // aftertouch
//         case 0xd0 :
//             controllers.aftertouch_cc = buf[1];
//             controllers.refresh();
//         return;
            
//     }

//     switch (cmd) {
//         case 0xe0 :
//             controllers.values_[kControllerPitch] = buf[1] | (buf[2] << 7);
//         break;
//     }
// }

void DexedAudioProcessor::keydown(uint8_t pitch, uint8_t velo) {
    if ( velo == 0 ) {
        keyup(pitch);
        return;
    }

    pitch += data[144] - 24;
    
    if ( normalizeDxVelocity ) {
        velo = ((float)velo) * 0.7874015; // 100/127
    }
    
    int note = currentNote;
    for (int i=0; i<MAX_ACTIVE_NOTES; i++) {
        if (!voices[note].keydown) {
            currentNote = (note + 1) % MAX_ACTIVE_NOTES;
            lfo.keydown();  // TODO: should only do this if # keys down was 0
            voices[note].midi_note = pitch;
            voices[note].velocity = velo;
            voices[note].sustained = sustain;
            voices[note].keydown = true;
            voices[note].dx7_note.init(data, pitch, velo);
            if ( data[136] )
                voices[note].dx7_note.oscSync();
            break;
        }
        note = (note + 1) % MAX_ACTIVE_NOTES;
    }
    
    if ( monoMode ) {
        for(int i=0; i<MAX_ACTIVE_NOTES; i++) {            
            if ( voices[i].live ) {
                // all keys are up, only transfert signal
                if ( ! voices[i].keydown ) {
                    voices[i].live = false;
                    voices[note].dx7_note.transferSignal(voices[i].dx7_note);
                    break;
                }
                if ( voices[i].midi_note < pitch ) {
                    voices[i].live = false;
                    voices[note].dx7_note.transferState(voices[i].dx7_note);
                    break;
                }
                return;
            }
        }
    }
 
    voices[note].live = true;
}

void DexedAudioProcessor::keyup(uint8_t pitch) {
    pitch += data[144] - 24;

    int note;
    for (note=0; note<MAX_ACTIVE_NOTES; ++note) {
        if ( voices[note].midi_note == pitch && voices[note].keydown ) {
            voices[note].keydown = false;
            break;
        }
    }
    
    // note not found ?
    if ( note >= MAX_ACTIVE_NOTES ) {
        TRACE("note-off not found???");
        return;
    }
    
    if ( monoMode ) {
        int highNote = -1;
        int target = 0;
        for (int i=0; i<MAX_ACTIVE_NOTES;i++) {
            if ( voices[i].keydown && voices[i].midi_note > highNote ) {
                target = i;
                highNote = voices[i].midi_note;
            }
        }
        
        if ( highNote != -1 ) {
            voices[note].live = false;
            voices[target].live = true;
            voices[target].dx7_note.transferState(voices[note].dx7_note);
        }
    }
    
    if ( sustain ) {
        voices[note].sustained = true;
    } else {
        voices[note].dx7_note.keyup();
    }
}

void DexedAudioProcessor::panic() {
    for(int i=0;i<MAX_ACTIVE_NOTES;i++) {
        voices[i].keydown = false;
        voices[i].live = false;
        voices[i].dx7_note.oscSync();
    }
}

int DexedAudioProcessor::getEngineType() {
    return engineType;
}

void DexedAudioProcessor::setEngineType(int tp) {
    TRACE("settings engine %d", tp);
    
    switch (tp)  {
        case DEXED_ENGINE_MARKI:
            controllers.core = &engineMkI;
            break;
    }
    engineType = tp;
}

// ====================================================================
bool DexedAudioProcessor::peekVoiceStatus() {
    if ( currentNote == -1 )
        return false;

    // we are trying to find the last "keydown" note
    int note = currentNote;
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        if (voices[note].keydown) {
            voices[note].dx7_note.peekVoiceStatus(voiceStatus);
            return true;
        }
        if ( --note < 0 )
            note = MAX_ACTIVE_NOTES-1;
    }

    // not found; try a live note
    note = currentNote;
    for (int i = 0; i < MAX_ACTIVE_NOTES; i++) {
        if (voices[note].live) {
            voices[note].dx7_note.peekVoiceStatus(voiceStatus);
            return true;
        }
        if ( --note < 0 )
            note = MAX_ACTIVE_NOTES-1;
    }

    return true;
}