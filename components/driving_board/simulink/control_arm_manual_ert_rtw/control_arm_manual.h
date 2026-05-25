/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_arm_manual.h
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

#ifndef control_arm_manual_h_
#define control_arm_manual_h_
#ifndef control_arm_manual_COMMON_INCLUDES_
#define control_arm_manual_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                                 /* control_arm_manual_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T x;                            /* '<Root>/x' */
  real_T y;                            /* '<Root>/y' */
  real_T z;                            /* '<Root>/z' */
  real_T gripperAng;                   /* '<Root>/gripperAng' */
  real_T jawDesiredPosition;           /* '<Root>/jawDesiredPosition' */
  real_T gripperRotationDesiredPosition;
                                   /* '<Root>/gripperRotationDesiredPosition' */
  real_T actualposition;               /* '<Root>/jawActualPosition' */
  real_T gripperRotationActualPosition;
                                    /* '<Root>/gripperRotationActualPosition' */
  real_T gripperPitchActualPosition;   /* '<Root>/gripperPitchActualPosition' */
  real_T baseActualPosition;           /* '<Root>/baseActualPosition' */
  real_T stepperLeftActualPosition;    /* '<Root>/stepperLeftActualPosition' */
  real_T stepperRightActualPosition;   /* '<Root>/stepperRightActualPosition' */
  real_T deltaTime;                    /* '<Root>/deltaTime' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T error;                        /* '<Root>/error' */
  real_T controlJaw;                   /* '<Root>/controlJaw' */
  real_T controlWristRotation;         /* '<Root>/controlWristRotation' */
  real_T controlGripperPitch;          /* '<Root>/controlGripperPitch' */
  real_T controlBase;                  /* '<Root>/controlBase' */
  real_T stepperLeftSteps;             /* '<Root>/stepperLeftSteps' */
  real_T stepperLeftFrequency;         /* '<Root>/stepperLeftFrequency' */
  real_T stepperRightSteps;            /* '<Root>/stepperRightSteps' */
  real_T stepperRightFrequency;        /* '<Root>/stepperRightFrequency' */
  boolean_T pointReached;              /* '<Root>/pointReached' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Model entry point functions */
extern void control_arm_manual_initialize(void);
extern void control_arm_manual_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S1>/Gain19' : Unused code path elimination
 * Block '<S1>/Gain7' : Unused code path elimination
 * Block '<S1>/actual position' : Unused code path elimination
 * Block '<S1>/distance actual and  desired position' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('codegen_arm_manual/control_arm_manual')    - opens subsystem codegen_arm_manual/control_arm_manual
 * hilite_system('codegen_arm_manual/control_arm_manual/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'codegen_arm_manual'
 * '<S1>'   : 'codegen_arm_manual/control_arm_manual'
 * '<S2>'   : 'codegen_arm_manual/control_arm_manual/Compare To Constant3'
 * '<S3>'   : 'codegen_arm_manual/control_arm_manual/Distance'
 * '<S4>'   : 'codegen_arm_manual/control_arm_manual/MATLAB Function4'
 * '<S5>'   : 'codegen_arm_manual/control_arm_manual/inverse kinematics'
 */
#endif                                 /* control_arm_manual_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
