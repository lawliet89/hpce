#include <cstdlib>
#include "include/window_1d.hpp"

int main()
{
/*
  // uint8_t srcbuf[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
  uint8_t srcbuf[32] = {6,    2,    4,    5,    3,    7,    7,    2,
                        6,    8,    7,    1,    0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
*/
  uint8_t srcbuf[32] = {1,    7,    8,    9,    6,    4,    3,    5,
                        8,    5,    4,    6,    0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  
  printf("------------------------------------\n");
  for (int i = 0; i < 32; ++i) printf("%2x ", srcbuf[i]);
  printf("\n------------------------------------\n");
  
  
  
  for (int i = 0; i < 4; ++i) window_1d(srcbuf, nullptr, 32, 4, 4, 3, 2, 8);



  printf("------------------------------------\n");
  for (int i = 0; i < 32; ++i) printf("%2x ", srcbuf[i]);
  printf("\n------------------------------------\n");
}
