#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"

struct pairs
{
  uint32_t value;
  unsigned int death;
};

struct pairs *ring;

void minwindow_ptr(uint32_t *in, uint32_t *out, int n, const unsigned int k)
{
  int i;
  struct pairs *minpair;
  struct pairs *end;
  struct pairs *last;

  end = ring + k;
  last = ring;
  minpair = ring;
  minpair->value = in[0];
  minpair->death = k;
  out[0] = in[0];

  for (i = 1; i < n; i++) {
    if (minpair->death == i) {
      minpair++;
      if (minpair >= end) minpair = ring;
    }
    if (in[i] <= minpair->value) {
      minpair->value = in[i];
      minpair->death = i + k;
      last = minpair;
    } else {
      while (last->value >= in[i]) {
        if (last == ring) last = end;
        --last;
      }
      ++last;
      if (last == end) last = ring;
      last->value = in[i];
      last->death = i + k;
    }
    out[i] = minpair->value;
  }
}

//////////////////////////////
struct win_queue_entry
{
  uint32_t value;
  unsigned int retire_idx;
};

struct window_state
{
  uint32_t window_size;
  win_queue_entry *start;
  win_queue_entry *end;  // needed?
  win_queue_entry *q_head;
  win_queue_entry *q_tail;
};

void wrap_test(uint32_t *in, uint32_t *out, int n, const unsigned int k)
{
  int i = 0;
  window_state wss[1];

  // init

  window_state *ws = &wss[0];

  // once TODO: free memory
  ws->window_size = k;
  ws->start = new win_queue_entry[k];
  ws->end = ws->start + k;
  ws->q_head = ws->start;

  // per row
  ws->q_tail = ws->q_head;
  ws->q_head->value = 0xffffffff;  //|TODO: max different
  ws->q_head->retire_idx = ws->window_size;

  for (uint32_t i = 0; i < n; ++i) {
    uint32_t curr_val = in[i];

    // here would be the binding of ptr to
    // FOR ALL WINDOWS:
    window_state *ws = &wss[0];

    if (ws->q_head->retire_idx == i) {
      ws->q_head++;
      if (ws->q_head >= ws->end) ws->q_head = ws->start;
    }
    if (curr_val <= ws->q_head->value) {
      ws->q_head->value = curr_val;
      ws->q_head->retire_idx = i + ws->window_size;
      ws->q_tail = ws->q_head;
    } else {
      while (ws->q_tail->value >= curr_val) {
        if (ws->q_tail == ws->start) ws->q_tail = ws->end;
        --(ws->q_tail);
      }
      ++(ws->q_tail);
      if (ws->q_tail == ws->end) ws->q_tail = ws->start;

      ws->q_tail->value = curr_val;
      ws->q_tail->retire_idx = i + k;
    }
    if (i >= (ws->window_size - 1) / 2) {
      // out[i] = ws->q_head->value;
      printf("%2d: %x\n", i, ws->q_head->value);
    }
    // special drain
    if (i == n - 1) {
      for (uint32_t ii = 1; ii <= (ws->window_size - 1) / 2; ++ii) {
        if (ws->q_head->retire_idx == i+ii) {
          ws->q_head++;
          if (ws->q_head >= ws->end) ws->q_head = ws->start;
        }
        printf("%2d+%2d: %x\n", i, ii, ws->q_head->value);
      }
    }

  }
}

void mi7777nwindow_ptr(uint32_t *in, uint32_t *out, int n, const unsigned int k)
{
  int i;
  struct pairs *minpair;
  struct pairs *end;
  struct pairs *last;

  end = ring + k;
  last = ring;
  minpair = ring;
  minpair->value = in[0];
  minpair->death = k;
  out[0] = in[0];

  for (i = 1; i < n; i++) {
    if (minpair->death == i) {
      minpair++;
      if (minpair >= end) minpair = ring;
    }
    if (in[i] <= minpair->value) {
      minpair->value = in[i];
      minpair->death = i + k;
      last = minpair;
    } else {
      while (last->value >= in[i]) {
        if (last == ring) last = end;
        --last;
      }
      ++last;
      if (last == end) last = ring;
      last->value = in[i];
      last->death = i + k;
    }
    out[i] = minpair->value;
  }
}

