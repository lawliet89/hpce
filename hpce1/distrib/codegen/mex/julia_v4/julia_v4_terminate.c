/*
 * julia_v4_terminate.c
 *
 * Code generation for function 'julia_v4_terminate'
 *
 * C source code generated on: Tue Feb 04 15:52:20 2014
 *
 */

/* Include files */
#include "rt_nonfinite.h"
#include "julia_v4.h"
#include "julia_v4_terminate.h"

/* Function Definitions */
void julia_v4_atexit(void)
{
  emlrtCreateRootTLS(&emlrtRootTLSGlobal, &emlrtContextGlobal, NULL, 1);
  emlrtEnterRtStackR2012b(emlrtRootTLSGlobal);
  emlrtLeaveRtStackR2012b(emlrtRootTLSGlobal);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

void julia_v4_terminate(void)
{
  emlrtLeaveRtStackR2012b(emlrtRootTLSGlobal);
  emlrtDestroyRootTLS(&emlrtRootTLSGlobal);
}

/* End of code generation (julia_v4_terminate.c) */
