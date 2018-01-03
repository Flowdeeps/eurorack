#include "renaissance.h"

void Renaissance::init(int sampleRate, int samplesPerBlock) {
	vocalist.init(sampleRate, samplesPerBlock);
	dexed.init(sampleRate, samplesPerBlock);
	set_shape(0);
}

const char *Renaissance::patchName() {
	if (currentShape < NUM_VOCALIST_PATCHES) {
		return "SAM ";
	} else {
		return "DEX ";
	}
}

void Renaissance::set_shape(int shape) {
	currentShape = shape;
	if (currentShape < NUM_VOCALIST_PATCHES) {
		vocalist.set_shape(shape);
	} else {
		dexed.set_shape(shape-NUM_VOCALIST_PATCHES);
	}
}
