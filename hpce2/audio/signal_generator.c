#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define SAMPLE_SIZE 512
#define DEFAULT_SINE_FREQ 50
#define AMPLITUDE 30000
#define SAMPLE_RATE 44100
#define SAMPLE_INTERVAL 1.0/SAMPLE_RATE

int main(int argc, char *argv[])
{
	int freq = DEFAULT_SINE_FREQ;
	// Buffer containing n sample (left and right, both 16 bit).
	int16_t samples[SAMPLE_SIZE];
	unsigned cbBuffer = sizeof(int16_t) * SAMPLE_SIZE;
	int i;
	double phase = 0, t = 0;

	if (argc > 1 ) {
		freq = atoi(argv[1]);
	}

	while(1){
		// Generate samples
		for (i = 0; i < SAMPLE_SIZE; i += 2) {
			t += SAMPLE_INTERVAL;
			phase += 2*M_PI*freq*SAMPLE_INTERVAL;

			if (phase > 2*M_PI) {
				phase = fmod(phase, 2*M_PI);
				t = fmod(t, 1.0/freq);
			}

			samples[i] = (int16_t) (AMPLITUDE * sin(phase));
			samples[i + 1] = samples[i];
		}
		// Copy samples to output
		int done=write(STDOUT_FILENO, samples, cbBuffer);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(done!=cbBuffer){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
		}
	}

	return 0;
}
