#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define SAMPLE_SIZE 512
#define DEFAULT_SINE_FREQ 50
#define AMPLITUDE 30000
#define SAMPLE_RATE 44100
#define SAMPLE_INTERVAL 1.0/SAMPLE_RATE

#define MIN(a, b) a < b ? a : b

// 2 PI phase split over

int main(int argc, char *argv[])
{
	int16_t samples[SAMPLE_SIZE], buffer[SAMPLE_SIZE];
	unsigned cbBuffer = sizeof(int16_t) * SAMPLE_SIZE;
	int file1, file2;
	int file1Read, file2Read;
	int minBytesRead;
	int i, end;

	if (argc < 3 ) {
		fprintf(stderr, "Two input files needed");
		exit(1);
	}

	file1 = open(argv[1], O_RDONLY);
	file2 = open(argv[2], O_RDONLY);

	if (!file1 || !file2) {
		fprintf(stderr, "Cannot open input files");
		exit(1);
	}

	while(1){
		// Read from files
		file1Read = read(file1, samples, cbBuffer);
		file2Read = read(file2, buffer, cbBuffer);

		if (!file1Read || !file2Read) break;
		minBytesRead = MIN(file1Read, file2Read);

		for (i = 0, end = minBytesRead/sizeof(int16_t); i < end; ++i) {
			samples[i] = samples[i]/2 + buffer[i]/2;
		}

		// Copy samples to output
		int done=write(STDOUT_FILENO, samples, minBytesRead);
		if(done<0){
			fprintf(stderr, "%s : Write to stdout failed, error=%s.", argv[0], strerror(errno));
			exit(1);
		}else if(done!=minBytesRead){
			fprintf(stderr, "%s : Could not read requested number of bytes from stream.\n", argv[0]);
		}
	}

	return 0;
}
