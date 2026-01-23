/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control0.c
 *
 * Code generated for Simulink model 'control0'.
 *
 * Model version                  : 1.6
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Fri Jan 16 13:38:57 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control0.h"
#include "rtwtypes.h"
#include <math.h>
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
static void ramp(real_T rtu_v_des, real_T rtu_v_out_in, real_T rtu_acceleration,
                 real_T rtu_decelleration, real_T *rty_v_out, DW_ramp *localDW);
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

/*
 * Output and update for atomic system:
 *    '<S1>/ramp'
 *    '<S1>/ramp1'
 *    '<S1>/ramp2'
 *    '<S1>/ramp3'
 */
static void ramp(real_T rtu_v_des, real_T rtu_v_out_in, real_T rtu_acceleration,
                 real_T rtu_decelleration, real_T *rty_v_out, DW_ramp *localDW)
{
  real_T tmp;
  tmp = 0.001 * rtu_acceleration;
  if (fabs(localDW->v - rtu_v_des) < tmp) {
    *rty_v_out = localDW->v;
  } else {
    if (rtu_v_out_in < rtu_v_des) {
      localDW->v += tmp;
      if (localDW->v > rtu_v_des) {
        localDW->v = rtu_v_des;
      }
    } else {
      localDW->v -= rtu_decelleration * 0.001;
      if (localDW->v < 0.0) {
        localDW->v = 0.0;
      }
    }

    *rty_v_out = localDW->v;
  }
}

