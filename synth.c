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

// ADSR Envelope
typedef enum adsrState { ATTACK, DECAY, SUSTAIN, RELEASE, DONE } adsrState;
typedef struct Adsr {
	double a_dur; // attack duration
	double d_dur; // decay duration
	double s_vol; // sustain volume (0 to 1)
	double r_dur; // release duration
	adsrState state;
	double cur;
} Adsr;

// TODO: Low Frequency Oscillator (LFO) for modulation, vibrato, tremolo, etc

typedef struct Note {
	Osc osc;
	Adsr env;
} Note;

Osc create_simple_osc(double f, double a, double sr) {
	return (Osc){.phase = 0, .frequency = f, .amplitude = a, .sample_rate = sr};
}

Adsr create_adsr(double a, double d, double s, double r) {
	return (Adsr){.a_dur = a,
	              .d_dur = d,
	              .s_vol = s,
	              .r_dur = r,
	              .state = ATTACK,
	              .cur = 0};
}

Note create_note(double f, double a, double sr, double a_dur, double d_dur,
                 double s_vol, double r_dur) {
	return (Note){.osc = create_simple_osc(f, a, sr),
	              .env = create_adsr(a_dur, d_dur, s_vol, r_dur)};
}

double sin_sample(Osc *o) {
	double sample = sin(o->phase) * o->amplitude; // compute the current sample
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate; // update the phase
	if (o->phase >= 2 * M_PI) {
		// 2*pi is a full phase in radians, if the osc has exceded a full
		// phase, we subtract a full phase to wrap the excess around to keep it
		// in phase, better modeling a real sound osc
		o->phase -= 2 * M_PI;
	}
	return sample;
}

double cos_sample(Osc *o) {
	double sample = cos(o->phase) * o->amplitude;
	o->phase += (2 * M_PI * o->frequency) / o->sample_rate;
	if (o->phase >= 2 * M_PI) {
		o->phase -= 2 * M_PI;
	}
	return sample;
}

double square_sample(Osc *o) {
	double sample = (o->phase < M_PI) ? o->amplitude : -o->amplitude;
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

void render_raw_osc(Osc *o, double (*sample)(Osc *), double duration) {
	for (int i = 0; i < o->sample_rate * duration; i++) {
		double s = sample(o);
		output_byte(s);
	}
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

int main(int argc, char *argv[]) {
	double sample_rate = 44100;

	// Osc o = create_simple_osc(500, .25, sample_rate);
	// render_raw_osc(&o, sin_sample, 2);
	// silence(0.5, sample_rate);
	// render_raw_osc(&o, square_sample, 2);
	// silence(0.5, sample_rate);
	// render_raw_osc(&o, saw_sample, 2);
	// silence(0.5, sample_rate);
	// render_raw_osc(&o, triangle_sample, 2);
	// silence(0.5, sample_rate);
	// render_raw_osc(&o, noise_sample, 2);

	Osc E = create_simple_osc(N_E * pow(2, 4), .25, sample_rate);
	Osc G = create_simple_osc(N_G * pow(2, 4), .25, sample_rate);
	Osc A = create_simple_osc(N_A * pow(2, 4), .25, sample_rate);

	for (int i = 0; i < sample_rate * 2; i++) {
		double s = sin_sample(&E);
		output_byte(s);
	}
	for (int i = 0; i < sample_rate * 2; i++) {
		double s = sin_sample(&E) + sin_sample(&G);
		output_byte(s);
	}
	for (int i = 0; i < sample_rate * 4; i++) {
		double s = sin_sample(&E) + sin_sample(&G) + sin_sample(&A);
		output_byte(s);
	}
	// silence(0.5, sample_rate);

	Osc mary_had_a_little_lamb[] = {
	    create_simple_osc(N_E * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_D * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_C * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_D * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_E * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_E * pow(2, 4), .25, sample_rate),
	    create_simple_osc(N_E * pow(2, 4), .25, sample_rate),
	};

	// for (int i = 0; i < 7; i++) {
	// 	for (int j = 0; j < sample_rate * 0.5; j++) {
	// 		double s = sin_sample(&mary_had_a_little_lamb[i]);
	// 		output_byte(s);
	// 	}
	// }

	Osc E1 = create_simple_osc(N_E * pow(2, 4), 1, sample_rate);
	Osc E2 = create_simple_osc(N_E * pow(2, 4), 1, sample_rate);

	// E2.phase = M_PI;
	//
	// for (int i = 0; i < sample_rate * 2; i++) {
	// 	double s = sin_sample(&E1) + sin_sample(&E2);
	// 	output_byte(s);
	// }

	return EXIT_SUCCESS;
}
