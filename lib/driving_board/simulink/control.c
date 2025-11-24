/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control.c
 *
 * Code generated for Simulink model 'control'.
 *
 * Model version                  : 1.29
 * Simulink Coder version         : 24.2 (R2024b) 21-Jun-2024
 * C/C++ source code generated on : Mon Nov 17 11:34:47 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: AMD->x86-64 (Linux 64)
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control.h"
#include <math.h>
#include <emmintrin.h>
#include "rtwtypes.h"
#include <stddef.h>
#define NumBitsPerChar                 8U

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
extern real_T rt_atan2d_snf(real_T u0, real_T u1);
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);

/*===========*
 * Constants *
 *===========*/
#define RT_PI                          3.14159265358979323846
#define RT_PIF                         3.1415927F
#define RT_LN_10                       2.30258509299404568402
#define RT_LN_10F                      2.3025851F
#define RT_LOG10E                      0.43429448190325182765
#define RT_LOG10EF                     0.43429449F
#define RT_E                           2.7182818284590452354
#define RT_EF                          2.7182817F

/*
 * UNUSED_PARAMETER(x)
 *   Used to specify that a function parameter (argument) is required but not
 *   accessed by the function body.
 */
#ifndef UNUSED_PARAMETER
#if defined(__LCC__)
#define UNUSED_PARAMETER(x)                                      /* do nothing */
#else

/*
 * This is the semi-ANSI standard way of indicating that an
 * unused function parameter is required.
 */
#define UNUSED_PARAMETER(x)            (void) (x)
#endif
#endif

#define NOT_USING_NONFINITE_LITERALS   1

extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static void rt_InitInfAndNaN(size_t realSize);
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
typedef struct {
  struct {
    uint32_T wordH;
    uint32_T wordL;
  } words;
} BigEndianIEEEDouble;

typedef struct {
  struct {
    uint32_T wordL;
    uint32_T wordH;
  } words;
} LittleEndianIEEEDouble;

typedef struct {
  union {
    real32_T wordLreal;
    uint32_T wordLuint;
  } wordL;
} IEEESingle;

real_T rtInf;
real_T rtMinusInf;
real_T rtNaN;
real32_T rtInfF;
real32_T rtMinusInfF;
real32_T rtNaNF;
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);

/*
 * Initialize rtNaN needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetNaN(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T nan = 0.0;
  if (bitsPerReal == 32U) {
    nan = rtGetNaNF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF80000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    nan = tmpVal.fltVal;
  }

  return nan;
}

/*
 * Initialize rtNaNF needed by the generated code.
 * NaN is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetNaNF(void)
{
  IEEESingle nanF = { { 0.0F } };

  nanF.wordL.wordLuint = 0xFFC00000U;
  return nanF.wordL.wordLreal;
}

/*
 * Initialize the rtInf, rtMinusInf, and rtNaN needed by the
 * generated code. NaN is initialized as non-signaling. Assumes IEEE.
 */
static void rt_InitInfAndNaN(size_t realSize)
{
  (void) (realSize);
  rtNaN = rtGetNaN();
  rtNaNF = rtGetNaNF();
  rtInf = rtGetInf();
  rtInfF = rtGetInfF();
  rtMinusInf = rtGetMinusInf();
  rtMinusInfF = rtGetMinusInfF();
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
  return (boolean_T)((value==rtInf || value==rtMinusInf) ? 1U : 0U);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
  return (boolean_T)(((value)==rtInfF || (value)==rtMinusInfF) ? 1U : 0U);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
  boolean_T result = (boolean_T) 0;
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  if (bitsPerReal == 32U) {
    result = rtIsNaNF((real32_T)value);
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.fltVal = value;
    result = (boolean_T)((tmpVal.bitVal.words.wordH & 0x7FF00000) == 0x7FF00000 &&
                         ( (tmpVal.bitVal.words.wordH & 0x000FFFFF) != 0 ||
                          (tmpVal.bitVal.words.wordL != 0) ));
  }

  return result;
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
  IEEESingle tmp;
  tmp.wordL.wordLreal = value;
  return (boolean_T)( (tmp.wordL.wordLuint & 0x7F800000) == 0x7F800000 &&
                     (tmp.wordL.wordLuint & 0x007FFFFF) != 0 );
}

