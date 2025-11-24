/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control.c
 *
 * Code generated for Simulink model 'control'.
 *
 * Model version                  : 3.7
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Mon Nov 24 15:58:20 2025
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control.h"
#include <math.h>
#include "rtwtypes.h"
#include "math.h"

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
static real_T rtGetNaN(void);
static real32_T rtGetNaNF(void);
extern real_T rtInf;
extern real_T rtMinusInf;
extern real_T rtNaN;
extern real32_T rtInfF;
extern real32_T rtMinusInfF;
extern real32_T rtNaNF;
static boolean_T rtIsInf(real_T value);
static boolean_T rtIsInfF(real32_T value);
static boolean_T rtIsNaN(real_T value);
static boolean_T rtIsNaNF(real32_T value);
real_T rtNaN = -(real_T)NAN;
real_T rtInf = (real_T)INFINITY;
real_T rtMinusInf = -(real_T)INFINITY;
real32_T rtNaNF = -(real32_T)NAN;
real32_T rtInfF = (real32_T)INFINITY;
real32_T rtMinusInfF = -(real32_T)INFINITY;

/* Return rtNaN needed by the generated code. */
static real_T rtGetNaN(void)
{
  return rtNaN;
}

/* Return rtNaNF needed by the generated code. */
static real32_T rtGetNaNF(void)
{
  return rtNaNF;
}

/* Test if value is infinite */
static boolean_T rtIsInf(real_T value)
{
  return (boolean_T)isinf(value);
}

/* Test if single-precision value is infinite */
static boolean_T rtIsInfF(real32_T value)
{
  return (boolean_T)isinf(value);
}

/* Test if value is not a number */
static boolean_T rtIsNaN(real_T value)
{
  return (boolean_T)(isnan(value) != 0);
}

/* Test if single-precision value is not a number */
static boolean_T rtIsNaNF(real32_T value)
{
  return (boolean_T)(isnan(value) != 0);
}