/////

struct asc_min_entry
{
  uint32_t value;
  unsigned int retire_idx;
};

void minwindow_idx(uint32_t *in, uint32_t *out, int n, const unsigned int k)
{
  // asc_min_entry buf[k];
  asc_min_entry *buf = (asc_min_entry *)ring;
  unsigned a_head = 0;
  unsigned a_tail = 0;

  buf[a_head].value = in[0];
  buf[a_head].retire_idx = k;
  out[0] = in[0];

  for (int i = 0; i < n; ++i) {
    if (buf[a_head].retire_idx == i) {
      a_head = (a_head + 1) % k;
    }
    if (in[i] <= buf[a_head].value) {
      buf[a_head].value = in[i];
      buf[a_head].retire_idx = i + k;
      a_tail = a_head;
    } else {
      while (buf[a_tail].value >= in[i]) {
        a_tail = (a_tail - 1 + k) % k;
      }  // consider pre-checking next iteration... to not do back->forwards as
         // the last two steps...
      a_tail = (a_tail + 1) % k;
      buf[a_tail].value = in[i];
      buf[a_tail].retire_idx = i + k;
    }
    out[i] = buf[a_head].value;
  }

  // could push all minimum into the reverse scan without explicit check for new
  // best min, but would need a limiting condition on backscan to not go past
  // the A[0] anyway...
}

void minwindow_idx2(uint32_t *in, uint32_t *out, int n)
{
  // asc_min_entry buf[k];

  const unsigned int k = 1 << 7;

  asc_min_entry *buf = (asc_min_entry *)ring;
  unsigned a_head = 0;
  unsigned a_tail = 0;

  buf[a_head].value = in[0];
  buf[a_head].retire_idx = k;
  out[0] = in[0];

  for (int i = 0; i < n; ++i) {
    if (buf[a_head].retire_idx == i) {
      a_head = (a_head + 1) % k;
    }
    if (in[i] <= buf[a_head].value) {
      buf[a_head].value = in[i];
      buf[a_head].retire_idx = i + k;
      a_tail = a_head;
    } else {
      while (buf[a_tail].value >= in[i]) {
        a_tail = (a_tail - 1 + k) % k;
      }  // consider pre-checking next iteration... to not do back->forwards as
         // the last two steps...
      a_tail = (a_tail + 1) % k;
      buf[a_tail].value = in[i];
      buf[a_tail].retire_idx = i + k;
    }
    out[i] = buf[a_head].value;
  }

  // could push all minimum into the reverse scan without explicit check for new
  // best min, but would need a limiting condition on backscan to not go past
  // the A[0] anyway...
}

int main()
{
  const unsigned sz = 10;
  uint32_t a[sz] = {0, 5, 4, 3, 2, 1, 5, 6, 7, 4};
  // const unsigned sz = 1 << 22;
  // auto a = new uint32_t[sz];
  auto b = new uint32_t[sz];

  ring = (pairs *)malloc(sz * sizeof *ring);
  if (!ring) exit(EXIT_FAILURE);

  // for (int j = 0; j < sz; ++j) a[j] = rand() % 16;

  // for (int i = 0; i < (1 << 10); ++i) minwindow_ptr(a, b, sz, 3);
  // for (int i = 0; i < (1 << 10); ++i) minwindow_ptr_extra_broken(a, b, sz,
  // 129);
  // for (int i = 0; i < (1 << 10); ++i) minwindow_idx(a, b, sz, 64);
  // for (int i = 0; i < (1 << 10); ++i) minwindow_idx2(a, b, sz);

  // minwindow_ptr(a, b, sz, 3);
  wrap_test(a, b, sz, 5);

  for (int i = 0; i < sz; ++i) {
    printf("%4d ", a[i]);
  }
  printf("\n");

  for (int i = 0; i < sz; ++i) {
    printf("%4d ", b[i]);
  }
  printf("\n");

  free(ring);

  return 0;
}