/*
 * Initialize rtInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T inf = 0.0;
  if (bitsPerReal == 32U) {
    inf = rtGetInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0x7FF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    inf = tmpVal.fltVal;
  }

  return inf;
}

/*
 * Initialize rtInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetInfF(void)
{
  IEEESingle infF;
  infF.wordL.wordLuint = 0x7F800000U;
  return infF.wordL.wordLreal;
}

/*
 * Initialize rtMinusInf needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real_T rtGetMinusInf(void)
{
  size_t bitsPerReal = sizeof(real_T) * (NumBitsPerChar);
  real_T minf = 0.0;
  if (bitsPerReal == 32U) {
    minf = rtGetMinusInfF();
  } else {
    union {
      LittleEndianIEEEDouble bitVal;
      real_T fltVal;
    } tmpVal;

    tmpVal.bitVal.words.wordH = 0xFFF00000U;
    tmpVal.bitVal.words.wordL = 0x00000000U;
    minf = tmpVal.fltVal;
  }

  return minf;
}

/*
 * Initialize rtMinusInfF needed by the generated code.
 * Inf is initialized as non-signaling. Assumes IEEE.
 */
static real32_T rtGetMinusInfF(void)
{
  IEEESingle minfF;
  minfF.wordL.wordLuint = 0xFF800000U;
  return minfF.wordL.wordLreal;
}

real_T rt_atan2d_snf(real_T u0, real_T u1)
{
  real_T y;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = (rtNaN);
  } else if (rtIsInf(u0) && rtIsInf(u1)) {
    int32_T tmp;
    int32_T tmp_0;
    if (u0 > 0.0) {
      tmp = 1;
    } else {
      tmp = -1;
    }

    if (u1 > 0.0) {
      tmp_0 = 1;
    } else {
      tmp_0 = -1;
    }

    y = atan2(tmp, tmp_0);
  } else if (u1 == 0.0) {
    if (u0 > 0.0) {
      y = RT_PI / 2.0;
    } else if (u0 < 0.0) {
      y = -(RT_PI / 2.0);
    } else {
      y = 0.0;
    }
  } else {
    y = atan2(u0, u1);
  }

  return y;
}

