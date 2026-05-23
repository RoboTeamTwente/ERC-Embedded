/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_drive_manual.h
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

#ifndef control_drive_manual_h_
#define control_drive_manual_h_
#ifndef control_drive_manual_COMMON_INCLUDES_
#define control_drive_manual_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                               /* control_drive_manual_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  real_T UnitDelay2_DSTATE;            /* '<S1>/Unit Delay2' */
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T controllerSteering;           /* '<Root>/controllerSteering' */
  real_T controllerSpeed;              /* '<Root>/controllerSpeed' */
  real_T deltaTime;                    /* '<Root>/deltaTime' */
  real_T break_l;                      /* '<Root>/break' */
  real_T LFActualSpeed;                /* '<Root>/LFActualSpeed' */
  real_T LMActualSpeed;                /* '<Root>/LMActualSpeed' */
  real_T LBActualSpeed;                /* '<Root>/LBActualSpeed' */
  real_T RFActualSpeed;                /* '<Root>/RFActualSpeed' */
  real_T RMActualSpeed;                /* '<Root>/RMActualSpeed' */
  real_T RBActualSpeed;                /* '<Root>/RBActualSpeed' */
  real_T LFActualPos;                  /* '<Root>/LFActualPos' */
  real_T LMActualPos;                  /* '<Root>/LMActualPos' */
  real_T LBActualPos;                  /* '<Root>/LBActualPos' */
  real_T RFActualPos;                  /* '<Root>/RFActualPos' */
  real_T RMActualPos;                  /* '<Root>/RMActualPos' */
  real_T RBActualPos;                  /* '<Root>/RBActualPos' */
  real_T LFCurrent;                    /* '<Root>/LFCurrent' */
  real_T LMCurrent;                    /* '<Root>/LMCurrent' */
  real_T LBCurrent;                    /* '<Root>/LBCurrent' */
  real_T RFCurrent;                    /* '<Root>/RFCurrent' */
  real_T RMCurrent;                    /* '<Root>/RMCurrent' */
  real_T RBCurrent;                    /* '<Root>/RBCurrent' */
  real_T LFTemperature;                /* '<Root>/LFTemperature' */
  real_T LMTemperature;                /* '<Root>/LMTemperature' */
  real_T LBTemperature;                /* '<Root>/LBTemperature' */
  real_T RFTemperature;                /* '<Root>/RFTemperature' */
  real_T RMTemperature;                /* '<Root>/RMTemperature' */
  real_T RBTemperature;                /* '<Root>/RBTemperature' */
  real_T LFStatus;                     /* '<Root>/LFStatus' */
  real_T LMStatus;                     /* '<Root>/LMStatus' */
  real_T LBStatus;                     /* '<Root>/LBStatus' */
  real_T RFStatus;                     /* '<Root>/RFStatus' */
  real_T RMStatus;                     /* '<Root>/RMStatus' */
  real_T RBStatus;                     /* '<Root>/RBStatus' */
  real_T LFCanId;                      /* '<Root>/LFCanId' */
  real_T LMCanId;                      /* '<Root>/LMCanId' */
  real_T LBCanId;                      /* '<Root>/LBCanId' */
  real_T RFCanId;                      /* '<Root>/RFCanId' */
  real_T RMCanId;                      /* '<Root>/RMCanId' */
  real_T RBCanId;                      /* '<Root>/RBCanId' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T controlLF;                    /* '<Root>/controlLF' */
  real_T controlLM;                    /* '<Root>/controlLM' */
  real_T controlLB;                    /* '<Root>/controlLB' */
  real_T controlRF;                    /* '<Root>/controlRF' */
  real_T controlRM;                    /* '<Root>/controlRM' */
  real_T controlRB;                    /* '<Root>/controlRB' */
  real_T stepperLFSteps;               /* '<Root>/stepperLFSteps' */
  real_T stepperLFFrequency;           /* '<Root>/stepperLFFrequency' */
  real_T stepperLBSteps;               /* '<Root>/stepperLBSteps' */
  real_T stepperLBFrequency;           /* '<Root>/stepperLBFrequency' */
  real_T stepperRFSteps;               /* '<Root>/stepperRFSteps' */
  real_T stepperRFFrequency;           /* '<Root>/stepperRFFrequency' */
  real_T stepperRBSteps;               /* '<Root>/stepperRBSteps' */
  real_T stepperRBFrequency;           /* '<Root>/stepperRBFrequency' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Model entry point functions */
extern void control_drive_manual_initialize(void);
extern void control_drive_manual_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

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
 * hilite_system('codegen_drive_manual/control_drive_manual')    - opens subsystem codegen_drive_manual/control_drive_manual
 * hilite_system('codegen_drive_manual/control_drive_manual/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'codegen_drive_manual'
 * '<S1>'   : 'codegen_drive_manual/control_drive_manual'
 * '<S2>'   : 'codegen_drive_manual/control_drive_manual/MATLAB Function'
 * '<S3>'   : 'codegen_drive_manual/control_drive_manual/getting the steering angles and turing radii'
 * '<S4>'   : 'codegen_drive_manual/control_drive_manual/getting the wheel speeds from the turning radii'
 * '<S5>'   : 'codegen_drive_manual/control_drive_manual/setting the desired speed of the shassis'
 */
#endif                                 /* control_drive_manual_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