/* Model step function */
void control0_step(void)
{
  real_T rtb_Integrator;
  real_T rtb_Integrator_g;
  real_T rtb_Integrator_l;
  real_T rtb_Integrator_n;
  real_T rtb_UnitDelay3;
  real_T rtb_v_out;
  real_T rtb_v_out_a;
  real_T rtb_v_out_m;
  real_T u0;
  int16_T rtb_PulseGenerator1;

  /* Outport: '<Root>/controlJaw1' incorporates:
   *  UnitDelay: '<S1>/Unit Delay8'
   */
  rtY.controlJaw1 = rtDW.UnitDelay8_DSTATE;

  /* Outport: '<Root>/controlWristRotation' incorporates:
   *  UnitDelay: '<S1>/Unit Delay6'
   */
  rtY.controlWristRotation = rtDW.UnitDelay6_DSTATE;

  /* MATLAB Function: '<S1>/ramp3' incorporates:
   *  Constant: '<S1>/Constant6'
   *  Constant: '<S1>/Constant7'
   *  Constant: '<S1>/Constant8'
   *  UnitDelay: '<S1>/Unit Delay9'
   */
  ramp(0.0, rtDW.UnitDelay9_DSTATE, 0.8, 2.0, &rtb_v_out, &rtDW.sf_ramp3);

  /* Sum: '<S1>/Sum6' incorporates:
   *  Inport: '<Root>/jawActualPosition'
   *  UnitDelay: '<S1>/Unit Delay9'
   */
  rtb_Integrator = rtDW.UnitDelay9_DSTATE - rtU.jawActualPosition;

  /* Outport: '<Root>/controlGripperPitch' incorporates:
   *  UnitDelay: '<S1>/Unit Delay4'
   */
  rtY.controlGripperPitch = rtDW.UnitDelay4_DSTATE;

  /* Outport: '<Root>/controlBase' incorporates:
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  rtY.controlBase = rtDW.UnitDelay1_DSTATE;

  /* MATLAB Function: '<S1>/ramp2' incorporates:
   *  Constant: '<S1>/Constant4'
   *  Constant: '<S1>/Constant5'
   *  Constant: '<S1>/Constant9'
   *  UnitDelay: '<S1>/Unit Delay7'
   */
  ramp(0.0, rtDW.UnitDelay7_DSTATE, 0.8, 2.0, &rtb_v_out_m, &rtDW.sf_ramp2);

  /* Sum: '<S1>/Sum5' incorporates:
   *  Inport: '<Root>/gripperRotationActualPosition'
   *  UnitDelay: '<S1>/Unit Delay7'
   */
  rtb_Integrator_l = rtDW.UnitDelay7_DSTATE - rtU.gripperRotationActualPosition;

  /* DiscretePulseGenerator: '<S1>/Pulse Generator' */
  rtb_UnitDelay3 = ((rtDW.clockTickCounter < 1L) && (rtDW.clockTickCounter >= 0L));
  if (rtDW.clockTickCounter >= 1L) {
    rtDW.clockTickCounter = 0L;
  } else {
    rtDW.clockTickCounter++;
  }

  /* End of DiscretePulseGenerator: '<S1>/Pulse Generator' */

  /* Outport: '<Root>/stepperTopENA' incorporates:
   *  Abs: '<S1>/Abs'
   *  Constant: '<S2>/Constant'
   *  Gain: '<S1>/Gain3'
   *  Product: '<S1>/Product'
   *  RelationalOperator: '<S2>/Compare'
   *  UnitDelay: '<S1>/Unit Delay'
   */
  rtY.stepperTopENA = (real_T)(fabs(rtDW.UnitDelay_DSTATE) > 0.0002) *
    rtb_UnitDelay3 * 5.0;

  /* Signum: '<S1>/Sign1' incorporates:
   *  UnitDelay: '<S1>/Unit Delay'
   */
  if (rtIsNaN(rtDW.UnitDelay_DSTATE)) {
    /* Gain: '<S1>/Gain1' */
    u0 = (rtNaN);
  } else if (rtDW.UnitDelay_DSTATE < 0.0) {
    /* Gain: '<S1>/Gain1' */
    u0 = 1.0;
  } else {
    /* Gain: '<S1>/Gain1' */
    u0 = -(real_T)(rtDW.UnitDelay_DSTATE > 0.0);
  }

  /* End of Signum: '<S1>/Sign1' */

  /* Saturate: '<S1>/Saturation1' */
  if (u0 < 0.0) {
    u0 = 0.0;
  }

  /* Outport: '<Root>/stepperTopREV' incorporates:
   *  Gain: '<S1>/Gain2'
   *  Saturate: '<S1>/Saturation1'
   */
  rtY.stepperTopREV = 5.0 * u0;

  /* MATLAB Function: '<S1>/ramp1' incorporates:
   *  Constant: '<S1>/Constant10'
   *  Constant: '<S1>/Constant2'
   *  Constant: '<S1>/Constant3'
   *  UnitDelay: '<S1>/Unit Delay5'
   */
  ramp(0.0, rtDW.UnitDelay5_DSTATE, 0.8, 2.0, &rtb_UnitDelay3, &rtDW.sf_ramp1);

  /* Sum: '<S1>/Sum4' incorporates:
   *  Inport: '<Root>/gripperPitchActualPosition'
   *  UnitDelay: '<S1>/Unit Delay5'
   */
  rtb_Integrator_n = rtDW.UnitDelay5_DSTATE - rtU.gripperPitchActualPosition;

  /* MATLAB Function: '<S1>/ramp' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S1>/Constant1'
   *  Constant: '<S1>/Constant11'
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  ramp(0.0, rtDW.UnitDelay2_DSTATE, 0.8, 2.0, &rtb_v_out_a, &rtDW.sf_ramp);

  /* Sum: '<S1>/Sum2' incorporates:
   *  Inport: '<Root>/baseActualPosition'
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  rtb_Integrator_g = rtDW.UnitDelay2_DSTATE - rtU.baseActualPosition;

  /* DiscretePulseGenerator: '<S1>/Pulse Generator1' */
  rtb_PulseGenerator1 = ((rtDW.clockTickCounter_h < 1L) &&
    (rtDW.clockTickCounter_h >= 0L));
  if (rtDW.clockTickCounter_h >= 1L) {
    rtDW.clockTickCounter_h = 0L;
  } else {
    rtDW.clockTickCounter_h++;
  }

  /* End of DiscretePulseGenerator: '<S1>/Pulse Generator1' */

  /* Outport: '<Root>/stepperBottomENA' incorporates:
   *  Abs: '<S1>/Abs1'
   *  Constant: '<S3>/Constant'
   *  Gain: '<S1>/Gain6'
   *  Product: '<S1>/Product1'
   *  RelationalOperator: '<S3>/Compare'
   *  UnitDelay: '<S1>/Unit Delay3'
   */
  rtY.stepperBottomENA = (real_T)((fabs(rtDW.UnitDelay3_DSTATE) > 0.0002) *
    rtb_PulseGenerator1) * 5.0;

  /* Signum: '<S1>/Sign2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay3'
   */
  if (rtIsNaN(rtDW.UnitDelay3_DSTATE)) {
    /* Gain: '<S1>/Gain4' */
    u0 = (rtNaN);
  } else if (rtDW.UnitDelay3_DSTATE < 0.0) {
    /* Gain: '<S1>/Gain4' */
    u0 = 1.0;
  } else {
    /* Gain: '<S1>/Gain4' */
    u0 = -(real_T)(rtDW.UnitDelay3_DSTATE > 0.0);
  }

  /* End of Signum: '<S1>/Sign2' */

  /* Saturate: '<S1>/Saturation2' */
  if (u0 < 0.0) {
    u0 = 0.0;
  }

  /* Outport: '<Root>/stepperBottomREV' incorporates:
   *  Gain: '<S1>/Gain5'
   *  Saturate: '<S1>/Saturation2'
   */
  rtY.stepperBottomREV = 5.0 * u0;

  /* Update for UnitDelay: '<S1>/Unit Delay8' incorporates:
   *  DiscreteIntegrator: '<S202>/Integrator'
   *  Gain: '<S207>/Proportional Gain'
   *  Sum: '<S211>/Sum'
   */
  rtDW.UnitDelay8_DSTATE = 5.0 * rtb_Integrator + rtDW.Integrator_DSTATE;

  /* Update for UnitDelay: '<S1>/Unit Delay6' incorporates:
   *  DiscreteIntegrator: '<S150>/Integrator'
   *  Gain: '<S155>/Proportional Gain'
   *  Sum: '<S159>/Sum'
   */
  rtDW.UnitDelay6_DSTATE = 5.0 * rtb_Integrator_l + rtDW.Integrator_DSTATE_k;

  /* Update for UnitDelay: '<S1>/Unit Delay9' */
  rtDW.UnitDelay9_DSTATE = rtb_v_out;

  /* Update for DiscreteIntegrator: '<S202>/Integrator' incorporates:
   *  Gain: '<S199>/Integral Gain'
   */
  rtDW.Integrator_DSTATE += 0.125 * rtb_Integrator * 0.001;

  /* Update for UnitDelay: '<S1>/Unit Delay4' incorporates:
   *  DiscreteIntegrator: '<S98>/Integrator'
   *  Gain: '<S103>/Proportional Gain'
   *  Sum: '<S107>/Sum'
   */
  rtDW.UnitDelay4_DSTATE = 2.0 * rtb_Integrator_n + rtDW.Integrator_DSTATE_l;

  /* Update for UnitDelay: '<S1>/Unit Delay1' incorporates:
   *  DiscreteIntegrator: '<S46>/Integrator'
   *  Gain: '<S51>/Proportional Gain'
   *  Sum: '<S55>/Sum'
   */
  rtDW.UnitDelay1_DSTATE = 5.0 * rtb_Integrator_g + rtDW.Integrator_DSTATE_b;

  /* Update for UnitDelay: '<S1>/Unit Delay7' */
  rtDW.UnitDelay7_DSTATE = rtb_v_out_m;

  /* Update for DiscreteIntegrator: '<S150>/Integrator' incorporates:
   *  Gain: '<S147>/Integral Gain'
   */
  rtDW.Integrator_DSTATE_k += 0.125 * rtb_Integrator_l * 0.001;

  /* Update for UnitDelay: '<S1>/Unit Delay' incorporates:
   *  Constant: '<S1>/Constant12'
   *  Inport: '<Root>/stepperTopActualPosition'
   *  Sum: '<S1>/Sum1'
   */
  rtDW.UnitDelay_DSTATE = 0.0 - rtU.stepperTopActualPosition;

  /* Update for UnitDelay: '<S1>/Unit Delay5' */
  rtDW.UnitDelay5_DSTATE = rtb_UnitDelay3;

  /* Update for DiscreteIntegrator: '<S98>/Integrator' incorporates:
   *  Gain: '<S95>/Integral Gain'
   */
  rtDW.Integrator_DSTATE_l += 0.05 * rtb_Integrator_n * 0.001;

  /* Update for UnitDelay: '<S1>/Unit Delay2' */
  rtDW.UnitDelay2_DSTATE = rtb_v_out_a;

  /* Update for DiscreteIntegrator: '<S46>/Integrator' incorporates:
   *  Gain: '<S43>/Integral Gain'
   */
  rtDW.Integrator_DSTATE_b += 0.03 * rtb_Integrator_g * 0.001;

  /* Update for UnitDelay: '<S1>/Unit Delay3' incorporates:
   *  Constant: '<S1>/Constant13'
   *  Inport: '<Root>/stepperBottomActualPosition'
   *  Sum: '<S1>/Sum3'
   */
  rtDW.UnitDelay3_DSTATE = 0.0 - rtU.stepperBottomActualPosition;
}

/* Model initialize function */
void control0_initialize(void)
{
  /* ConstCode for Outport: '<Root>/pointReached' */
  rtY.pointReached = 0.0;

  /* ConstCode for Outport: '<Root>/pointInRange' */
  rtY.pointInRange = 0.0;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */