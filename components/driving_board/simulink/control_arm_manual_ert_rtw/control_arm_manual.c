/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_arm_manual.c
 *
 * Code generated for Simulink model 'control_arm_manual'.
 *
 * Model version                  : 1.8
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Thu May 21 11:07:17 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#include "control_arm_manual.h"
#include <math.h>
#include "rtwtypes.h"
#include "math.h"

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;
extern real_T rt_powd_snf(real_T u0, real_T u1);
extern real_T rt_roundd_snf(real_T u);
static real_T rtGetInf(void);
static real32_T rtGetInfF(void);
static real_T rtGetMinusInf(void);
static real32_T rtGetMinusInfF(void);
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

/* Return rtInf needed by the generated code. */
static real_T rtGetInf(void)
{
  return rtInf;
}

/* Get rtInfF needed by the generated code. */
static real32_T rtGetInfF(void)
{
  return rtInfF;
}

/* Return rtMinusInf needed by the generated code. */
static real_T rtGetMinusInf(void)
{
  return rtMinusInf;
}

/* Return rtMinusInfF needed by the generated code. */
static real32_T rtGetMinusInfF(void)
{
  return rtMinusInfF;
}

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

real_T rt_powd_snf(real_T u0, real_T u1)
{
  real_T y;
  if (rtIsNaN(u0) || rtIsNaN(u1)) {
    y = (rtNaN);
  } else {
    real_T tmp;
    real_T tmp_0;
    tmp = fabs(u0);
    tmp_0 = fabs(u1);
    if (rtIsInf(u1)) {
      if (tmp == 1.0) {
        y = 1.0;
      } else if (tmp > 1.0) {
        if (u1 > 0.0) {
          y = (rtInf);
        } else {
          y = 0.0;
        }
      } else if (u1 > 0.0) {
        y = 0.0;
      } else {
        y = (rtInf);
      }
    } else if (tmp_0 == 0.0) {
      y = 1.0;
    } else if (tmp_0 == 1.0) {
      if (u1 > 0.0) {
        y = u0;
      } else {
        y = 1.0 / u0;
      }
    } else if (u1 == 2.0) {
      y = u0 * u0;
    } else if ((u1 == 0.5) && (u0 >= 0.0)) {
      y = sqrt(u0);
    } else if ((u0 < 0.0) && (u1 > floor(u1))) {
      y = (rtNaN);
    } else {
      y = pow(u0, u1);
    }
  }

  return y;
}

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
void control_arm_manual_step(void)
{
  real_T P2planar[16];
  real_T P4planar[16];
  real_T P6i[16];
  real_T theta5_0[16];
  real_T theta5_1[16];
  real_T rtb_angles[5];
  real_T P6i_0;
  real_T XE;
  real_T XEi;
  real_T YE;
  real_T a;
  real_T angToBase;
  real_T theta0;
  real_T theta2;
  real_T theta3;
  real_T theta5;
  int16_T P6i_tmp_1;
  int16_T P6i_tmp_2;
  int16_T i;
  int16_T i_0;
  int16_T theta5_tmp;
  int16_T theta5_tmp_0;
  int8_T P2planar_tmp[16];
  int8_T P6i_tmp[4];
  int8_T P6i_tmp_0[4];
  static const int8_T b[4] = { 0, 0, 0, 1 };

  static const int8_T c[4] = { 0, 0, 1, 0 };

  static const int8_T b_b[16] = { 1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0,
    1 };

  static const real_T d[5] = { 0.0, 1.5707963267948966, 0.0, 0.0,
    3.1415926535897931 };

  static const real_T c_b[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.3, 0.0, 0.0, 1.0 };

  static const real_T a_0[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, 0.0, 0.0, 0.065, 1.0 };

  static const int8_T c_a[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1
  };

  static const real_T a_1[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    -1.0, 0.0, 0.0, 0.0, 0.0, 0.149, 1.0 };

  static const real_T d_b[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, -0.12, 0.0, 0.0, 1.0 };

  static const real_T b_b_0[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.35, 0.0, 0.0, 1.0 };

  static const real_T c_b_0[16] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.12, 0.0, 0.0, 1.0 };

  /* MATLAB Function: '<S1>/inverse kinematics' incorporates:
   *  Inport: '<Root>/gripperAng'
   *  Inport: '<Root>/x'
   *  Inport: '<Root>/y'
   *  Inport: '<Root>/z'
   */
  if (rtU.x < 0.0) {
    theta0 = atan(rtU.y / rtU.x) + 3.1415926535897931;
  } else {
    theta0 = atan(rtU.y / rtU.x);
  }

  angToBase = theta0 - 3.1415926535897931;
  if (theta0 > 3.1415926535897931) {
    theta0 -= 6.2831853071795862;
  }

  theta0 = -theta0;
  XEi = sin(angToBase);
  theta5 = cos(angToBase);
  XE = sin(rtU.gripperAng);
  YE = cos(rtU.gripperAng);
  theta5_0[0] = theta5;
  theta5_0[4] = -XEi;
  theta5_0[8] = 0.0;
  theta5_0[12] = rtU.x;
  theta5_0[1] = XEi;
  theta5_0[5] = theta5;
  theta5_0[9] = 0.0;
  theta5_0[13] = rtU.y;
  theta5_0[2] = 0.0;
  theta5_0[6] = 0.0;
  theta5_0[10] = 1.0;
  theta5_0[14] = rtU.z;
  P2planar[0] = YE;
  P2planar[4] = -XE;
  P2planar[8] = 0.0;
  P2planar[12] = 0.0;
  P2planar[1] = XE;
  P2planar[5] = YE;
  P2planar[9] = 0.0;
  P2planar[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    /* MATLAB Function: '<S1>/MATLAB Function4' */
    P6i_tmp_1 = b[i_0];
    P6i_tmp[i_0] = (int8_T)P6i_tmp_1;

    /* MATLAB Function: '<S1>/MATLAB Function4' */
    P6i_tmp_2 = c[i_0];
    P6i_tmp_0[i_0] = (int8_T)P6i_tmp_2;
    theta5_tmp = i_0 << 2;
    theta5_0[theta5_tmp + 3] = P6i_tmp_1;
    P2planar[theta5_tmp + 2] = P6i_tmp_2;
    P2planar[theta5_tmp + 3] = P6i_tmp_1;
    theta5_1[theta5_tmp] = 0.0;
    theta5_1[theta5_tmp + 1] = 0.0;
    theta5_1[theta5_tmp + 2] = 0.0;
    theta5_1[theta5_tmp + 3] = 0.0;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    theta5 = theta5_1[theta5_tmp];
    theta2 = theta5_1[theta5_tmp + 1];
    angToBase = theta5_1[theta5_tmp + 2];
    XEi = theta5_1[theta5_tmp + 3];
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = theta5_tmp + i;
      P6i_tmp_2 = b_b[P6i_tmp_1];
      theta5_tmp_0 = i << 2;
      theta5 += theta5_0[theta5_tmp_0] * (real_T)P6i_tmp_2;
      theta2 += theta5_0[theta5_tmp_0 + 1] * (real_T)P6i_tmp_2;
      angToBase += theta5_0[theta5_tmp_0 + 2] * (real_T)P6i_tmp_2;
      XEi += theta5_0[theta5_tmp_0 + 3] * (real_T)P6i_tmp_2;
      P4planar[P6i_tmp_1] = 0.0;
    }

    theta5_1[theta5_tmp + 3] = XEi;
    theta5_1[theta5_tmp + 2] = angToBase;
    theta5_1[theta5_tmp + 1] = theta2;
    theta5_1[theta5_tmp] = theta5;
    YE = P4planar[theta5_tmp];
    angToBase = P4planar[theta5_tmp + 1];
    XEi = P4planar[theta5_tmp + 2];
    theta3 = P4planar[theta5_tmp + 3];
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = theta5_tmp + i;
      theta2 = c_b[P6i_tmp_1];
      theta5_tmp_0 = i << 2;
      YE += P2planar[theta5_tmp_0] * theta2;
      angToBase += P2planar[theta5_tmp_0 + 1] * theta2;
      XEi += P2planar[theta5_tmp_0 + 2] * theta2;
      theta3 += P2planar[theta5_tmp_0 + 3] * theta2;
      P6i[P6i_tmp_1] = 0.0;
    }

    P4planar[theta5_tmp + 3] = theta3;
    P4planar[theta5_tmp + 2] = XEi;
    P4planar[theta5_tmp + 1] = angToBase;
    P4planar[theta5_tmp] = YE;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    P6i_tmp_1 = i_0 << 2;
    YE = P6i[P6i_tmp_1];
    XE = P6i[P6i_tmp_1 + 1];
    theta5 = P6i[P6i_tmp_1 + 2];
    P6i_0 = P6i[P6i_tmp_1 + 3];
    for (i = 0; i < 4; i++) {
      theta2 = P4planar[P6i_tmp_1 + i];
      P6i_tmp_2 = i << 2;
      YE += theta5_1[P6i_tmp_2] * theta2;
      XE += theta5_1[P6i_tmp_2 + 1] * theta2;
      theta5 += theta5_1[P6i_tmp_2 + 2] * theta2;
      P6i_0 += theta5_1[P6i_tmp_2 + 3] * theta2;
    }

    P6i[P6i_tmp_1 + 3] = P6i_0;
    P6i[P6i_tmp_1 + 2] = theta5;
    P6i[P6i_tmp_1 + 1] = XE;
    P6i[P6i_tmp_1] = YE;
  }

  XEi = sqrt(P6i[12] * P6i[12] + P6i[13] * P6i[13]);
  theta2 = XEi * XEi;
  theta3 = (P6i[14] - 0.065) * (P6i[14] - 0.065);
  theta5 = (((((((theta2 * 0.24499999999999997 + 0.04624375) + theta3 *
                 0.24499999999999997) - 0.0625) + theta2 * 0.5) + theta3 * 0.5)
             - rt_powd_snf(XEi, 4.0)) - theta2 * 2.0 * theta3) - rt_powd_snf
    (P6i[14] - 0.065, 4.0);
  if (theta5 < 0.0) {
    for (i_0 = 0; i_0 < 5; i_0++) {
      rtb_angles[i_0] = d[i_0];
    }

    /* Outport: '<Root>/error' */
    rtY.error = 1.0;
  } else {
    angToBase = atan(((P6i[14] - 0.065) * 0.7 + sqrt(theta5)) / ((((0.7 * XEi +
      0.12249999999999998) - 0.25) + theta2) + theta3)) * 2.0;
    XEi = (theta2 - 0.02250000000000002) + theta3;
    theta2 = atan(sqrt(((0.72249999999999992 - theta2) - theta3) * XEi) / XEi) *
      -2.0;
    theta3 = (-rtU.gripperAng - angToBase) - theta2;
    XEi = sin(angToBase);
    theta5 = cos(angToBase);
    XE = sin(theta2);
    YE = cos(theta2);
    theta5_0[0] = theta5;
    theta5_0[4] = -XEi;
    theta5_0[8] = 0.0;
    theta5_0[12] = 0.0;
    theta5_0[1] = XEi;
    theta5_0[5] = theta5;
    theta5_0[9] = 0.0;
    theta5_0[13] = 0.0;
    for (i_0 = 0; i_0 < 4; i_0++) {
      theta5_tmp = i_0 << 2;
      theta5_0[theta5_tmp + 2] = P6i_tmp_0[i_0];
      theta5_0[theta5_tmp + 3] = P6i_tmp[i_0];
      XEi = 0.0;
      theta5 = 0.0;
      P6i_0 = 0.0;
      a = 0.0;
      for (i = 0; i < 4; i++) {
        theta2 = theta5_0[theta5_tmp + i];
        P6i_tmp_1 = i << 2;
        XEi += a_0[P6i_tmp_1] * theta2;
        theta5 += a_0[P6i_tmp_1 + 1] * theta2;
        P6i_0 += a_0[P6i_tmp_1 + 2] * theta2;
        a += a_0[P6i_tmp_1 + 3] * theta2;
      }

      P6i[theta5_tmp + 3] = a;
      P6i[theta5_tmp + 2] = P6i_0;
      P6i[theta5_tmp + 1] = theta5;
      P6i[theta5_tmp] = XEi;
    }

    P2planar[0] = YE;
    P2planar[4] = -XE;
    P2planar[8] = 0.0;
    P2planar[12] = 0.35;
    P2planar[1] = XE;
    P2planar[5] = YE;
    P2planar[9] = 0.0;
    P2planar[13] = 0.0;
    for (i_0 = 0; i_0 < 4; i_0++) {
      theta5_tmp_0 = i_0 << 2;
      P2planar[theta5_tmp_0 + 2] = P6i_tmp_0[i_0];
      P2planar[theta5_tmp_0 + 3] = P6i_tmp[i_0];
      YE = 0.0;
      XE = 0.0;
      theta5 = 0.0;
      P6i_0 = 0.0;
      for (i = 0; i < 4; i++) {
        P6i_tmp_1 = theta5_tmp_0 + i;
        theta2 = P6i[P6i_tmp_1];
        P6i_tmp_2 = i << 2;
        YE += (real_T)c_a[P6i_tmp_2] * theta2;
        XE += (real_T)c_a[P6i_tmp_2 + 1] * theta2;
        theta5 += (real_T)c_a[P6i_tmp_2 + 2] * theta2;
        P6i_0 += (real_T)c_a[P6i_tmp_2 + 3] * theta2;
        theta5_1[P6i_tmp_1] = 0.0;
      }

      theta5_0[theta5_tmp_0 + 3] = P6i_0;
      theta5_0[theta5_tmp_0 + 2] = theta5;
      theta5_0[theta5_tmp_0 + 1] = XE;
      theta5_0[theta5_tmp_0] = YE;
    }

    for (i_0 = 0; i_0 < 4; i_0++) {
      P6i_tmp_2 = i_0 << 2;
      YE = theta5_1[P6i_tmp_2];
      XE = theta5_1[P6i_tmp_2 + 1];
      theta5 = theta5_1[P6i_tmp_2 + 2];
      P6i_0 = theta5_1[P6i_tmp_2 + 3];
      for (i = 0; i < 4; i++) {
        theta2 = P2planar[P6i_tmp_2 + i];
        P6i_tmp_1 = i << 2;
        YE += theta5_0[P6i_tmp_1] * theta2;
        XE += theta5_0[P6i_tmp_1 + 1] * theta2;
        theta5 += theta5_0[P6i_tmp_1 + 2] * theta2;
        P6i_0 += theta5_0[P6i_tmp_1 + 3] * theta2;
      }

      theta5_1[P6i_tmp_2 + 3] = P6i_0;
      theta5_1[P6i_tmp_2 + 2] = theta5;
      theta5_1[P6i_tmp_2 + 1] = XE;
      theta5_1[P6i_tmp_2] = YE;
    }

    for (i_0 = 0; i_0 < 4; i_0++) {
      YE = 0.0;
      XE = 0.0;
      theta5 = 0.0;
      P6i_0 = 0.0;
      for (i = 0; i < 4; i++) {
        theta2 = d_b[(i_0 << 2) + i];
        P6i_tmp_1 = i << 2;
        YE += theta5_1[P6i_tmp_1] * theta2;
        XE += theta5_1[P6i_tmp_1 + 1] * theta2;
        theta5 += theta5_1[P6i_tmp_1 + 2] * theta2;
        P6i_0 += theta5_1[P6i_tmp_1 + 3] * theta2;
      }

      P6i_tmp_1 = i_0 << 2;
      P6i[P6i_tmp_1 + 3] = P6i_0;
      P6i[P6i_tmp_1 + 2] = theta5;
      P6i[P6i_tmp_1 + 1] = XE;
      P6i[P6i_tmp_1] = YE;
    }

    theta2 = P6i[12] * P6i[12];
    XEi = (P6i[14] - 0.149) * (P6i[14] - 0.149);
    XEi = atan((sqrt((((((((theta2 * 0.0288 + 0.00205056) + XEi * 0.0288) -
                          0.0061465600000000023) + theta2 * 0.15680000000000002)
                        + XEi * 0.15680000000000002) - rt_powd_snf(P6i[12], 4.0))
                      - theta2 * 2.0 * XEi) - rt_powd_snf(P6i[14] - 0.149, 4.0))
                + (P6i[14] - 0.149) * 0.24) / ((((0.24 * P6i[12] + 0.0144) -
      0.078400000000000011) + theta2) + XEi)) * 2.0;
    if ((fabs(theta0) > 3.1415926535897931) || ((angToBase < 0.0) || (angToBase >
          2.3561944901923448) || (theta3 < -2.3561944901923448) || (theta3 >
          0.78539816339744828) || (XEi < 1.5707963267948966) || (XEi >
          4.71238898038469))) {
      for (i_0 = 0; i_0 < 5; i_0++) {
        rtb_angles[i_0] = d[i_0];
      }

      /* Outport: '<Root>/error' */
      rtY.error = 2.0;
    } else {
      rtb_angles[0] = theta0;
      rtb_angles[1] = angToBase;
      rtb_angles[3] = theta3;
      rtb_angles[4] = XEi;

      /* Outport: '<Root>/error' */
      rtY.error = 0.0;
    }
  }

  /* End of MATLAB Function: '<S1>/inverse kinematics' */

  /* Outport: '<Root>/controlGripperPitch' incorporates:
   *  Gain: '<S1>/Gain4'
   */
  rtY.controlGripperPitch = 57.295779513082323 * rtb_angles[3];

  /* Outport: '<Root>/controlBase' incorporates:
   *  Gain: '<S1>/Gain6'
   */
  rtY.controlBase = 63.0 * rtb_angles[0];

  /* Outport: '<Root>/stepperLeftSteps' incorporates:
   *  Constant: '<S1>/Constant5'
   *  Constant: '<S1>/Pi'
   *  Gain: '<S1>/Gain10'
   *  Gain: '<S1>/Gain13'
   *  Product: '<S1>/Divide1'
   *  Rounding: '<S1>/Round'
   *  Sum: '<S1>/Sum7'
   */
  rtY.stepperLeftSteps = rt_roundd_snf((rtb_angles[4] + 1.5707963267948966) *
    160.0 * 57.295779513082323 / 1.8);

  /* Outport: '<Root>/stepperRightSteps' incorporates:
   *  Constant: '<S1>/Constant2'
   *  Constant: '<S1>/Pi1'
   *  Gain: '<S1>/Gain11'
   *  Gain: '<S1>/Gain8'
   *  Product: '<S1>/Divide2'
   *  Rounding: '<S1>/Round1'
   *  Sum: '<S1>/Sum9'
   */
  rtY.stepperRightSteps = rt_roundd_snf((rtb_angles[1] + 1.5707963267948966) *
    160.0 * 57.295779513082323 / 1.8);

  /* Gain: '<S1>/Gain15' incorporates:
   *  Gain: '<S1>/Gain14'
   *  Inport: '<Root>/baseActualPosition'
   */
  angToBase = 0.015873015873015872 * rtU.baseActualPosition *
    -0.015873015873015872;

  /* Sum: '<S1>/Sum11' incorporates:
   *  Constant: '<S1>/Pi2'
   *  Gain: '<S1>/Gain12'
   *  Gain: '<S1>/Gain17'
   *  Gain: '<S1>/degrees per step1'
   *  Gain: '<S1>/gearbox1'
   *  Inport: '<Root>/stepperRightActualPosition'
   */
  theta3 = 0.00625 * rtU.stepperRightActualPosition * 1.8 * 0.017453292519943295
    * 0.00625 - 1.5707963267948966;

  /* Gain: '<S1>/Gain16' incorporates:
   *  Inport: '<Root>/gripperPitchActualPosition'
   */
  theta0 = 0.047619047619047616 * rtU.gripperPitchActualPosition;

  /* Sum: '<S1>/Sum10' incorporates:
   *  Constant: '<S1>/Pi2'
   *  Gain: '<S1>/Gain18'
   *  Gain: '<S1>/Gain9'
   *  Gain: '<S1>/degrees per step'
   *  Gain: '<S1>/gearbox'
   *  Inport: '<Root>/stepperLeftActualPosition'
   */
  XEi = 0.00625 * rtU.stepperLeftActualPosition * 1.8 * 0.017453292519943295 *
    0.00625 - 1.5707963267948966;

  /* MATLAB Function: '<S1>/MATLAB Function4' */
  theta2 = sin(XEi);
  theta5 = cos(XEi);
  theta5_0[0] = theta5;
  theta5_0[4] = -theta2;
  theta5_0[8] = 0.0;
  theta5_0[12] = 0.0;
  theta5_0[1] = theta2;
  theta5_0[5] = theta5;
  theta5_0[9] = 0.0;
  theta5_0[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    theta5_0[theta5_tmp + 2] = P6i_tmp_0[i_0];
    theta5_0[theta5_tmp + 3] = P6i_tmp[i_0];
    YE = 0.0;
    XE = 0.0;
    theta5 = 0.0;
    P6i_0 = 0.0;
    for (i = 0; i < 4; i++) {
      theta2 = theta5_0[theta5_tmp + i];
      P6i_tmp_1 = i << 2;
      YE += a_1[P6i_tmp_1] * theta2;
      XE += a_1[P6i_tmp_1 + 1] * theta2;
      theta5 += a_1[P6i_tmp_1 + 2] * theta2;
      P6i_0 += a_1[P6i_tmp_1 + 3] * theta2;
    }

    P6i[theta5_tmp + 3] = P6i_0;
    P6i[theta5_tmp + 2] = theta5;
    P6i[theta5_tmp + 1] = XE;
    P6i[theta5_tmp] = YE;
  }

  theta2 = sin(theta3);
  theta3 = cos(theta3);
  for (i_0 = 0; i_0 < 16; i_0++) {
    P2planar_tmp[i_0] = c_a[i_0];
  }

  P2planar[0] = theta3;
  P2planar[4] = -theta2;
  P2planar[8] = 0.0;
  P2planar[12] = 0.0;
  P2planar[1] = theta2;
  P2planar[5] = theta3;
  P2planar[9] = 0.0;
  P2planar[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    P2planar[theta5_tmp + 2] = P6i_tmp_0[i_0];
    P2planar[theta5_tmp + 3] = P6i_tmp[i_0];
    YE = 0.0;
    XE = 0.0;
    theta5 = 0.0;
    P6i_0 = 0.0;
    for (i = 0; i < 4; i++) {
      theta2 = P2planar[theta5_tmp + i];
      P6i_tmp_2 = i << 2;
      YE += a_0[P6i_tmp_2] * theta2;
      XE += a_0[P6i_tmp_2 + 1] * theta2;
      theta5 += a_0[P6i_tmp_2 + 2] * theta2;
      P6i_0 += a_0[P6i_tmp_2 + 3] * theta2;
    }

    theta5_0[theta5_tmp + 3] = P6i_0;
    theta5_0[theta5_tmp + 2] = theta5;
    theta5_0[theta5_tmp + 1] = XE;
    theta5_0[theta5_tmp] = YE;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    theta3 = 0.0;
    YE = 0.0;
    XE = 0.0;
    theta5 = 0.0;
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = (i_0 << 2) + i;
      theta2 = theta5_0[P6i_tmp_1];
      P6i_tmp_2 = i << 2;
      theta3 += (real_T)P2planar_tmp[P6i_tmp_2] * theta2;
      YE += (real_T)P2planar_tmp[P6i_tmp_2 + 1] * theta2;
      XE += (real_T)P2planar_tmp[P6i_tmp_2 + 2] * theta2;
      theta5 += (real_T)P2planar_tmp[P6i_tmp_2 + 3] * theta2;
      P2planar[P6i_tmp_1] = 0.0;
    }

    P6i_tmp_2 = i_0 << 2;
    theta5_1[P6i_tmp_2 + 3] = theta5;
    theta5_1[P6i_tmp_2 + 2] = XE;
    theta5_1[P6i_tmp_2 + 1] = YE;
    theta5_1[P6i_tmp_2] = theta3;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    theta3 = P2planar[theta5_tmp];
    YE = P2planar[theta5_tmp + 1];
    XE = P2planar[theta5_tmp + 2];
    theta5 = P2planar[theta5_tmp + 3];
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = theta5_tmp + i;
      theta2 = b_b_0[P6i_tmp_1];
      P6i_tmp_2 = i << 2;
      theta3 += theta5_1[P6i_tmp_2] * theta2;
      YE += theta5_1[P6i_tmp_2 + 1] * theta2;
      XE += theta5_1[P6i_tmp_2 + 2] * theta2;
      theta5 += theta5_1[P6i_tmp_2 + 3] * theta2;
      theta5_0[P6i_tmp_1] = 0.0;
    }

    P2planar[theta5_tmp + 3] = theta5;
    P2planar[theta5_tmp + 2] = XE;
    P2planar[theta5_tmp + 1] = YE;
    P2planar[theta5_tmp] = theta3;
    theta3 = theta5_0[theta5_tmp];
    YE = theta5_0[theta5_tmp + 1];
    XE = theta5_0[theta5_tmp + 2];
    theta5 = theta5_0[theta5_tmp + 3];
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = theta5_tmp + i;
      theta2 = P6i[P6i_tmp_1];
      P6i_tmp_2 = i << 2;
      theta3 += (real_T)P2planar_tmp[P6i_tmp_2] * theta2;
      YE += (real_T)P2planar_tmp[P6i_tmp_2 + 1] * theta2;
      XE += (real_T)P2planar_tmp[P6i_tmp_2 + 2] * theta2;
      theta5 += (real_T)P2planar_tmp[P6i_tmp_2 + 3] * theta2;
      P4planar[P6i_tmp_1] = 0.0;
    }

    theta5_0[theta5_tmp + 3] = theta5;
    theta5_0[theta5_tmp + 2] = XE;
    theta5_0[theta5_tmp + 1] = YE;
    theta5_0[theta5_tmp] = theta3;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    P6i_tmp_1 = i_0 << 2;
    theta3 = P4planar[P6i_tmp_1];
    YE = P4planar[P6i_tmp_1 + 1];
    XE = P4planar[P6i_tmp_1 + 2];
    theta5 = P4planar[P6i_tmp_1 + 3];
    for (i = 0; i < 4; i++) {
      theta2 = c_b_0[P6i_tmp_1 + i];
      theta5_tmp = i << 2;
      theta3 += theta5_0[theta5_tmp] * theta2;
      YE += theta5_0[theta5_tmp + 1] * theta2;
      XE += theta5_0[theta5_tmp + 2] * theta2;
      theta5 += theta5_0[theta5_tmp + 3] * theta2;
    }

    P4planar[P6i_tmp_1 + 3] = theta5;
    P4planar[P6i_tmp_1 + 2] = XE;
    P4planar[P6i_tmp_1 + 1] = YE;
    P4planar[P6i_tmp_1] = theta3;
  }

  XE = P4planar[12] - P2planar[12];
  YE = P4planar[14] - P2planar[14];
  theta2 = XE * XE;
  theta3 = YE * YE;
  theta5 = (((((((theta2 * 0.15680000000000002 - 0.0038886400000000022) + theta3
                 * 0.15680000000000002) - 0.00020735999999999997) + theta2 *
               0.0288) + theta3 * 0.0288) - rt_powd_snf(XE, 4.0)) - theta2 * 2.0
            * theta3) - rt_powd_snf(YE, 4.0);
  if (theta5 > 0.0) {
    theta5 = atan((0.56 * YE + sqrt(theta5)) / ((((0.56 * XE +
      0.078400000000000011) - 0.0144) + theta2) + theta3)) * 2.0;
    YE = (theta2 - 0.0256) + theta3;
    XE = atan(sqrt(((0.16 - theta2) - theta3) * YE) / YE) * -2.0;
  } else {
    theta5 = 0.0;
    XE = 0.0;
  }

  theta5 += 3.1415926535897931 - XEi;
  theta2 = sin(angToBase);
  theta3 = cos(angToBase);
  angToBase = sin(theta5);
  XEi = cos(theta5);
  theta5 = sin(XE);
  XE = cos(XE);
  YE = sin(theta0);
  theta0 = cos(theta0);
  P2planar[0] = theta3;
  P2planar[4] = -theta2;
  P2planar[8] = 0.0;
  P2planar[12] = 0.0;
  P2planar[1] = theta2;
  P2planar[5] = theta3;
  P2planar[9] = 0.0;
  P2planar[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    P2planar[theta5_tmp + 2] = P6i_tmp_0[i_0];
    P2planar[theta5_tmp + 3] = P6i_tmp[i_0];
    theta5_0[theta5_tmp] = 0.0;
    theta5_0[theta5_tmp + 1] = 0.0;
    theta5_0[theta5_tmp + 2] = 0.0;
    theta5_0[theta5_tmp + 3] = 0.0;
  }

  P4planar[0] = XEi;
  P4planar[4] = -angToBase;
  P4planar[8] = 0.0;
  P4planar[12] = 0.12;
  P4planar[1] = angToBase;
  P4planar[5] = XEi;
  P4planar[9] = 0.0;
  P4planar[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    P6i_tmp_2 = i_0 << 2;
    P4planar[P6i_tmp_2 + 2] = P6i_tmp_0[i_0];
    P4planar[P6i_tmp_2 + 3] = P6i_tmp[i_0];
    theta3 = theta5_0[P6i_tmp_2];
    angToBase = theta5_0[P6i_tmp_2 + 1];
    XEi = theta5_0[P6i_tmp_2 + 2];
    P6i_0 = theta5_0[P6i_tmp_2 + 3];
    for (i = 0; i < 4; i++) {
      P6i_tmp_1 = P6i_tmp_2 + i;
      theta2 = P6i[P6i_tmp_1];
      theta5_tmp = i << 2;
      theta3 += P2planar[theta5_tmp] * theta2;
      angToBase += P2planar[theta5_tmp + 1] * theta2;
      XEi += P2planar[theta5_tmp + 2] * theta2;
      P6i_0 += P2planar[theta5_tmp + 3] * theta2;
      theta5_1[P6i_tmp_1] = 0.0;
    }

    theta5_0[P6i_tmp_2 + 3] = P6i_0;
    theta5_0[P6i_tmp_2 + 2] = XEi;
    theta5_0[P6i_tmp_2 + 1] = angToBase;
    theta5_0[P6i_tmp_2] = theta3;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    theta5_tmp = i_0 << 2;
    theta3 = theta5_1[theta5_tmp];
    angToBase = theta5_1[theta5_tmp + 1];
    XEi = theta5_1[theta5_tmp + 2];
    P6i_0 = theta5_1[theta5_tmp + 3];
    for (i = 0; i < 4; i++) {
      theta2 = P4planar[theta5_tmp + i];
      P6i_tmp_1 = i << 2;
      theta3 += theta5_0[P6i_tmp_1] * theta2;
      angToBase += theta5_0[P6i_tmp_1 + 1] * theta2;
      XEi += theta5_0[P6i_tmp_1 + 2] * theta2;
      P6i_0 += theta5_0[P6i_tmp_1 + 3] * theta2;
    }

    theta5_1[theta5_tmp + 3] = P6i_0;
    theta5_1[theta5_tmp + 2] = XEi;
    theta5_1[theta5_tmp + 1] = angToBase;
    theta5_1[theta5_tmp] = theta3;
  }

  P6i[0] = XE;
  P6i[4] = -theta5;
  P6i[8] = 0.0;
  P6i[12] = 0.28;
  P6i[1] = theta5;
  P6i[5] = XE;
  P6i[9] = 0.0;
  P6i[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    P6i_tmp_1 = i_0 << 2;
    P6i[P6i_tmp_1 + 2] = P6i_tmp_0[i_0];
    P6i[P6i_tmp_1 + 3] = P6i_tmp[i_0];
    theta3 = 0.0;
    angToBase = 0.0;
    XEi = 0.0;
    P6i_0 = 0.0;
    for (i = 0; i < 4; i++) {
      theta2 = P6i[P6i_tmp_1 + i];
      theta5_tmp = i << 2;
      theta3 += theta5_1[theta5_tmp] * theta2;
      angToBase += theta5_1[theta5_tmp + 1] * theta2;
      XEi += theta5_1[theta5_tmp + 2] * theta2;
      P6i_0 += theta5_1[theta5_tmp + 3] * theta2;
    }

    P2planar[P6i_tmp_1 + 3] = P6i_0;
    P2planar[P6i_tmp_1 + 2] = XEi;
    P2planar[P6i_tmp_1 + 1] = angToBase;
    P2planar[P6i_tmp_1] = theta3;
  }

  P6i[0] = theta0;
  P6i[4] = -YE;
  P6i[8] = 0.0;
  P6i[12] = 0.62;
  P6i[1] = YE;
  P6i[5] = theta0;
  P6i[9] = 0.0;
  P6i[13] = 0.0;
  for (i_0 = 0; i_0 < 4; i_0++) {
    P6i_tmp_1 = i_0 << 2;
    P6i[P6i_tmp_1 + 2] = P6i_tmp_0[i_0];
    P6i[P6i_tmp_1 + 3] = P6i_tmp[i_0];
    theta3 = 0.0;
    angToBase = 0.0;
    XEi = 0.0;
    P6i_0 = 0.0;
    for (i = 0; i < 4; i++) {
      theta2 = P6i[P6i_tmp_1 + i];
      theta5_tmp = i << 2;
      theta3 += P2planar[theta5_tmp] * theta2;
      angToBase += P2planar[theta5_tmp + 1] * theta2;
      XEi += P2planar[theta5_tmp + 2] * theta2;
      P6i_0 += P2planar[theta5_tmp + 3] * theta2;
    }

    theta5_0[P6i_tmp_1 + 3] = P6i_0;
    theta5_0[P6i_tmp_1 + 2] = XEi;
    theta5_0[P6i_tmp_1 + 1] = angToBase;
    theta5_0[P6i_tmp_1] = theta3;
  }

  for (i_0 = 0; i_0 < 4; i_0++) {
    theta3 = 0.0;
    YE = 0.0;
    XE = 0.0;
    theta5 = 0.0;
    for (i = 0; i < 4; i++) {
      theta2 = c_b[(i_0 << 2) + i];
      theta5_tmp = i << 2;
      theta3 += theta5_0[theta5_tmp] * theta2;
      YE += theta5_0[theta5_tmp + 1] * theta2;
      XE += theta5_0[theta5_tmp + 2] * theta2;
      theta5 += theta5_0[theta5_tmp + 3] * theta2;
    }

    theta5_tmp = i_0 << 2;
    P2planar[theta5_tmp + 3] = theta5;
    P2planar[theta5_tmp + 2] = XE;
    P2planar[theta5_tmp + 1] = YE;
    P2planar[theta5_tmp] = theta3;
  }

  /* Sum: '<S3>/Sum' incorporates:
   *  Inport: '<Root>/x'
   *  MATLAB Function: '<S1>/MATLAB Function4'
   */
  theta0 = P2planar[12] - rtU.x;

  /* DotProduct: '<S3>/Dot Product' */
  theta2 = theta0 * theta0;

  /* Sum: '<S3>/Sum' incorporates:
   *  Inport: '<Root>/y'
   *  MATLAB Function: '<S1>/MATLAB Function4'
   */
  theta0 = P2planar[13] - rtU.y;

  /* DotProduct: '<S3>/Dot Product' */
  theta2 += theta0 * theta0;

  /* Sum: '<S3>/Sum' incorporates:
   *  Inport: '<Root>/z'
   *  MATLAB Function: '<S1>/MATLAB Function4'
   */
  theta0 = P2planar[14] - rtU.z;

  /* Outport: '<Root>/pointReached' incorporates:
   *  Constant: '<S2>/Constant'
   *  DotProduct: '<S3>/Dot Product'
   *  RelationalOperator: '<S2>/Compare'
   *  Sqrt: '<S3>/sqrt'
   */
  rtY.pointReached = (sqrt(theta0 * theta0 + theta2) <= 0.002);

  /* Outport: '<Root>/controlJaw' incorporates:
   *  Gain: '<S1>/Gain3'
   *  Inport: '<Root>/jawDesiredPosition'
   */
  rtY.controlJaw = 21.0 * rtU.jawDesiredPosition;

  /* Outport: '<Root>/controlWristRotation' incorporates:
   *  Gain: '<S1>/Gain5'
   *  Inport: '<Root>/gripperRotationDesiredPosition'
   */
  rtY.controlWristRotation = 21.0 * rtU.gripperRotationDesiredPosition;

  /* Product: '<S1>/Divide' incorporates:
   *  Constant: '<S1>/One'
   *  Gain: '<S1>/Gain2'
   *  Inport: '<Root>/deltaTime'
   *  Product: '<S1>/Divide3'
   */
  theta0 = 1.0 / (5.0 * rtU.deltaTime);

  /* Outport: '<Root>/stepperLeftFrequency' incorporates:
   *  Product: '<S1>/Divide'
   */
  rtY.stepperLeftFrequency = theta0;

  /* Outport: '<Root>/stepperRightFrequency' */
  rtY.stepperRightFrequency = theta0;
}

/* Model initialize function */
void control_arm_manual_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
