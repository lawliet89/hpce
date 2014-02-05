/*
 * julia_v4.c
 *
 * Code generation for function 'julia_v4'
 *
 * C source code generated on: Tue Feb 04 15:52:20 2014
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "julia_v4.h"
#include "julia_v4_emxutil.h"

/* Function Definitions */
void julia_v4(emxArray_creal_T *vz, const creal_T c, int32_T maxiter,
              emxArray_real_T *viter)
{
  int32_T i1;
  int32_T unnamed_idx_1;
  real_T vz_re;
  real_T vz_im;

  /*  for code generation poruposes */
  /* vz */
  /*  c */
  /*  maxiter  */
  i1 = viter->size[0] * viter->size[1];
  viter->size[0] = 1;
  emxEnsureCapacity((emxArray__common *)viter, i1, (int32_T)sizeof(real_T));
  unnamed_idx_1 = vz->size[1];
  i1 = viter->size[0] * viter->size[1];
  viter->size[1] = unnamed_idx_1;
  emxEnsureCapacity((emxArray__common *)viter, i1, (int32_T)sizeof(real_T));
  unnamed_idx_1 = vz->size[1];
  for (i1 = 0; i1 < unnamed_idx_1; i1++) {
    viter->data[i1] = 1.0;
  }

  i1 = vz->size[1];
  for (unnamed_idx_1 = 0; unnamed_idx_1 < i1; unnamed_idx_1++) {
    while ((viter->data[viter->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1)
             - 1)] < maxiter) && (!(muDoubleScalarHypot(vz->data[vz->size[0] *
              ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)].re, vz->data
              [vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)].im) >
             1.0))) {
      viter->data[viter->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        ++;
      vz_re = vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .re * vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .re - vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .im * vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .im;
      vz_im = vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .re * vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .im + vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .im * vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)]
        .re;
      vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)].re =
        vz_re + c.re;
      vz->data[vz->size[0] * ((int32_T)(1.0 + (real_T)unnamed_idx_1) - 1)].im =
        vz_im + c.im;
    }
  }
}

/* End of code generation (julia_v4.c) */
