#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Frequencies of musical notes in the 0th octave (C0 to B0)
// multiply by powers of 2 to get higher octaves, e.g. A4 = A * 2^4 = 440Hz
#define N_C 16.35
#define N_Cs 17.32
#define N_D 18.35
#define N_Ds 19.45
#define N_E 20.60
#define N_F 21.83
#define N_Fs 23.12
#define N_G 24.50
#define N_Gs 25.96
#define N_A 27.50
#define N_As 29.14
#define N_B 30.87

// Wave

// Oscillator
typedef struct Osc {
	double phase;       // current position in osc
	double frequency;   // Pitch
	double amplitude;   // amplitude, volume (0 to 1)
	double sample_rate; // "frame rate" of the sound
} Osc;

typedef enum AdsrState {
	ADSR_ATTACK,
	ADSR_DECAY,
	ADSR_SUSTAIN,
	ADSR_RELEASE,
	ADSR_OFF
} AdsrState;

typedef struct Adsr {
	AdsrState state;
	double attack_time;
	double decay_time;
	double sustain_level;
	double release_time;
	double current_amplitude;
} Adsr;

// TODO: Low Frequency Oscillator (LFO) for modulation, vibrato, tremolo, etc

// Note
typedef double (*sample_func)(Osc *); // function pointer type for sample functions

typedef struct Note {
	Osc osc;
	bool adsr_enabled;
	Adsr adsr;
	sample_func sample_func; // function pointer to the sample function for this note
} Note;

Osc create_osc(double f, double a, double sr) {
	return (Osc){
	    .phase = 0,
	    .frequency = f,
	    .amplitude = a,
	    .sample_rate = sr};
}

Adsr create_adsr(double attack, double decay, double sustain, double release) {
	return (Adsr){
	    .state = ADSR_OFF,
	    .attack_time = attack,
	    .decay_time = decay,
	    .sustain_level = sustain,
	    .release_time = release,
	    .current_amplitude = 0};
}

Note create_note(double f, double a, double sr, sample_func s_fn, double attack, double decay, double sustain, double release) {
	if (attack > 0 || decay > 0 || sustain > 0 || release > 0) {
		return (Note){
		    .osc = create_osc(f, a, sr),
		    .adsr_enabled = true,
		    .adsr = create_adsr(attack, decay, sustain, release),
		    .sample_func = s_fn};
	} else {
		return (Note){
		    .osc = create_osc(f, a, sr),
		    .adsr_enabled = false,
		    .sample_func = s_fn};
	}
}

Note create_simple_note(double f, double a, double sr, sample_func s_fn) {
	return (Note){
	    .osc = create_osc(f, a, sr),
	    .adsr_enabled = false,
	    .sample_func = s_fn};
}

double sin_sample(Osc *o) {
	double sample = sin(o->phase);                          // compute the current sample
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate; // update the phase
	if (o->phase >= 2 * M_PI) {
		// 2*pi is a full phase in radians, if the osc has exceded a full
		// phase, we subtract a full phase to wrap the excess around to keep
		// it in phase, better modeling a real sound osc
		o->phase -= 2 * M_PI;
	}
	return sample;
}

double square_sample(Osc *o) {
	double sample = (o->phase < M_PI);
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate;
	if (o->phase >= 2 * M_PI) {
		o->phase -= 2 * M_PI;
	}
	return sample;
}

double saw_sample(Osc *o) {
	double sample = (o->phase / M_PI) - 1; // map phase from -1 to 1
	sample *= o->amplitude;
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate;
	if (o->phase >= 2 * M_PI) {
		o->phase -= 2 * M_PI;
	}
	return sample;
}

double triangle_sample(Osc *o) {
	double sample; // map from 0 to 2*pi
	if (o->phase < M_PI) {
		// rising from -1 to 1
		sample = -1 + (2 * o->phase / M_PI);
	} else {
		// falling from 1 to -1
		sample = 1 - (2 * (o->phase - M_PI) / M_PI);
	}
	sample *= o->amplitude;
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate;
	if (o->phase >= 2 * M_PI) {
		o->phase -= 2 * M_PI;
	}
	return sample;
}

// TODO: Pulse Wave

// Tools and Testing

void output_byte(double sample) {
	// Clamp to -1.0 to 1.0 to prevent weird wrap-around noises
	if (sample > 1.0)
		sample = 1.0;
	if (sample < -1.0)
		sample = -1.0;

	// Convert -1.0...1.0 to 0...255
	uint8_t byte = (uint8_t)(sample * 127 + 128);
	putchar(byte);
}

double noise_sample(Osc *o) {
	// random noise between -1 and 1
	double sample = ((double)rand() / RAND_MAX) * 2 - 1;
	sample *= o->amplitude;
	return sample;
}

