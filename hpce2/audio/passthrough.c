#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define DEFAULT_SAMPLES 512

int main(int argc, char *argv[])
{
	int n = DEFAULT_SAMPLES; // number of samples
	// Buffer containing n sample (left and right, both 16 bit).
	int16_t *samples;
	unsigned cbBuffer;

	if (argc > 1 ) {
		n = atoi(argv[1]);
	}

	samples = malloc(sizeof(int16_t) * n);
	cbBuffer=sizeof(sizeof(int16_t) * n);

	while(1){
		// Read one sample from input
		int got=read(STDIN_FILENO, samples, cbBuffer);
		if(got<0){
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(got==0){
			break;	 // end of file
		}

		// Copy samples to output
		int done=write(STDOUT_FILENO, samples, got);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}
	}

	free(samples);
	return 0;
}
