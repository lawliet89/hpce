#include <unistd.h>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cmath>

int main(int argc, char *argv[]) {
  try {
    if (argc < 3) {
      fprintf(stderr, "Usage: process width height [bits] [levels]\n");
      fprintf(stderr, "   bits=8 by default\n");
      fprintf(stderr, "   levels=1 by default\n");
      exit(1);
    }

    unsigned w = atoi(argv[1]);
    unsigned h = atoi(argv[2]);

    unsigned bits = 8;
    if (argc > 3) {
      bits = atoi(argv[3]);
    }

    if (bits > 32)
      throw std::invalid_argument("Bits must be <= 32.");

    unsigned tmp = bits;
    while (tmp != 1) {
      tmp >>= 1;
      if (tmp == 0)
        throw std::invalid_argument("Bits must be a binary power.");
    }

    if (((w * bits) % 64) != 0) {
      throw std::invalid_argument(" width*bits must be divisible by 64.");
    }

    int levels = 1;
    if (argc > 4) {
      levels = atoi(argv[4]);
    }

    if (abs(levels) > std::min(std::min(w / 4u, h / 4u), 64u)) {
      throw std::invalid_argument(
          "0 <= abs(levels) <= min(width/4, height/4, 64)");
    }

    fprintf(stderr, "Processing %d x %d image with %d bits per pixel.\n", w, h,
            bits);
  }
  catch (std::exception &e) {
    std::cerr << "Caught exception : " << e.what() << "\n";
    return 1;
  }
}
