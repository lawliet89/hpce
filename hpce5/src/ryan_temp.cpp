#include <cstdlib>
#include "include/window_1d.hpp"

int main()
{
  // uint8_t srcbuf[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
  uint8_t srcbuf[24] = {6,    2,    4,    5,    3,    7,    7,    2,
                      6,    8,    7,    1,    0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  for (int i = 0; i < 4; ++i) window_1d(srcbuf, nullptr, 24, 3, 4, 3,
     1, 8);

  printf("------------------------------------\n");
  for (int i=0; i<24; ++i)
    printf("%2x ", srcbuf[i]);
  printf("\n------------------------------------\n");
}
