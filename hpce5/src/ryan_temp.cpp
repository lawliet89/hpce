#include <cstdlib>
#include "include/window_1d.hpp"

int main()
{
  printf("hello\n");

  uint8_t srcbuf[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

  window_1d(srcbuf, nullptr, 12,    3, 4, 3,    999, 8);
  window_1d(srcbuf, nullptr, 12,    3, 4, 3,    999, 8);
  window_1d(srcbuf, nullptr, 12,    3, 4, 3,    999, 8);
  window_1d(srcbuf, nullptr, 12,    3, 4, 3,    999, 8);
  window_1d(srcbuf, nullptr, 12,    3, 4, 3,    999, 8);



}