void silence(double time, double sample_rate) {
	for (int i = 0; i < time * sample_rate; i++) {
		output_byte(0);
	}
}

void play(Note *note, double duration) {
	int total_samples = duration * note->osc.sample_rate;
	for (int i = 0; i < total_samples; i++) {

		double sample = note->sample_func(&note->osc);
		output_byte(sample);
	}
}

void play_adsr(Note *note) {
	int total_samples = (note->adsr.attack_time + note->adsr.decay_time + note->adsr.release_time) * note->osc.sample_rate;
	note->adsr.state = ADSR_ATTACK; // Start the ADSR envelope
	for (int i = 0; i < total_samples; i++) {
		// Update ADSR envelope
		switch (note->adsr.state) {
		case ADSR_ATTACK:
			note->adsr.current_amplitude += 1.0 / (note->adsr.attack_time * note->osc.sample_rate);
			if (note->adsr.current_amplitude >= 1.0) {
				note->adsr.current_amplitude = 1.0;
				note->adsr.state = ADSR_DECAY;
			}
			break;
		case ADSR_DECAY:
			note->adsr.current_amplitude -= (1.0 - note->adsr.sustain_level) / (note->adsr.decay_time * note->osc.sample_rate);
			if (note->adsr.current_amplitude <= note->adsr.sustain_level) {
				note->adsr.current_amplitude = note->adsr.sustain_level;
				note->adsr.state = ADSR_SUSTAIN;
			}
			break;
		case ADSR_SUSTAIN:
			// Sustain level is maintained until the note is released
			break;
		case ADSR_RELEASE:
			note->adsr.current_amplitude -= note->adsr.sustain_level / (note->adsr.release_time * note->osc.sample_rate);
			if (note->adsr.current_amplitude <= 0) {
				note->adsr.current_amplitude = 0;
				note->adsr.state = ADSR_OFF;
			}
			break;
		case ADSR_OFF:
			// Note is silent
			break;
		}

		double sample = note->sample_func(&note->osc) * note->adsr.current_amplitude;
		output_byte(sample);
	}
}

int main(int argc, char *argv[]) {
	double sample_rate = 44100;

	// Osc mary_had_a_little_lamb_osc[] = {
	//     create_osc(N_E * pow(2, 4), .25, sample_rate),
	//     create_osc(N_D * pow(2, 4), .25, sample_rate),
	//     create_osc(N_C * pow(2, 4), .25, sample_rate),
	//     create_osc(N_D * pow(2, 4), .25, sample_rate),
	//     create_osc(N_E * pow(2, 4), .25, sample_rate),
	//     create_osc(N_E * pow(2, 4), .25, sample_rate),
	//     create_osc(N_E * pow(2, 4), .25, sample_rate),
	// };

	// Note mary_had_a_little_lamb[] = {
	//     create_simple_note(N_E * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_D * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_C * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_D * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_E * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_E * pow(2, 4), .25, sample_rate, sin_sample),
	//     create_simple_note(N_E * pow(2, 4), .25, sample_rate, sin_sample),
	// };

	// Osc E = create_osc(N_E * pow(2, 4), .1, sample_rate);
	// Osc G = create_osc(N_G * pow(2, 4), .1, sample_rate);
	// Osc A = create_osc(N_A * pow(2, 4), .1, sample_rate);

	// for (int i = 0; i < sample_rate * 2; i++) {
	// 	double s = sin_sample(&E);
	// 	output_byte(s);
	// }
	// for (int i = 0; i < sample_rate * 2; i++) {
	// 	double s = sin_sample(&E) + sin_sample(&G);
	// 	s /= 2;
	// 	output_byte(s);
	// }
	// for (int i = 0; i < sample_rate * 2; i++) {
	// 	double s = sin_sample(&E) + sin_sample(&G) + sin_sample(&A);
	// 	s /= 3;
	// 	output_byte(s);
	// }

	// Note simple = create_simple_note(N_E * pow(2, 4), .25, sample_rate, sin_sample);
	// play(&simple, 2.0);
	// Note adsr_note = create_note(N_G * pow(2, 4), .25, sample_rate, sin_sample, 0.5, 0.5, 0.7, 1.0);
	// play_adsr(&adsr_note);

	Note drum = create_note(N_C * pow(2, 2), .5, sample_rate, noise_sample, 0.01, 0.1, 0.0, 0.5);
	play_adsr(&drum);

	Note kick = create_note(N_C * pow(2, 2), .5, sample_rate, saw_sample, 0.01, 0.5, 0.0, 0.5);
	play_adsr(&kick);

	Note snare = create_note(N_C * pow(2, 2), .5, sample_rate, square_sample, 0.01, 0.2, 0.0, 0.5);
	play_adsr(&snare);

	return EXIT_SUCCESS;
}
