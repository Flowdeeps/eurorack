#include <stdio.h>
#include <math.h>

int midinote_to_logfreq(int midinote) {
    const int base = 50857777;  // (1 << 24) * (log(440) / log(2) - 69/12)
    const int step = (1 << 24) / 12;
    return base + step * midinote;
}

const int coarsemul[] = {
    -16777216, 0, 16777216, 26591258, 33554432, 38955489, 43368474, 47099600,
    50331648, 53182516, 55732705, 58039632, 60145690, 62083076, 63876816,
    65546747, 67108864, 68576247, 69959732, 71268397, 72509921, 73690858,
    74816848, 75892776, 76922906, 77910978, 78860292, 79773775, 80654032,
    81503396, 82323963, 83117622
};

int osc_freq(int midinote, int mode, int coarse, int fine, int detune) {
    // TODO: pitch randomization
    int logfreq;
    if (mode == 0) {
        logfreq = midinote_to_logfreq(midinote);
        logfreq += coarsemul[coarse & 31];
        if (fine) {
            // (1 << 24) / log(2)
            logfreq += (int)floor(24204406.323123 * log(1 + 0.01 * fine) + 0.5);
        }
        // This was measured at 7.213Hz per count at 9600Hz, but the exact
        // value is somewhat dependent on midinote. Close enough for now.
        logfreq += 12606 * (detune - 7);
    } else {
        // ((1 << 24) * log(10) / log(2) * .01) << 3
        logfreq = (4458616 * ((coarse & 3) * 100 + fine)) >> 3;
        logfreq += detune > 7 ? 13457 * (detune - 7) : 0;
    }
    return logfreq;
}

int main(int argc, char **argv) {
	for (int i = 0; i < 127; i++) {
		printf("%d, %d, %d\n", i, midinote_to_logfreq(i), osc_freq(i, 0, 0, 0, 0));
	}
}