/* Model step function */
void control_step(void)
{
  __m128d tmp;
  __m128d tmp_0;
  __m128d tmp_1;
  __m128d tmp_2;
  real_T rtb_Sum_l[6];
  real_T y[4];
  real_T rtb_R_L;
  real_T rtb_R_R;
  real_T rtb_deltaL;
  real_T rtb_deltaR;
  real_T rtb_wheel_speed_LF;
  real_T rtb_wheel_speed_RF;
  int32_T i;
  boolean_T b_y;
  boolean_T exitg1;

  /* Outport: '<Root>/controlb' */
  for (i = 0; i < 6; i++) {
    /* UnitDelay: '<S1>/Unit Delay' */
    rtb_deltaL = rtDW.UnitDelay_DSTATE[i];

    /* Saturate: '<S1>/Saturation' */
    if (rtb_deltaL > 24.0) {
      rtY.controlb[i] = 24.0;
    } else if (rtb_deltaL < -24.0) {
      rtY.controlb[i] = -24.0;
    } else {
      rtY.controlb[i] = rtb_deltaL;
    }

    /* End of Saturate: '<S1>/Saturation' */
  }

  /* End of Outport: '<Root>/controlb' */

  /* MATLAB Function: '<S3>/MATLAB Function2' incorporates:
   *  Constant: '<S3>/Constant4'
   *  Constant: '<S56>/Constant'
   *  Inport: '<Root>/dist2goal'
   *  RelationalOperator: '<S56>/Compare'
   */
  if (!rtDW.v_not_empty) {
    rtDW.v = 0.3;
    rtDW.v_not_empty = true;
  }

  if (rtU.dist2goal <= 1.0) {
    rtDW.v -= 5.0E-5;
    if (rtDW.v < 0.0) {
      rtDW.v = 0.0;
    }
  } else {
    rtDW.v = 0.3;
  }

  if (sqrt(rtU.dist2goal * rtU.dist2goal) <= 0.0) {
    rtDW.v = 0.0;
  }

  /* MATLAB Function: '<S3>/MATLAB Function' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Constant: '<S1>/Constant7'
   *  Inport: '<Root>/R'
   *  Inport: '<Root>/alpha'
   */
  if (rtU.alpha < 0.0) {
    rtb_deltaL = rt_atan2d_snf(0.57, rtU.R + 0.185) * 57.295779513082323;
    rtb_deltaR = rt_atan2d_snf(0.57, rtU.R - 0.185) * 57.295779513082323;
    rtb_R_L = rtU.R + 0.185;
    rtb_R_R = rtU.R - 0.185;
  } else if (rtU.alpha > 0.0) {
    rtb_deltaL = -(rt_atan2d_snf(0.57, rtU.R - 0.185) * 57.295779513082323);
    rtb_deltaR = -(rt_atan2d_snf(0.57, rtU.R + 0.185) * 57.295779513082323);
    rtb_R_L = rtU.R - 0.185;
    rtb_R_R = rtU.R + 0.185;
  } else {
    rtb_deltaL = 0.0;
    rtb_deltaR = 0.0;
    rtb_R_L = rtU.R;
    rtb_R_R = rtU.R;
  }

  /* MATLAB Function: '<S3>/MATLAB Function1' incorporates:
   *  Constant: '<S1>/Constant7'
   *  Constant: '<S3>/Constant2'
   *  Gain: '<S3>/Gain4'
   *  Gain: '<S3>/Gain5'
   *  Inport: '<Root>/R'
   *  MATLAB Function: '<S3>/MATLAB Function'
   *  MATLAB Function: '<S3>/MATLAB Function2'
   *  SignalConversion generated from: '<S58>/ SFunction '
   */
  y[0] = fabs(rtb_deltaL);
  y[1] = fabs(-rtb_deltaL);
  y[2] = fabs(rtb_deltaR);
  y[3] = fabs(-rtb_deltaR);
  b_y = false;
  i = 0;
  exitg1 = false;
  while ((!exitg1) && (i < 4)) {
    if (y[i] > 0.0) {
      b_y = true;
      exitg1 = true;
    } else {
      i++;
    }
  }

  if (b_y) {
    rtb_wheel_speed_LF = sqrt(rtb_R_L * rtb_R_L + 0.32489999999999997) * (rtDW.v
      / 0.1) / rtU.R;
    rtb_R_L = rtDW.v * rtb_R_L / (rtU.R * 0.1);
    rtb_wheel_speed_RF = sqrt(rtb_R_R * rtb_R_R + 0.32489999999999997) * (rtDW.v
      / 0.1) / rtU.R;
    rtb_R_R = rtDW.v * rtb_R_R / (rtU.R * 0.1);
  } else {
    rtb_wheel_speed_LF = rtDW.v / 0.1;
    rtb_R_L = rtb_wheel_speed_LF;
    rtb_wheel_speed_RF = rtb_wheel_speed_LF;
    rtb_R_R = rtb_wheel_speed_LF;
  }

  /* End of MATLAB Function: '<S3>/MATLAB Function1' */

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/actspeed'
   */
  _mm_storeu_pd(&rtb_Sum_l[0], _mm_sub_pd(_mm_set_pd(rtb_R_L, rtb_wheel_speed_LF),
    _mm_loadu_pd(&rtU.actspeed[0])));
  _mm_storeu_pd(&rtb_Sum_l[2], _mm_sub_pd(_mm_set_pd(rtb_wheel_speed_RF,
    rtb_wheel_speed_LF), _mm_loadu_pd(&rtU.actspeed[2])));
  _mm_storeu_pd(&rtb_Sum_l[4], _mm_sub_pd(_mm_set_pd(rtb_wheel_speed_RF, rtb_R_R),
    _mm_loadu_pd(&rtU.actspeed[4])));

  /* Outport: '<Root>/angerror' incorporates:
   *  Gain: '<S3>/Gain4'
   *  Gain: '<S3>/Gain5'
   *  Inport: '<Root>/actang'
   *  Sum: '<S1>/Sum1'
   */
  _mm_storeu_pd(&rtY.angerror[0], _mm_sub_pd(_mm_set_pd(-rtb_deltaL, rtb_deltaL),
    _mm_loadu_pd(&rtU.actang[0])));
  _mm_storeu_pd(&rtY.angerror[2], _mm_sub_pd(_mm_set_pd(-rtb_deltaR, rtb_deltaR),
    _mm_loadu_pd(&rtU.actang[2])));

  /* Outport: '<Root>/desspeed' */
  rtY.desspeed[0] = rtb_wheel_speed_LF;
  rtY.desspeed[1] = rtb_R_L;
  rtY.desspeed[2] = rtb_wheel_speed_LF;
  rtY.desspeed[3] = rtb_wheel_speed_RF;
  rtY.desspeed[4] = rtb_R_R;
  rtY.desspeed[5] = rtb_wheel_speed_RF;

  /* Outport: '<Root>/desang' incorporates:
   *  Gain: '<S3>/Gain4'
   *  Gain: '<S3>/Gain5'
   */
  rtY.desang[0] = rtb_deltaL;
  rtY.desang[1] = -rtb_deltaL;
  rtY.desang[2] = rtb_deltaR;
  rtY.desang[3] = -rtb_deltaR;
  for (i = 0; i <= 4; i += 2) {
    /* Gain: '<S31>/Derivative Gain' */
    tmp = _mm_loadu_pd(&rtb_Sum_l[i]);

    /* Sum: '<S33>/SumD' incorporates:
     *  DiscreteIntegrator: '<S33>/Filter'
     *  Gain: '<S31>/Derivative Gain'
     */
    tmp_0 = _mm_loadu_pd(&rtDW.Filter_DSTATE[i]);

    /* Gain: '<S41>/Filter Coefficient' incorporates:
     *  Gain: '<S31>/Derivative Gain'
     *  Sum: '<S33>/SumD'
     */
    tmp_1 = _mm_mul_pd(_mm_sub_pd(_mm_mul_pd(_mm_set1_pd(0.198895104349163), tmp),
      tmp_0), _mm_set1_pd(8689.11352061538));

    /* Sum: '<S47>/Sum' incorporates:
     *  DiscreteIntegrator: '<S38>/Integrator'
     *  Gain: '<S31>/Derivative Gain'
     */
    tmp_2 = _mm_loadu_pd(&rtDW.Integrator_DSTATE[i]);

    /* Update for UnitDelay: '<S1>/Unit Delay' incorporates:
     *  Gain: '<S31>/Derivative Gain'
     *  Gain: '<S41>/Filter Coefficient'
     *  Gain: '<S43>/Proportional Gain'
     *  Sum: '<S47>/Sum'
     */
    _mm_storeu_pd(&rtDW.UnitDelay_DSTATE[i], _mm_add_pd(_mm_add_pd(_mm_mul_pd
      (_mm_set1_pd(54.7409706092039), tmp), tmp_2), tmp_1));

    /* Update for DiscreteIntegrator: '<S33>/Filter' incorporates:
     *  Gain: '<S31>/Derivative Gain'
     *  Gain: '<S41>/Filter Coefficient'
     *  Sum: '<S33>/SumD'
     */
    _mm_storeu_pd(&rtDW.Filter_DSTATE[i], _mm_add_pd(_mm_mul_pd(_mm_set1_pd
      (0.0001), tmp_1), tmp_0));

    /* Update for DiscreteIntegrator: '<S38>/Integrator' incorporates:
     *  Gain: '<S31>/Derivative Gain'
     *  Gain: '<S35>/Integral Gain'
     *  Sum: '<S47>/Sum'
     */
    _mm_storeu_pd(&rtDW.Integrator_DSTATE[i], _mm_add_pd(_mm_mul_pd(_mm_set1_pd
      (0.00122680114575866), tmp), tmp_2));
  }
}

/* Model initialize function */
void control_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
