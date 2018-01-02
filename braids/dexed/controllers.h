/*
 * Copyright 2013 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CONTROLLERS_H
#define __CONTROLLERS_H

#include "synth.h"
#include "Dexed.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

class FmCore;

struct FmMod {
    int range;
    bool pitch;
    bool amp;
    bool eg;
    
    FmMod() {
        range = 0;
        pitch = false;
        amp = false;
        eg = false;
    }
};

class Controllers {
    void applyMod(int cc, FmMod &mod) {
        float range = 0.01 * mod.range;
        int total = cc * range;
        if ( mod.amp )
            amp_mod = max(amp_mod, total);
        
        if ( mod.pitch )
            pitch_mod = max(pitch_mod, total);
        
        if ( mod.eg )
            eg_mod = max(eg_mod, total);
    }
    
public:    
    char opSwitch[7];
    
    int amp_mod;
    int pitch_mod;
    int eg_mod;

    int timbre_amount;
    int color_amount;
        
    FmMod timbre;
    FmMod color;
    
    Controllers() {
        amp_mod = 0;
        pitch_mod = 0;
        eg_mod = 0;
        strcpy(opSwitch, "111111");
    }

    void defaults() {
        timbre.range = 100;
        timbre.eg = true;

        color.range = 100;
        color.amp = true;
    }

    void refresh() {
        amp_mod = 0;
        pitch_mod = 0;
        eg_mod = 0;
        
        applyMod(timbre_amount, timbre);
        applyMod(color_amount, color);
        
        if (!(timbre.eg || color.eg))
            eg_mod = 127;
    }
    
    FmCore *core;
};

#endif  // __CONTROLLERS_H

