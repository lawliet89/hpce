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

  // window state
  static uint32_t num_windows_assigned;
  static window_state* wss;
  static window_state* ws;

  // TODO: threaded -> done once anyway
  static bool serial_hack = 0;
  if (!serial_hack) {
    //////////////////////////
    // init here
    //////////////////////////
    i = 0;
    img_w_bytes = (img_width_pix * 8) / bit_width;
    chunks_per_img = ((img_height * img_w_bytes) + chunk_size - 1) / chunk_size;
    chunk_cnt = 0;
    curr_chunk = in_buf;

    // create and initialise rolling windows
    num_windows_assigned = n_levels;
    wss = new window_state[num_windows_assigned];
    for (int m = 0; m < num_windows_assigned; ++m) {
      window_state* ws = &wss[m];
      ws->window_size = (m + 1) * 2 + 1;
      ws->start = new win_queue_entry[ws->window_size];
      ws->q_head = ws->start;
    }

    //////////////////////////
    serial_hack = 1;
  }

  for (int j = 0; j < chunk_size; ++j) {
    // if at the start of a row, restart the windows' state
    // TODO: consider moving this to 'end of row' stage, after special window tail-drain
    if (i == 0) {
      for (int m = 0; m < num_windows_assigned; ++m) {
        ws = &wss[m];
        ws->q_tail = ws->q_head;
        ws->q_head->value = 0xffffffff;  // TODO: max different
        ws->q_head->retire_idx = ws->window_size;
      }
      // TODO
      fprintf(stderr, "\nROWINIT\n");
    }

    uint8_t curr_val = *(curr_chunk + j);

    // step all windows by 1
    for (int m = 0; m < num_windows_assigned; ++m) {
      ws = &wss[m];

      win_queue_entry* end = ws->start + ws->window_size;
      uint32_t n_depth = (ws->window_size - 1) / 2;
      if (ws->q_head->retire_idx == i) {
        ws->q_head++;
        if (ws->q_head >= end) ws->q_head = ws->start;
      }
      if (curr_val <= ws->q_head->value) {
        ws->q_head->value = curr_val;
        ws->q_head->retire_idx = i + ws->window_size;
        ws->q_tail = ws->q_head;
      } else {
        while (ws->q_tail->value >= curr_val) {
          if (ws->q_tail == ws->start) ws->q_tail = end;
          --(ws->q_tail);
        }
        ++(ws->q_tail);
        if (ws->q_tail == end) ws->q_tail = ws->start;

        ws->q_tail->value = curr_val;
        ws->q_tail->retire_idx = i + ws->window_size;
      }
      if (i >= n_depth) {
        // out[i] = ws->q_head->value;
        fprintf(stderr, "[%d] %2d: %x\n", ws->window_size, i, ws->q_head->value);
      }
      // special drain
      if (i == img_w_bytes - 1) {
        for (uint32_t ii = 1; ii <= n_depth; ++ii) {
          if (ws->q_head->retire_idx == i + ii) {
            //fprintf(stderr, "evicting: %d at %d\n", ws->q_head->value, i + ii);
            ws->q_head++;
            if (ws->q_head >= end) ws->q_head = ws->start;
          }
          fprintf(stderr, "[%d] %2d+%2d: %x\n", ws->window_size, i, ii,
                 ws->q_head->value);
        }
      }
    }

    // TODO: retain:
    // printf("%x ", *(curr_chunk + j));

    // if (curr_chunk + j - 2 * img_w_bytes < in_buf)
    //  printf("^(%x) ", *(curr_chunk + j - 2 * img_w_bytes + buf_size));
    // else
    //  printf("^(%x) ", *(curr_chunk + j - 2 * img_w_bytes));

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
    fprintf(stderr, "EXITING\n");
    return;
  }
  // TODO once: free memory at thread cleanup
}

// does 1d rolling window min/max over the given chunks of rows and accumulates
// results vertically
void window_1d_OLD(uint8_t* const in_buf, uint8_t* const out_buf,
                   uint64_t buf_size, const uint32_t chunk_size,
                   const uint32_t img_width_pix, const uint32_t img_height,
                   const uint32_t n_levels, const uint8_t bit_width)
{
  // TODO: in threaded case, local vals, not static
  static uint32_t i;  // counts bytes

  static uint32_t img_w_bytes;
  static uint64_t chunks_per_img;

  static uint64_t chunk_cnt;  // counter
  static uint8_t* curr_chunk;

  // TODO
  // stuff for minwindow

  // TODO

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

    if (curr_chunk + j - 2 * img_w_bytes < in_buf)
      printf("^(%x) ", *(curr_chunk + j - 2 * img_w_bytes + buf_size));
    else
      printf("^(%x) ", *(curr_chunk + j - 2 * img_w_bytes));

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
