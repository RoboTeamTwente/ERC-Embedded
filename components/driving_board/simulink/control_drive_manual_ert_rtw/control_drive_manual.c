/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_drive_manual.c
 *
 * Code generated for Simulink model 'control_drive_manual'.
 *
 * Model version                  : 1.13
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Mon May 18 12:32:25 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control_drive_manual.h"
#include <math.h>
#include "rtwtypes.h"

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
extern real_T rt_roundd_snf(real_T u);
real_T rt_roundd_snf(real_T u)
{
  real_T y;
  if (fabs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = floor(u + 0.5);
    } else if (u > -0.5) {
      y = u * 0.0;
    } else {
      y = ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

/* Model step function */
void control_drive_manual_step(void)
{
  real_T rtb_R;
  real_T rtb_deltaL;
  real_T rtb_deltaR;
  real_T rtb_wheel_speed_LF;
  real_T rtb_wheel_speed_LF_tmp;
  real_T rtb_wheel_speed_LM;
  real_T rtb_wheel_speed_LM_tmp;
  real_T rtb_wheel_speed_RM;

  /* MATLAB Function: '<S1>/MATLAB Function' incorporates:
   *  Constant: '<S1>/integer maximum1'
   *  Constant: '<S1>/smallest turn radius'
   *  Inport: '<Root>/controllerSteering'
   */
  if (rtU.controllerSteering != 0.0) {
    rtb_R = 0.0;
  } else {
    rtb_R = 3.0 / (rtU.controllerSteering / 2.147483647E+9);
  }

  /* End of MATLAB Function: '<S1>/MATLAB Function' */

  /* MATLAB Function: '<S1>/getting the steering angles and turing radii' incorporates:
   *  Constant: '<S1>/Constant5'
   *  Constant: '<S1>/Constant6'
   */
  if (rtb_R < 0.0) {
    rtb_wheel_speed_LM = rtb_R + 0.37;
    rtb_wheel_speed_RM = rtb_R - 0.37;
    rtb_deltaL = atan(0.57 / (rtb_R + 0.37));
    rtb_deltaR = atan(0.57 / (rtb_R - 0.37));
  } else if (rtb_R > 0.0) {
    rtb_wheel_speed_LM = rtb_R - 0.37;
    rtb_wheel_speed_RM = rtb_R + 0.37;
    rtb_deltaL = -atan(0.57 / (rtb_R - 0.37));
    rtb_deltaR = -atan(0.57 / (rtb_R + 0.37));
  } else {
    rtb_deltaL = 0.0;
    rtb_deltaR = 0.0;
    rtb_wheel_speed_LM = rtb_R;
    rtb_wheel_speed_RM = rtb_R;
  }

  /* MATLAB Function: '<S1>/getting the wheel speeds from the turning radii' incorporates:
   *  Constant: '<S1>/Constant2'
   *  MATLAB Function: '<S1>/getting the steering angles and turing radii'
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  if (rtb_R == 0.0) {
    rtb_wheel_speed_LF = rtDW.UnitDelay2_DSTATE / 0.1;
    rtb_wheel_speed_LM = rtb_wheel_speed_LF;
    rtb_R = rtb_wheel_speed_LF;
    rtb_wheel_speed_RM = rtb_wheel_speed_LF;
  } else {
    rtb_wheel_speed_LF_tmp = rtDW.UnitDelay2_DSTATE / 0.1;
    rtb_wheel_speed_LF = sqrt(rtb_wheel_speed_LM * rtb_wheel_speed_LM +
      0.081224999999999992) * rtb_wheel_speed_LF_tmp / rtb_R;
    rtb_wheel_speed_LM_tmp = rtb_R * 0.1;
    rtb_wheel_speed_LM = rtDW.UnitDelay2_DSTATE * rtb_wheel_speed_LM /
      rtb_wheel_speed_LM_tmp;
    rtb_R = sqrt(rtb_wheel_speed_RM * rtb_wheel_speed_RM + 0.081224999999999992)
      * rtb_wheel_speed_LF_tmp / rtb_R;
    rtb_wheel_speed_RM = rtDW.UnitDelay2_DSTATE * rtb_wheel_speed_RM /
      rtb_wheel_speed_LM_tmp;
  }

  /* End of MATLAB Function: '<S1>/getting the wheel speeds from the turning radii' */

  /* Gain: '<S1>/pole pairs' incorporates:
   *  Gain: '<S1>/gear box'
   *  Gain: '<S1>/pole pairs2'
   *  Gain: '<S1>/rad//s to rpm'
   */
  rtb_wheel_speed_LF = 9.5492965855137211 * rtb_wheel_speed_LF * 10.0 * 21.0;

  /* Outport: '<Root>/controlLF' incorporates:
   *  Gain: '<S1>/pole pairs'
   */
  rtY.controlLF = rtb_wheel_speed_LF;

  /* Outport: '<Root>/controlLM' incorporates:
   *  Gain: '<S1>/gear box1'
   *  Gain: '<S1>/pole pairs1'
   *  Gain: '<S1>/rad//s to rpm1'
   */
  rtY.controlLM = 9.5492965855137211 * rtb_wheel_speed_LM * 10.0 * 21.0;

  /* Outport: '<Root>/controlLB' */
  rtY.controlLB = rtb_wheel_speed_LF;

  /* Gain: '<S1>/pole pairs3' incorporates:
   *  Gain: '<S1>/gear box3'
   *  Gain: '<S1>/pole pairs5'
   *  Gain: '<S1>/rad//s to rpm3'
   */
  rtb_wheel_speed_LM = 9.5492965855137211 * rtb_R * 10.0 * 21.0;

  /* Outport: '<Root>/controlRF' incorporates:
   *  Gain: '<S1>/pole pairs3'
   */
  rtY.controlRF = rtb_wheel_speed_LM;

  /* Outport: '<Root>/controlRM' incorporates:
   *  Gain: '<S1>/gear box4'
   *  Gain: '<S1>/pole pairs4'
   *  Gain: '<S1>/rad//s to rpm4'
   */
  rtY.controlRM = 9.5492965855137211 * rtb_wheel_speed_RM * 10.0 * 21.0;

  /* Outport: '<Root>/controlRB' */
  rtY.controlRB = rtb_wheel_speed_LM;

  /* Outport: '<Root>/stepperLFSteps' incorporates:
   *  Constant: '<S1>/Constant7'
   *  Gain: '<S1>/Gain'
   *  Product: '<S1>/Divide2'
   *  Rounding: '<S1>/Round1'
   */
  rtY.stepperLFSteps = rt_roundd_snf(57.295779513082323 * rtb_deltaL / 1.8);

  /* Outport: '<Root>/stepperLBSteps' incorporates:
   *  Constant: '<S1>/Constant1'
   *  Gain: '<S1>/Gain1'
   *  Gain: '<S1>/Gain4'
   *  Product: '<S1>/Divide1'
   *  Rounding: '<S1>/Round2'
   */
  rtY.stepperLBSteps = rt_roundd_snf(57.295779513082323 * -rtb_deltaL / 1.8);

  /* Outport: '<Root>/stepperRFSteps' incorporates:
   *  Constant: '<S1>/Constant3'
   *  Gain: '<S1>/Gain2'
   *  Product: '<S1>/Divide3'
   *  Rounding: '<S1>/Round3'
   */
  rtY.stepperRFSteps = rt_roundd_snf(57.295779513082323 * rtb_deltaR / 1.8);

  /* Outport: '<Root>/stepperRBSteps' incorporates:
   *  Constant: '<S1>/Constant4'
   *  Gain: '<S1>/Gain3'
   *  Gain: '<S1>/Gain5'
   *  Product: '<S1>/Divide4'
   *  Rounding: '<S1>/Round4'
   */
  rtY.stepperRBSteps = rt_roundd_snf(57.295779513082323 * -rtb_deltaR / 1.8);

  /* Product: '<S1>/Product' incorporates:
   *  Constant: '<S1>/integer maximum'
   *  Constant: '<S1>/max speed'
   *  Inport: '<Root>/controllerSpeed'
   *  Product: '<S1>/Divide'
   */
  rtb_deltaL = rtU.controllerSpeed / 2.147483647E+9 * 0.7;

  /* MATLAB Function: '<S1>/setting the desired speed of the shassis' incorporates:
   *  Constant: '<S1>/Constant10'
   *  Constant: '<S1>/Constant11'
   *  Inport: '<Root>/deltaTime'
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  rtb_deltaR = rtDW.UnitDelay2_DSTATE;
  if (rtDW.UnitDelay2_DSTATE >= 0.0) {
    if (rtDW.UnitDelay2_DSTATE < rtb_deltaL) {
      rtb_deltaR = 0.05 * rtU.deltaTime + rtDW.UnitDelay2_DSTATE;
    }

    if (rtb_deltaR > rtb_deltaL) {
      rtb_deltaR -= 0.1 * rtU.deltaTime;
    }
  }

  if (rtb_deltaR <= 0.0) {
    if (rtb_deltaR > rtb_deltaL) {
      rtb_deltaR -= 0.05 * rtU.deltaTime;
    }

    if (rtb_deltaR < rtb_deltaL) {
      rtb_deltaR += 0.1 * rtU.deltaTime;
    }
  }

  /* Update for UnitDelay: '<S1>/Unit Delay2' incorporates:
   *  MATLAB Function: '<S1>/setting the desired speed of the shassis'
   */
  rtDW.UnitDelay2_DSTATE = rtb_deltaR;
}

/* Model initialize function */
void control_drive_manual_initialize(void)
{
  /* ConstCode for Outport: '<Root>/stepperLFFrequency' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtY.stepperLFFrequency = 100.0;

  /* ConstCode for Outport: '<Root>/stepperLBFrequency' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtY.stepperLBFrequency = 100.0;

  /* ConstCode for Outport: '<Root>/stepperRFFrequency' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtY.stepperRFFrequency = 100.0;

  /* ConstCode for Outport: '<Root>/stepperRBFrequency' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtY.stepperRBFrequency = 100.0;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
