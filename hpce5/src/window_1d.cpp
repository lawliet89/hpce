#include "include/window_1d.hpp"

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height, const uint32_t n_levels,
               const uint8_t bit_width)
{
  // TODO: in threaded case, local vals, not static
  static uint32_t i;  // counts bytes

  static uint32_t img_w_bytes;
  static uint64_t chunks_per_img;

  static uint64_t chunk_cnt;  // counter
  static uint8_t* curr_chunk;

  //TODO
  // stuff for minwindow
  
  //TODO


  // TODO: threaded -> done once anyway
  static bool serial_hack = 0;
  if (!serial_hack) {
    // init here
    i = 0;
    img_w_bytes = (img_width_pix * 8) / bit_width;
    chunks_per_img = ((img_height * img_w_bytes) + chunk_size - 1) / chunk_size;
    chunk_cnt = 0;
    curr_chunk = in_buf;

    // MALLOC SHEI TODO

    serial_hack = 1;
  }

  for (int j = 0; j < chunk_size; ++j) {
    // if done with row, restart the window on next row
    if (i == 0) {
      // TODO
      printf("\nROWINIT\n");
    }
    printf("%x ", *(curr_chunk + j));

    if (curr_chunk + j - 2*img_w_bytes < in_buf)
      printf("^(%x) ", *(curr_chunk + j - 2*img_w_bytes + buf_size));
    else
      printf("^(%x) ", *(curr_chunk + j - 2*img_w_bytes));

    // advance byte count for current row
    i = (i + 1 == img_w_bytes) ? 0 : i + 1;  // aka (i+1)%img_w_bytes
  }
  // done with chunk
  curr_chunk = (curr_chunk + chunk_size == in_buf + buf_size)
                   ? in_buf
                   : curr_chunk + chunk_size;
  // TODO: same for out_buf

  if (++chunk_cnt == chunks_per_img) {
    // TODO
    // reset image if chunk
    printf("EXITING\n");
    return;
  }
}
