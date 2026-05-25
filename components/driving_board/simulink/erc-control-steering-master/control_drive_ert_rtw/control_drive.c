/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_drive.c
 *
 * Code generated for Simulink model 'control_drive'.
 *
 * Model version                  : 3.32
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Wed Apr 15 12:43:44 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control_drive.h"
#include "rtwtypes.h"
#include <math.h>

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
static void MATLABFunction1(real_T rtu_current, real_T rtu_desired, real_T
  rtu_accelleration, real_T *rty_out);

/*
 * Output and update for atomic system:
 *    '<S1>/MATLAB Function1'
 *    '<S1>/MATLAB Function2'
 *    '<S1>/MATLAB Function3'
 *    '<S1>/MATLAB Function4'
 *    '<S1>/MATLAB Function5'
 *    '<S1>/MATLAB Function6'
 */
static void MATLABFunction1(real_T rtu_current, real_T rtu_desired, real_T
  rtu_accelleration, real_T *rty_out)
{
  real_T current;
  current = rtu_current;
  if (rtu_current < rtu_desired) {
    current = rtu_current + rtu_accelleration;
  }

  if (current > rtu_desired) {
    current -= rtu_accelleration;
  }

  *rty_out = current;
}

/* Model step function */
void control_drive_step(void)
{
  real_T UnitDelay10_DSTATE_tmp;
  real_T UnitDelay4_DSTATE_tmp;
  real_T UnitDelay6_DSTATE_tmp;
  real_T UnitDelay8_DSTATE_tmp;
  real_T alpha;
  real_T rtb_Integrator_b_tmp;
  real_T rtb_Integrator_c;
  real_T rtb_deltaL;
  real_T rtb_deltaR;
  real_T rtb_out_a;
  real_T rtb_out_i;
  real_T rtb_out_j;

  /* Saturate: '<S1>/Saturation' incorporates:
   *  UnitDelay: '<S1>/Unit Delay'
   */
  if (rtDW.UnitDelay_DSTATE > 5.0) {
    /* Outport: '<Root>/controlLF' */
    rtY.controlLF = 5.0;
  } else if (rtDW.UnitDelay_DSTATE < -5.0) {
    /* Outport: '<Root>/controlLF' */
    rtY.controlLF = -5.0;
  } else {
    /* Outport: '<Root>/controlLF' */
    rtY.controlLF = rtDW.UnitDelay_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation' */

  /* Saturate: '<S1>/Saturation1' incorporates:
   *  UnitDelay: '<S1>/Unit Delay2'
   */
  if (rtDW.UnitDelay2_DSTATE > 5.0) {
    /* Outport: '<Root>/controlLM' */
    rtY.controlLM = 5.0;
  } else if (rtDW.UnitDelay2_DSTATE < -5.0) {
    /* Outport: '<Root>/controlLM' */
    rtY.controlLM = -5.0;
  } else {
    /* Outport: '<Root>/controlLM' */
    rtY.controlLM = rtDW.UnitDelay2_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation1' */

  /* Saturate: '<S1>/Saturation2' incorporates:
   *  UnitDelay: '<S1>/Unit Delay4'
   */
  if (rtDW.UnitDelay4_DSTATE > 5.0) {
    /* Outport: '<Root>/controlLB' */
    rtY.controlLB = 5.0;
  } else if (rtDW.UnitDelay4_DSTATE < -5.0) {
    /* Outport: '<Root>/controlLB' */
    rtY.controlLB = -5.0;
  } else {
    /* Outport: '<Root>/controlLB' */
    rtY.controlLB = rtDW.UnitDelay4_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation2' */

  /* Saturate: '<S1>/Saturation3' incorporates:
   *  UnitDelay: '<S1>/Unit Delay6'
   */
  if (rtDW.UnitDelay6_DSTATE > 5.0) {
    /* Outport: '<Root>/controlRF' */
    rtY.controlRF = 5.0;
  } else if (rtDW.UnitDelay6_DSTATE < -5.0) {
    /* Outport: '<Root>/controlRF' */
    rtY.controlRF = -5.0;
  } else {
    /* Outport: '<Root>/controlRF' */
    rtY.controlRF = rtDW.UnitDelay6_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation3' */

  /* Saturate: '<S1>/Saturation4' incorporates:
   *  UnitDelay: '<S1>/Unit Delay8'
   */
  if (rtDW.UnitDelay8_DSTATE > 5.0) {
    /* Outport: '<Root>/controlRM' */
    rtY.controlRM = 5.0;
  } else if (rtDW.UnitDelay8_DSTATE < -5.0) {
    /* Outport: '<Root>/controlRM' */
    rtY.controlRM = -5.0;
  } else {
    /* Outport: '<Root>/controlRM' */
    rtY.controlRM = rtDW.UnitDelay8_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation4' */

  /* Saturate: '<S1>/Saturation5' incorporates:
   *  UnitDelay: '<S1>/Unit Delay10'
   */
  if (rtDW.UnitDelay10_DSTATE > 5.0) {
    /* Outport: '<Root>/controlRB' */
    rtY.controlRB = 5.0;
  } else if (rtDW.UnitDelay10_DSTATE < -5.0) {
    /* Outport: '<Root>/controlRB' */
    rtY.controlRB = -5.0;
  } else {
    /* Outport: '<Root>/controlRB' */
    rtY.controlRB = rtDW.UnitDelay10_DSTATE;
  }

  /* End of Saturate: '<S1>/Saturation5' */

  /* MATLAB Function: '<S1>/getting the steering angles and turing radii' incorporates:
   *  Constant: '<S1>/Constant'
   *  Constant: '<S1>/Constant7'
   *  Inport: '<Root>/R'
   *  Inport: '<Root>/dist2goal'
   */
  if (rtU.R == 0.0) {
    alpha = 0.0;
  } else {
    alpha = rtU.dist2goal / rtU.R;
  }

  rtb_out_j = fabs(rtU.R);
  if (alpha < 0.0) {
    alpha = rtb_out_j + 0.37;
    rtb_out_i = rtb_out_j - 0.37;
    rtb_deltaL = atan(0.57 / (rtb_out_j + 0.37));
    rtb_deltaR = atan(0.57 / (rtb_out_j - 0.37));
  } else if (alpha > 0.0) {
    alpha = rtb_out_j - 0.37;
    rtb_out_i = rtb_out_j + 0.37;
    rtb_deltaL = -atan(0.57 / (rtb_out_j - 0.37));
    rtb_deltaR = -atan(0.57 / (rtb_out_j + 0.37));
  } else {
    rtb_deltaL = 0.0;
    rtb_deltaR = 0.0;
    alpha = rtb_out_j;
    rtb_out_i = rtb_out_j;
  }

  /* Outport: '<Root>/StepsLF' incorporates:
   *  Constant: '<S1>/Constant6'
   *  Gain: '<S1>/Gain'
   *  Product: '<S1>/Divide'
   */
  rtY.StepsLF = 57.295779513082323 * rtb_deltaL / 1.8;

  /* Outport: '<Root>/StepsLB' incorporates:
   *  Constant: '<S1>/Constant3'
   *  Gain: '<S1>/Gain2'
   *  Gain: '<S1>/Gain4'
   *  Product: '<S1>/Divide2'
   */
  rtY.StepsLB = 57.295779513082323 * -rtb_deltaL / 1.8;

  /* Outport: '<Root>/StepsRF' incorporates:
   *  Constant: '<S1>/Constant4'
   *  Gain: '<S1>/Gain3'
   *  Product: '<S1>/Divide3'
   */
  rtY.StepsRF = 57.295779513082323 * rtb_deltaR / 1.8;

  /* Outport: '<Root>/StepsRB' incorporates:
   *  Constant: '<S1>/Constant5'
   *  Gain: '<S1>/Gain5'
   *  Gain: '<S1>/Gain6'
   *  Product: '<S1>/Divide4'
   */
  rtY.StepsRB = 57.295779513082323 * -rtb_deltaR / 1.8;

  /* MATLAB Function: '<S1>/setting desired speed dependant  on the distance to the goal' incorporates:
   *  Constant: '<S1>/desired speed'
   *  Inport: '<Root>/dist2goal'
   */
  if (rtU.dist2goal <= 1.0) {
    rtb_deltaR = 0.0;
  } else if (rtU.dist2goal <= 2.0) {
    rtb_deltaR = 0.35;
  } else {
    rtb_deltaR = 0.7;
  }

  /* End of MATLAB Function: '<S1>/setting desired speed dependant  on the distance to the goal' */

  /* MATLAB Function: '<S1>/getting the wheel speeds from the turning radii' incorporates:
   *  Constant: '<S1>/Constant2'
   *  MATLAB Function: '<S1>/getting the steering angles and turing radii'
   */
  if (rtb_out_j > 0.0) {
    rtb_deltaL = rtb_deltaR / 0.1;
    rtb_out_a = sqrt(alpha * alpha + 0.081224999999999992) * rtb_deltaL /
      rtb_out_j;
    rtb_Integrator_b_tmp = rtb_out_j * 0.1;
    rtb_Integrator_c = rtb_deltaR * alpha / rtb_Integrator_b_tmp;
    rtb_deltaL = sqrt(rtb_out_i * rtb_out_i + 0.081224999999999992) * rtb_deltaL
      / rtb_out_j;
    alpha = rtb_deltaR * rtb_out_i / rtb_Integrator_b_tmp;
  } else {
    rtb_out_a = rtb_deltaR / 0.1;
    rtb_Integrator_c = rtb_out_a;
    rtb_deltaL = rtb_out_a;
    alpha = rtb_out_a;
  }

  /* Product: '<S1>/Product' incorporates:
   *  Constant: '<S1>/acceleration'
   *  Inport: '<Root>/deltaTime'
   */
  rtb_deltaR = 43.0 * rtU.deltaTime;

  /* MATLAB Function: '<S1>/getting the wheel speeds from the turning radii' incorporates:
   *  Constant: '<S1>/Constant1'
   */
  rtb_Integrator_b_tmp = rtb_out_a * 43.0;

  /* MATLAB Function: '<S1>/MATLAB Function1' incorporates:
   *  MATLAB Function: '<S1>/getting the wheel speeds from the turning radii'
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  MATLABFunction1(rtDW.UnitDelay1_DSTATE, rtb_Integrator_b_tmp, rtb_deltaR,
                  &rtb_out_j);

  /* MATLAB Function: '<S1>/MATLAB Function2' incorporates:
   *  Constant: '<S1>/Constant1'
   *  MATLAB Function: '<S1>/getting the wheel speeds from the turning radii'
   *  UnitDelay: '<S1>/Unit Delay3'
   */
  MATLABFunction1(rtDW.UnitDelay3_DSTATE, rtb_Integrator_c * 43.0, rtb_deltaR,
                  &rtb_out_i);

  /* MATLAB Function: '<S1>/MATLAB Function3' incorporates:
   *  UnitDelay: '<S1>/Unit Delay5'
   */
  MATLABFunction1(rtDW.UnitDelay5_DSTATE, rtb_Integrator_b_tmp, rtb_deltaR,
                  &rtb_out_a);

  /* MATLAB Function: '<S1>/getting the wheel speeds from the turning radii' incorporates:
   *  Constant: '<S1>/Constant1'
   */
  rtb_Integrator_b_tmp = rtb_deltaL * 43.0;

  /* MATLAB Function: '<S1>/MATLAB Function4' incorporates:
   *  MATLAB Function: '<S1>/getting the wheel speeds from the turning radii'
   *  UnitDelay: '<S1>/Unit Delay7'
   */
  MATLABFunction1(rtDW.UnitDelay7_DSTATE, rtb_Integrator_b_tmp, rtb_deltaR,
                  &rtb_Integrator_c);

  /* MATLAB Function: '<S1>/MATLAB Function5' incorporates:
   *  Constant: '<S1>/Constant1'
   *  MATLAB Function: '<S1>/getting the wheel speeds from the turning radii'
   *  UnitDelay: '<S1>/Unit Delay9'
   */
  MATLABFunction1(rtDW.UnitDelay9_DSTATE, alpha * 43.0, rtb_deltaR, &rtb_deltaL);

  /* MATLAB Function: '<S1>/MATLAB Function6' incorporates:
   *  UnitDelay: '<S1>/Unit Delay11'
   */
  MATLABFunction1(rtDW.UnitDelay11_DSTATE, rtb_Integrator_b_tmp, rtb_deltaR,
                  &alpha);

  /* Gain: '<S56>/Proportional Gain' incorporates:
   *  Gain: '<S48>/Integral Gain'
   *  Inport: '<Root>/LFActualSpeed'
   *  Sum: '<S1>/Sum'
   *  UnitDelay: '<S1>/Unit Delay1'
   */
  rtb_deltaR = (rtDW.UnitDelay1_DSTATE - rtU.LFActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay' incorporates:
   *  DiscreteIntegrator: '<S51>/Integrator'
   *  Gain: '<S56>/Proportional Gain'
   *  Sum: '<S60>/Sum'
   */
  rtDW.UnitDelay_DSTATE = rtb_deltaR + rtDW.Integrator_DSTATE_p;

  /* Gain: '<S108>/Proportional Gain' incorporates:
   *  Gain: '<S100>/Integral Gain'
   *  Inport: '<Root>/LMActualSpeed'
   *  Sum: '<S1>/Sum2'
   *  UnitDelay: '<S1>/Unit Delay3'
   */
  rtb_Integrator_b_tmp = (rtDW.UnitDelay3_DSTATE - rtU.LMActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay2' incorporates:
   *  DiscreteIntegrator: '<S103>/Integrator'
   *  Gain: '<S108>/Proportional Gain'
   *  Sum: '<S112>/Sum'
   */
  rtDW.UnitDelay2_DSTATE = rtb_Integrator_b_tmp + rtDW.Integrator_DSTATE;

  /* Gain: '<S160>/Proportional Gain' incorporates:
   *  Gain: '<S152>/Integral Gain'
   *  Inport: '<Root>/LBActualSpeed'
   *  Sum: '<S1>/Sum3'
   *  UnitDelay: '<S1>/Unit Delay5'
   */
  UnitDelay4_DSTATE_tmp = (rtDW.UnitDelay5_DSTATE - rtU.LBActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay4' incorporates:
   *  DiscreteIntegrator: '<S155>/Integrator'
   *  Gain: '<S160>/Proportional Gain'
   *  Sum: '<S164>/Sum'
   */
  rtDW.UnitDelay4_DSTATE = UnitDelay4_DSTATE_tmp + rtDW.Integrator_DSTATE_n;

  /* Gain: '<S212>/Proportional Gain' incorporates:
   *  Gain: '<S204>/Integral Gain'
   *  Inport: '<Root>/RFActualSpeed'
   *  Sum: '<S1>/Sum4'
   *  UnitDelay: '<S1>/Unit Delay7'
   */
  UnitDelay6_DSTATE_tmp = (rtDW.UnitDelay7_DSTATE - rtU.RFActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay6' incorporates:
   *  DiscreteIntegrator: '<S207>/Integrator'
   *  Gain: '<S212>/Proportional Gain'
   *  Sum: '<S216>/Sum'
   */
  rtDW.UnitDelay6_DSTATE = UnitDelay6_DSTATE_tmp + rtDW.Integrator_DSTATE_o;

  /* Gain: '<S264>/Proportional Gain' incorporates:
   *  Gain: '<S256>/Integral Gain'
   *  Inport: '<Root>/RMActualSpeed'
   *  Sum: '<S1>/Sum5'
   *  UnitDelay: '<S1>/Unit Delay9'
   */
  UnitDelay8_DSTATE_tmp = (rtDW.UnitDelay9_DSTATE - rtU.RMActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay8' incorporates:
   *  DiscreteIntegrator: '<S259>/Integrator'
   *  Gain: '<S264>/Proportional Gain'
   *  Sum: '<S268>/Sum'
   */
  rtDW.UnitDelay8_DSTATE = UnitDelay8_DSTATE_tmp + rtDW.Integrator_DSTATE_f;

  /* Gain: '<S316>/Proportional Gain' incorporates:
   *  Gain: '<S308>/Integral Gain'
   *  Inport: '<Root>/RBActualSpeed'
   *  Sum: '<S1>/Sum6'
   *  UnitDelay: '<S1>/Unit Delay11'
   */
  UnitDelay10_DSTATE_tmp = (rtDW.UnitDelay11_DSTATE - rtU.RBActualSpeed) * 0.2;

  /* Update for UnitDelay: '<S1>/Unit Delay10' incorporates:
   *  DiscreteIntegrator: '<S311>/Integrator'
   *  Gain: '<S316>/Proportional Gain'
   *  Sum: '<S320>/Sum'
   */
  rtDW.UnitDelay10_DSTATE = UnitDelay10_DSTATE_tmp + rtDW.Integrator_DSTATE_e;

  /* Update for UnitDelay: '<S1>/Unit Delay1' */
  rtDW.UnitDelay1_DSTATE = rtb_out_j;

  /* Update for UnitDelay: '<S1>/Unit Delay3' */
  rtDW.UnitDelay3_DSTATE = rtb_out_i;

  /* Update for DiscreteIntegrator: '<S103>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S116>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE += rtb_Integrator_b_tmp * rtU.deltaTime;

  /* Update for UnitDelay: '<S1>/Unit Delay5' */
  rtDW.UnitDelay5_DSTATE = rtb_out_a;

  /* Update for DiscreteIntegrator: '<S155>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S168>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE_n += UnitDelay4_DSTATE_tmp * rtU.deltaTime;

  /* Update for UnitDelay: '<S1>/Unit Delay7' */
  rtDW.UnitDelay7_DSTATE = rtb_Integrator_c;

  /* Update for DiscreteIntegrator: '<S207>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S220>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE_o += UnitDelay6_DSTATE_tmp * rtU.deltaTime;

  /* Update for UnitDelay: '<S1>/Unit Delay9' */
  rtDW.UnitDelay9_DSTATE = rtb_deltaL;

  /* Update for DiscreteIntegrator: '<S259>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S272>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE_f += UnitDelay8_DSTATE_tmp * rtU.deltaTime;

  /* Update for UnitDelay: '<S1>/Unit Delay11' */
  rtDW.UnitDelay11_DSTATE = alpha;

  /* Update for DiscreteIntegrator: '<S311>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S324>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE_e += UnitDelay10_DSTATE_tmp * rtU.deltaTime;

  /* Update for DiscreteIntegrator: '<S51>/Integrator' incorporates:
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S64>/Uintegral*Ts Prod Out'
   */
  rtDW.Integrator_DSTATE_p += rtb_deltaR * rtU.deltaTime;
}

/* Model initialize function */
void control_drive_initialize(void)
{
  /* ConstCode for Outport: '<Root>/FreqLF' incorporates:
   *  Constant: '<S1>/frequency'
   */
  rtY.FreqLF = 50.0;

  /* ConstCode for Outport: '<Root>/FreqLB' incorporates:
   *  Constant: '<S1>/frequency'
   */
  rtY.FreqLB = 50.0;

  /* ConstCode for Outport: '<Root>/FreqRF' incorporates:
   *  Constant: '<S1>/frequency'
   */
  rtY.FreqRF = 50.0;

  /* ConstCode for Outport: '<Root>/FreqRB' incorporates:
   *  Constant: '<S1>/frequency'
   */
  rtY.FreqRB = 50.0;
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
