#ifndef __RENAISSANCE_H__
#define __RENAISSANCE_H__

#include "dexed/dexed_audio_processor.h"
#include "vocalist/vocalist.h"

class Renaissance {
public:
	Renaissance() { }
	void init(int sampleRate, int samplesPerBlock);
	const char *patchName();

	inline void Render(const uint8_t* sync_buffer, int16_t* buffer, size_t size) {
		if (currentShape < NUM_VOCALIST_PATCHES) {
			vocalist.Render(sync_buffer, buffer, size);
		} else {
			dexed.Render(sync_buffer, buffer, size);
		}
	}

    void set_shape(int i);

	inline void set_pitch(int16_t pitch) {
		dexed.set_pitch(pitch);
		vocalist.set_pitch(pitch);
	}

	inline void set_parameters(
          int16_t parameter_1,
          int16_t parameter_2) {

	    dexed.set_parameters(parameter_1, parameter_2);
	    vocalist.set_parameters(parameter_1, parameter_2);
    }

    inline void Strike() {
    	dexed.Strike();
    	vocalist.Strike();
  	}

	inline void set_gatestate(bool gatestate) {
		dexed.set_gatestate(gatestate);
		vocalist.set_gatestate(gatestate);
	}

	Vocalist vocalist;
	DexedAudioProcessor dexed;
	int currentShape;
};

#endif