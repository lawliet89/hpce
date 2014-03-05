#ifndef WINDOW_1D_H_
#define WINDOW_1D_H_

// holds value and retirement index of ascending minima or descending maxima
struct win_queue_entry
{
  uint32_t value;
  uint32_t retire_idx;
};


// scans 

//TODO: out_buf
//TODO: remove bitwidth via specialisation
//TODO: condition var+mutex argument?
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, const uint32_t chunk_size, const uint32_t img_width, const uint32_t n_levels, const uint8_t bit_width);
// chunk size in bytes


// 1 window, 1 byte only, horizontal, chunked

// n windows, 1 byte only, horiz

// n windows diamond, 1 byte only

// all bitness

// compile-time generation of unpacked funcs

#endif
