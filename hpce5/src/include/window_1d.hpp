#ifndef WINDOW_1D_H_
#define WINDOW_1D_H_

#include <cstdint>


//TODO: out_buf
//TODO: remove bitwidth via specialisation
//TODO: condition var+mutex argument?
void window_1d(uint8_t* const in_buf, uint8_t* const out_buf, uint64_t buf_size,
               const uint32_t chunk_size, const uint32_t img_width_pix,
               const uint32_t img_height, const uint32_t n_levels,
               const uint8_t bit_width);

#endif
