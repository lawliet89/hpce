#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define CHANNELS 2
#define MIN(a, b) a < b ? a : b

// 2 PI phase split over

int main(int argc, char *argv[])
{
	int file1, file2;
	int file1Read, file2Read;
	int minBytesRead;
	int i, end;
	int windowSize, bufferSize;

	if (argc < 4 ) {
		fprintf(stderr, "Two input files and window size needed!");
		exit(1);
	}

	file1 = open(argv[1], O_RDONLY);
	file2 = open(argv[2], O_RDONLY);
	windowSize = atoi(argv[3]);

	if (!file1 || !file2) {
		fprintf(stderr, "Cannot open input files");
		exit(1);
	}

	bufferSize = sizeof(int16_t) * CHANNELS * windowSize;
	int16_t *buffer1 = malloc(bufferSize);
	int16_t *buffer2 = malloc(bufferSize);

	double rmsProduct = 0, file1msSum = 0, file2msSum = 0;

	while(1){
		// Read from files
		file1Read = read(file1, buffer1, bufferSize);
		file2Read = read(file2, buffer2, bufferSize);

		if (!file1Read || !file2Read) break;
		minBytesRead = MIN(file1Read, file2Read);

		double file1SquareSum = 0, file2SquareSum = 0;

		for (i = 0, end = minBytesRead/sizeof(int16_t)/CHANNELS; i < end; ++i) {
			double file1Mono = (buffer1[i*2] + buffer1[i*2 + 1])/2;
			double file2Mono = (buffer2[i*2] + buffer2[i*2 + 1])/2;

			file1SquareSum += file1Mono*file1Mono;
			file2SquareSum += file2Mono*file2Mono;
		}

		file1msSum += file1SquareSum/i;
		file2msSum += file2SquareSum/i;
		rmsProduct += sqrt(file1SquareSum/i) * sqrt(file2SquareSum/i);
	}

	double correlation = rmsProduct/sqrt(file1msSum * file2msSum);
	printf("%lf", correlation);

	return 0;
}
