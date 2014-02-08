#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define CHANNELS 2

int main(int argc, char *argv[])
{
	if (argc < 2 ) {
		fprintf(stderr, "Needs an input file for coefficients");
		exit(1);
	}

	// Handle coefficients to determine frame size
	FILE *coefficientFile = fopen(argv[1], "r");

	if (!coefficientFile) {
		fprintf(stderr, "Unable to open coefficient file %s", argv[1]);
		exit(1);
	}

	// Determine frame size
	unsigned filterOrder = 0;
	double temp;

	while(!feof(coefficientFile)) {
		fscanf(coefficientFile, "%lf", &temp);
		filterOrder++;
	}

	double *coefficients = malloc(sizeof(double) * filterOrder);

	rewind(coefficientFile);

	// Read coefficients
	for (int i = 0; i < filterOrder; ++i)
		fscanf(coefficientFile, "%lf", coefficients + i);

	fclose(coefficientFile);

	// Buffer Samples
	unsigned bufferSize = CHANNELS * filterOrder;
	int16_t *samples = calloc(bufferSize, sizeof(int16_t));
	int16_t *previous = calloc(bufferSize, sizeof(int16_t));
	int16_t *output = calloc(bufferSize, sizeof(int16_t));

	while(1){
		// Read frame
		int got = read(STDIN_FILENO, samples, sizeof(int16_t) * bufferSize);

		if (got<0) {
			fprintf(stderr, "%s : Read from stdin failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		} else if(got==0) {
			break;	 // end of file
		}

		for (int i = 0, end = got/sizeof(int16_t)/CHANNELS; i < end; ++i) {
			int16_t *readPtr = samples + i*2;
			double currentLeft = 0, currentRight = 0;
			for (int j = 0; j < i; ++j) {
				currentLeft += *readPtr * coefficients[j];
				currentRight += *(readPtr + 1) * coefficients[j];

				readPtr -= 2;
			}

			readPtr = previous + bufferSize - 2;
			for (int j = i; j < filterOrder; ++j) {
				currentLeft += *readPtr * coefficients[j];
				currentRight += *(readPtr + 1) * coefficients[j];

				readPtr -= 2;
			}

			output[i*2] = (int16_t) currentLeft;
			output[i*2 + 1] = (int16_t) currentRight;
		}


		// Copy samples to output
		int done=write(STDOUT_FILENO, output, got);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}

		// swap buffers
		int16_t *tempPtr = samples;
		samples = previous;
		previous = tempPtr;
	}

	free(samples);
	return 0;
}