/* Model step function */
void control_step(void)
{
  real_T rtb_Sum_l[6];
  real_T y[4];
  real_T rtb_R_L;
  real_T rtb_R_R;
  real_T rtb_deltaL;
  real_T rtb_deltaL_tmp;
  real_T rtb_deltaR;
  real_T rtb_wheel_speed_LF;
  real_T rtb_wheel_speed_LF_tmp;
  real_T rtb_wheel_speed_RF;
  int16_T i;
  boolean_T b_y;
  boolean_T exitg1;

  /* MATLAB Function: '<S1>/MATLAB Function2' incorporates:
   *  Constant: '<S1>/Constant4'
   *  Constant: '<S2>/Constant'
   *  Inport: '<Root>/dist2goal'
   *  RelationalOperator: '<S2>/Compare'
   */
  if (!rtDW.v_not_empty) {
    rtDW.v = 0.3;
    rtDW.v_not_empty = true;
  }

  if (rtU.dist2goal <= 1.0) {
    rtDW.v -= 0.0005;
    if (rtDW.v < 0.0) {
      rtDW.v = 0.0;
    }
  } else {
    rtDW.v = 0.3;
  }

  if (rtU.dist2goal <= 0.5) {
    rtDW.v = 0.0;
  }

  /* MATLAB Function: '<S1>/MATLAB Function' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Inport: '<Root>/dist2goal'
   *  Inport: '<Root>/steerang'
   */
  rtb_wheel_speed_RF = 1.0471975511965976 * rtU.dist2goal;
  if (rtU.steerang < 0.0) {
    rtb_R_L = rtU.dist2goal + 0.185;
    rtb_R_R = rtU.dist2goal - 0.185;
  } else if (rtU.steerang > 0.0) {
    rtb_R_L = rtU.dist2goal - 0.185;
    rtb_R_R = rtU.dist2goal + 0.185;
  } else {
    rtb_R_L = rtU.dist2goal;
    rtb_R_R = rtU.dist2goal;
  }

  rtb_deltaR = sin(rtU.steerang);
  rtb_wheel_speed_LF = 2.0 * rtb_wheel_speed_RF;
  rtb_deltaL_tmp = rtb_wheel_speed_LF * rtb_deltaR;
  rtb_wheel_speed_LF *= cos(rtU.steerang);
  rtb_deltaR *= 0.37;
  rtb_deltaL = atan(rtb_deltaL_tmp / (rtb_wheel_speed_LF + rtb_deltaR));
  rtb_deltaR = atan(rtb_deltaL_tmp / (rtb_wheel_speed_LF - rtb_deltaR));

  /* MATLAB Function: '<S1>/MATLAB Function1' incorporates:
   *  Constant: '<S1>/Constant2'
   *  Gain: '<S1>/Gain4'
   *  Gain: '<S1>/Gain5'
   *  Inport: '<Root>/dist2goal'
   *  MATLAB Function: '<S1>/MATLAB Function'
   *  MATLAB Function: '<S1>/MATLAB Function2'
   *  SignalConversion generated from: '<S4>/ SFunction '
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
    rtb_wheel_speed_RF *= rtb_wheel_speed_RF;
    rtb_wheel_speed_LF_tmp = rtDW.v / 0.1;
    rtb_wheel_speed_LF = sqrt(rtb_R_L * rtb_R_L + rtb_wheel_speed_RF) *
      rtb_wheel_speed_LF_tmp / rtU.dist2goal;
    rtb_deltaL_tmp = rtU.dist2goal * 0.1;
    rtb_R_L = rtDW.v * rtb_R_L / rtb_deltaL_tmp;
    rtb_wheel_speed_RF = sqrt(rtb_R_R * rtb_R_R + rtb_wheel_speed_RF) *
      rtb_wheel_speed_LF_tmp / rtU.dist2goal;
    rtb_R_R = rtDW.v * rtb_R_R / rtb_deltaL_tmp;
  } else {
    rtb_wheel_speed_LF = rtDW.v / 0.1;
    rtb_R_L = rtb_wheel_speed_LF;
    rtb_wheel_speed_RF = rtb_wheel_speed_LF;
    rtb_R_R = rtb_wheel_speed_LF;
  }

  /* End of MATLAB Function: '<S1>/MATLAB Function1' */

  /* Outport: '<Root>/desspeed' */
  rtY.desspeed[0] = rtb_wheel_speed_LF;
  rtY.desspeed[1] = rtb_R_L;
  rtY.desspeed[2] = rtb_wheel_speed_LF;
  rtY.desspeed[3] = rtb_wheel_speed_RF;
  rtY.desspeed[4] = rtb_R_R;
  rtY.desspeed[5] = rtb_wheel_speed_RF;

  /* Outport: '<Root>/controlb' */
  for (i = 0; i < 6; i++) {
    /* UnitDelay: '<S1>/Unit Delay' */
    rtb_deltaL_tmp = rtDW.UnitDelay_DSTATE[i];

    /* Saturate: '<S1>/Saturation' */
    if (rtb_deltaL_tmp > 24.0) {
      rtY.controlb[i] = 24.0;
    } else if (rtb_deltaL_tmp < -24.0) {
      rtY.controlb[i] = -24.0;
    } else {
      rtY.controlb[i] = rtb_deltaL_tmp;
    }

    /* End of Saturate: '<S1>/Saturation' */
  }

  /* End of Outport: '<Root>/controlb' */

  /* Outport: '<Root>/desang' incorporates:
   *  Gain: '<S1>/Gain4'
   *  Gain: '<S1>/Gain5'
   */
  rtY.desang[0] = rtb_deltaL;
  rtY.desang[1] = -rtb_deltaL;
  rtY.desang[2] = rtb_deltaR;
  rtY.desang[3] = -rtb_deltaR;

  /* DiscretePulseGenerator: '<S1>/Pulse Generator' */
  i = ((rtDW.clockTickCounter < 1L) && (rtDW.clockTickCounter >= 0L));
  if (rtDW.clockTickCounter >= 9L) {
    rtDW.clockTickCounter = 0L;
  } else {
    rtDW.clockTickCounter++;
  }

  /* End of DiscretePulseGenerator: '<S1>/Pulse Generator' */

  /* Saturate: '<S1>/Saturation2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  if (rtDW.UnitDelay1_DSTATE[0] > 1.0) {
    rtb_deltaL_tmp = 1.0;
  } else if (rtDW.UnitDelay1_DSTATE[0] < -1.0) {
    rtb_deltaL_tmp = -1.0;
  } else {
    rtb_deltaL_tmp = rtDW.UnitDelay1_DSTATE[0];
  }

  /* Outport: '<Root>/pwnenable' incorporates:
   *  Abs: '<S1>/Abs'
   *  Gain: '<S1>/Gain2'
   *  Product: '<S1>/Product'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwnenable[0] = (real_T)i * fabs(rtb_deltaL_tmp) * 5.0;

  /* Signum: '<S1>/Sign' incorporates:
   *  Gain: '<S1>/Gain'
   *  Saturate: '<S1>/Saturation1'
   */
  if (rtIsNaN(-rtb_deltaL_tmp)) {
    rtb_deltaL_tmp = (rtNaN);
  } else if (-rtb_deltaL_tmp < 0.0) {
    /* Saturate: '<S1>/Saturation1' */
    rtb_deltaL_tmp = 0.0;
  } else {
    rtb_deltaL_tmp = (-rtb_deltaL_tmp > 0.0);
  }

  /* Outport: '<Root>/pwmrev' incorporates:
   *  Gain: '<S1>/Gain1'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwmrev[0] = 5.0 * rtb_deltaL_tmp;

  /* Saturate: '<S1>/Saturation2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  if (rtDW.UnitDelay1_DSTATE[1] > 1.0) {
    rtb_deltaL_tmp = 1.0;
  } else if (rtDW.UnitDelay1_DSTATE[1] < -1.0) {
    rtb_deltaL_tmp = -1.0;
  } else {
    rtb_deltaL_tmp = rtDW.UnitDelay1_DSTATE[1];
  }

  /* Outport: '<Root>/pwnenable' incorporates:
   *  Abs: '<S1>/Abs'
   *  Gain: '<S1>/Gain2'
   *  Product: '<S1>/Product'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwnenable[1] = (real_T)i * fabs(rtb_deltaL_tmp) * 5.0;

  /* Signum: '<S1>/Sign' incorporates:
   *  Gain: '<S1>/Gain'
   *  Saturate: '<S1>/Saturation1'
   */
  if (rtIsNaN(-rtb_deltaL_tmp)) {
    rtb_deltaL_tmp = (rtNaN);
  } else if (-rtb_deltaL_tmp < 0.0) {
    /* Saturate: '<S1>/Saturation1' */
    rtb_deltaL_tmp = 0.0;
  } else {
    rtb_deltaL_tmp = (-rtb_deltaL_tmp > 0.0);
  }

  /* Outport: '<Root>/pwmrev' incorporates:
   *  Gain: '<S1>/Gain1'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwmrev[1] = 5.0 * rtb_deltaL_tmp;

  /* Saturate: '<S1>/Saturation2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  if (rtDW.UnitDelay1_DSTATE[2] > 1.0) {
    rtb_deltaL_tmp = 1.0;
  } else if (rtDW.UnitDelay1_DSTATE[2] < -1.0) {
    rtb_deltaL_tmp = -1.0;
  } else {
    rtb_deltaL_tmp = rtDW.UnitDelay1_DSTATE[2];
  }

  /* Outport: '<Root>/pwnenable' incorporates:
   *  Abs: '<S1>/Abs'
   *  Gain: '<S1>/Gain2'
   *  Product: '<S1>/Product'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwnenable[2] = (real_T)i * fabs(rtb_deltaL_tmp) * 5.0;

  /* Signum: '<S1>/Sign' incorporates:
   *  Gain: '<S1>/Gain'
   *  Saturate: '<S1>/Saturation1'
   */
  if (rtIsNaN(-rtb_deltaL_tmp)) {
    rtb_deltaL_tmp = (rtNaN);
  } else if (-rtb_deltaL_tmp < 0.0) {
    /* Saturate: '<S1>/Saturation1' */
    rtb_deltaL_tmp = 0.0;
  } else {
    rtb_deltaL_tmp = (-rtb_deltaL_tmp > 0.0);
  }

  /* Outport: '<Root>/pwmrev' incorporates:
   *  Gain: '<S1>/Gain1'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwmrev[2] = 5.0 * rtb_deltaL_tmp;

  /* Saturate: '<S1>/Saturation2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  if (rtDW.UnitDelay1_DSTATE[3] > 1.0) {
    rtb_deltaL_tmp = 1.0;
  } else if (rtDW.UnitDelay1_DSTATE[3] < -1.0) {
    rtb_deltaL_tmp = -1.0;
  } else {
    rtb_deltaL_tmp = rtDW.UnitDelay1_DSTATE[3];
  }

  /* Outport: '<Root>/pwnenable' incorporates:
   *  Abs: '<S1>/Abs'
   *  Gain: '<S1>/Gain2'
   *  Product: '<S1>/Product'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwnenable[3] = (real_T)i * fabs(rtb_deltaL_tmp) * 5.0;

  /* Signum: '<S1>/Sign' incorporates:
   *  Gain: '<S1>/Gain'
   *  Saturate: '<S1>/Saturation1'
   */
  if (rtIsNaN(-rtb_deltaL_tmp)) {
    rtb_deltaL_tmp = (rtNaN);
  } else if (-rtb_deltaL_tmp < 0.0) {
    /* Saturate: '<S1>/Saturation1' */
    rtb_deltaL_tmp = 0.0;
  } else {
    rtb_deltaL_tmp = (-rtb_deltaL_tmp > 0.0);
  }

  /* Outport: '<Root>/pwmrev' incorporates:
   *  Gain: '<S1>/Gain1'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.pwmrev[3] = 5.0 * rtb_deltaL_tmp;

  /* Sum: '<S1>/Sum' incorporates:
   *  Inport: '<Root>/actspeed'
   */
  rtb_Sum_l[0] = rtb_wheel_speed_LF - rtU.actspeed[0];
  rtb_Sum_l[1] = rtb_R_L - rtU.actspeed[1];
  rtb_Sum_l[2] = rtb_wheel_speed_LF - rtU.actspeed[2];
  rtb_Sum_l[3] = rtb_wheel_speed_RF - rtU.actspeed[3];
  rtb_Sum_l[4] = rtb_R_R - rtU.actspeed[4];
  rtb_Sum_l[5] = rtb_wheel_speed_RF - rtU.actspeed[5];

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  Gain: '<S1>/Gain4'
   *  Gain: '<S1>/Gain5'
   *  Inport: '<Root>/actang'
   *  Sum: '<S1>/Sum1'
   */
  rtDW.UnitDelay1_DSTATE[0] = rtb_deltaL - rtU.actang[0];
  rtDW.UnitDelay1_DSTATE[1] = -rtb_deltaL - rtU.actang[1];
  rtDW.UnitDelay1_DSTATE[2] = rtb_deltaR - rtU.actang[2];
  rtDW.UnitDelay1_DSTATE[3] = -rtb_deltaR - rtU.actang[3];
  for (i = 0; i < 6; i++) {
    /* Gain: '<S46>/Proportional Gain' incorporates:
     *  Sum: '<S1>/Sum'
     */
    rtb_R_L = 5.0 * rtb_Sum_l[i];

    /* Sum: '<S51>/Sum Fdbk' incorporates:
     *  Gain: '<S46>/Proportional Gain'
     */
    rtb_R_R = rtDW.Integrator_DSTATE[i];
    rtb_deltaL = rtb_R_L + rtb_R_R;

    /* DiscreteIntegrator: '<S41>/Integrator' incorporates:
     *  Gain: '<S33>/Kb'
     *  Gain: '<S38>/Integral Gain'
     *  Sum: '<S33>/SumI2'
     *  Sum: '<S33>/SumI4'
     *  Sum: '<S51>/Sum Fdbk'
     */
    rtb_deltaL = ((rtb_deltaL - rtb_deltaL) * 2.0 + rtb_R_L) * 0.001 + rtb_R_R;

    /* Update for UnitDelay: '<S1>/Unit Delay' incorporates:
     *  DiscreteIntegrator: '<S41>/Integrator'
     *  Gain: '<S46>/Proportional Gain'
     *  Sum: '<S50>/Sum'
     */
    rtDW.UnitDelay_DSTATE[i] = rtb_R_L + rtb_deltaL;

    /* Update for DiscreteIntegrator: '<S41>/Integrator' */
    rtDW.Integrator_DSTATE[i] = rtb_deltaL;
  }
}

/* Model initialize function */
void control_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */