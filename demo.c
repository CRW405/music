#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 44100.0
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void basic_sin(double duration, double frequency, int volume) {
	//
	// y = amplitude * math.sin(2*pi*frequency+time)
	//
	// amplitude = height of wave, loudness
	// frequency = waves per second, pitch
	// time = seriously?
	//

	for (int i = 0; i < SAMPLE_RATE * duration; i++) {
		double time = (double)i / SAMPLE_RATE;
		double wave = sin(2.0 * M_PI * frequency * time);
		char sample = (char)(wave * volume + 128);
		// 128 is the middle of the 0-255 range
		// for unsigned 8-bit audio
		putchar(sample);
	}
}
void sine_envelope(double duration, double frequency, int volume,
                   double attack_time, double decay_time) {
	//
	// envelope is when you ramp up the volume at the start of the note (attack)
	// and then ramp it down at the end (decay) this makes the note sound more
	// natural and less like a pure sine wave
	//
	// attack is the time it takes for the note to reach its maximum volume
	// decay is the time it takes for the note to fade out after the attack
	// the total duration of the note is attack_time + decay_time
	//

	int total_samples = (int)(SAMPLE_RATE * duration);
	int attack_samples = (int)(SAMPLE_RATE * attack_time);
	int decay_samples = (int)(SAMPLE_RATE * decay_time);

	double curr_volume = 0.0;

	// if at beginning of note, ramp up volume
	for (int i = 0; i < attack_samples; i++) {
		double time = (double)i / SAMPLE_RATE;
		double wave = sin(2.0 * M_PI * frequency * time);
		curr_volume = (double)i / attack_samples * volume; // linear ramp up
		uint8_t sample = (uint8_t)(wave * curr_volume + 128);
		putchar(sample);
	}

	// sustain at max volume
	for (int i = attack_samples; i < total_samples - decay_samples; i++) {
		double time = (double)i / SAMPLE_RATE;
		double wave = sin(2.0 * M_PI * frequency * time);
		uint8_t sample = (uint8_t)(wave * volume + 128);
		putchar(sample);
	}

	// ramp down volume at end of note
	for (int i = total_samples - decay_samples; i < total_samples; i++) {
		double time = (double)i / SAMPLE_RATE;
		double wave = sin(2.0 * M_PI * frequency * time);
		curr_volume = (double)(total_samples - i) / decay_samples * volume;
		// linear ramp down
		uint8_t sample = (uint8_t)(wave * curr_volume + 128);
		putchar(sample);
	}
}

void kick(double duration, double frequency, int volume) {
	//
	// a kick drum sound is a sine wave that starts at a high frequency and
	// quickly drops to a low frequency the envelope is a simple decay that
	// starts at full volume and quickly drops to zero
	//
	// a phase accumulator is used to keep track of the current position in the
	// sine wave, and the frequency is modulated over time to create the
	// characteristic "thump" sound of a kick drum
	//

	int total_samples = (int)(SAMPLE_RATE * duration);
	double phase = 0.0;

	for (int i = 0; i < total_samples; i++) {
		double progress = (double)i / total_samples;

		double current_freq = frequency * (1.0 - progress);
		// frequency drops over time

		// double envelope = (1 - progress) * (1 - progress);
		// simple decay envelope

		// double curr_vol = volume * envelope;
		double curr_vol = volume;

		phase += 2.0 * M_PI * current_freq / SAMPLE_RATE;
		double wave = sin(phase);
		uint8_t sample = (uint8_t)(wave * curr_vol + 128);
		putchar(sample);
	}
}

void snare(double duration, double frequency, int volume, double mix) {
	//
	// a snare drum sound is a combination of a noise burst and a sine wave
	// the noise burst creates the "snap" sound of the snare, while the sine
	// wave adds some tonal character to the sound the envelope is a simple
	// decay that starts at full volume and quickly drops to zero
	//

	int total_samples = (int)(SAMPLE_RATE * duration);
	double phase = 0.0;

	for (int i = 0; i < total_samples; i++) {
		double progress = (double)i / total_samples;

		phase += 2.0 * M_PI * frequency / SAMPLE_RATE;
		double drum_skin = sin(phase);
		// sine wave for tonal character, drops off quickly

		double wires = ((double)rand() / RAND_MAX) * 2.0 - 1.0; // white noise
		// noise burst for snap

		double envelope = (1 - progress) * (1 - progress);
		// simple decay envelope

		double mixed = (drum_skin * mix + wires * (1.0 - mix)) * envelope;
		// mix noise and sine wave

		uint8_t sample = (uint8_t)(mixed * volume + 128);
		putchar(sample);
	}
}

void silence(double duration) {
	int total_samples = (int)(SAMPLE_RATE * duration);
	for (int i = 0; i < total_samples; i++) {
		putchar(128); // middle of the range for silence
	}
}

int main(int argc, char *argv[]) {
	// // double sample_rate = 44100.0; // "frame rate" of the audio
	// // double duration = 1.0;
	// // double frequency = 261.63; // middle C
	// // int volume = 64; // 0-127 for signed 8-bit audio, 0-255 for unsigned
	//
	// float middle_c = 261.63;
	// basic_sin(1.0, middle_c, 64);
	// basic_sin(1.0, middle_c * 2, 64); // one octave above middle C
	// basic_sin(1.0, middle_c / 2, 64); // one octave below middle C
	//
	// basic_sin(1.0, middle_c, 64);
	// basic_sin(1.0, middle_c * 1.5, 64);
	// // perfect fifth above middle C
	//
	// // sounds flutey
	// float middle_f = 349.23;
	// sine_envelope(2.0, middle_f, 64, 0.5, 0.5);
	// sine_envelope(2.0, middle_f * 1.5, 64, 1.0, 0.25);
	// sine_envelope(2.0, middle_f * 1.5, 64, 0.25, 1.0);
	//
	// float middle_a = 220.0;
	// sine_envelope(1, middle_a, 64, 0.5, 0.5);
	// sine_envelope(2, middle_a * 1.5, 64, 1.0, 0.25);
	// sine_envelope(1, middle_a * 1.5, 64, 0.25, 1.0);

	// for (int i = 0; i < 4; i++) {
	// 	kick(0.2, 50.0, 100);
	// 	snare(0.2, 64);
	// 	kick(0.2, 400.0, 64);
	// 	snare(0.2, 64);
	// }

	while (1) {
		kick(0.1, 200.0, 100);
		kick(0.1, 400.0, 100);
		kick(0.1, 800.0, 100);
		kick(0.1, 400.0, 100);
		kick(0.1, 200.0, 100);
		for (int i = 0; i < 4; i++) {
			snare(0.1, 800, 100, .6);
			snare(0.1, 400, 100, .6);
			snare(0.1, 200, 100, .6);
			if (i % 2 == 1) {
				snare(.1, 1600, 50, .5);
			}
		}
	}

	// TODO: look into sequencing
	// TODO: look into concurrent beats, sample_final = sample1 + sample2 + ...
	// but make sure to clamp to 255 for unsigned 8-bit audio

	return EXIT_SUCCESS;
}
