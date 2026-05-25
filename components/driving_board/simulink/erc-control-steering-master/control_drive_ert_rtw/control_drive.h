/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control_drive.h
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

#ifndef control_drive_h_
#define control_drive_h_
#ifndef control_drive_COMMON_INCLUDES_
#define control_drive_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                                 /* control_drive_COMMON_INCLUDES_ */

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
  real_T UnitDelay_DSTATE;             /* '<S1>/Unit Delay' */
  real_T UnitDelay2_DSTATE;            /* '<S1>/Unit Delay2' */
  real_T UnitDelay4_DSTATE;            /* '<S1>/Unit Delay4' */
  real_T UnitDelay6_DSTATE;            /* '<S1>/Unit Delay6' */
  real_T UnitDelay8_DSTATE;            /* '<S1>/Unit Delay8' */
  real_T UnitDelay10_DSTATE;           /* '<S1>/Unit Delay10' */
  real_T UnitDelay1_DSTATE;            /* '<S1>/Unit Delay1' */
  real_T UnitDelay3_DSTATE;            /* '<S1>/Unit Delay3' */
  real_T Integrator_DSTATE;            /* '<S103>/Integrator' */
  real_T UnitDelay5_DSTATE;            /* '<S1>/Unit Delay5' */
  real_T Integrator_DSTATE_n;          /* '<S155>/Integrator' */
  real_T UnitDelay7_DSTATE;            /* '<S1>/Unit Delay7' */
  real_T Integrator_DSTATE_o;          /* '<S207>/Integrator' */
  real_T UnitDelay9_DSTATE;            /* '<S1>/Unit Delay9' */
  real_T Integrator_DSTATE_f;          /* '<S259>/Integrator' */
  real_T UnitDelay11_DSTATE;           /* '<S1>/Unit Delay11' */
  real_T Integrator_DSTATE_e;          /* '<S311>/Integrator' */
  real_T Integrator_DSTATE_p;          /* '<S51>/Integrator' */
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T R;                            /* '<Root>/R' */
  real_T dist2goal;                    /* '<Root>/dist2goal' */
  real_T LFActualSpeed;                /* '<Root>/LFActualSpeed' */
  real_T LMActualSpeed;                /* '<Root>/LMActualSpeed' */
  real_T LBActualSpeed;                /* '<Root>/LBActualSpeed' */
  real_T RFActualSpeed;                /* '<Root>/RFActualSpeed' */
  real_T RMActualSpeed;                /* '<Root>/RMActualSpeed' */
  real_T RBActualSpeed;                /* '<Root>/RBActualSpeed' */
  real_T deltaTime;                    /* '<Root>/deltaTime' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T controlLF;                    /* '<Root>/controlLF' */
  real_T controlLM;                    /* '<Root>/controlLM' */
  real_T controlLB;                    /* '<Root>/controlLB' */
  real_T controlRF;                    /* '<Root>/controlRF' */
  real_T controlRM;                    /* '<Root>/controlRM' */
  real_T controlRB;                    /* '<Root>/controlRB' */
  real_T FreqLF;                       /* '<Root>/FreqLF' */
  real_T StepsLF;                      /* '<Root>/StepsLF' */
  real_T FreqLB;                       /* '<Root>/FreqLB' */
  real_T StepsLB;                      /* '<Root>/StepsLB' */
  real_T FreqRF;                       /* '<Root>/FreqRF' */
  real_T StepsRF;                      /* '<Root>/StepsRF' */
  real_T FreqRB;                       /* '<Root>/FreqRB' */
  real_T StepsRB;                      /* '<Root>/StepsRB' */
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
extern void control_drive_initialize(void);
extern void control_drive_step(void);

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
 * hilite_system('codegen_drive/control_drive')    - opens subsystem codegen_drive/control_drive
 * hilite_system('codegen_drive/control_drive/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'codegen_drive'
 * '<S1>'   : 'codegen_drive/control_drive'
 * '<S2>'   : 'codegen_drive/control_drive/MATLAB Function1'
 * '<S3>'   : 'codegen_drive/control_drive/MATLAB Function2'
 * '<S4>'   : 'codegen_drive/control_drive/MATLAB Function3'
 * '<S5>'   : 'codegen_drive/control_drive/MATLAB Function4'
 * '<S6>'   : 'codegen_drive/control_drive/MATLAB Function5'
 * '<S7>'   : 'codegen_drive/control_drive/MATLAB Function6'
 * '<S8>'   : 'codegen_drive/control_drive/PID Controller1'
 * '<S9>'   : 'codegen_drive/control_drive/PID Controller2'
 * '<S10>'  : 'codegen_drive/control_drive/PID Controller3'
 * '<S11>'  : 'codegen_drive/control_drive/PID Controller4'
 * '<S12>'  : 'codegen_drive/control_drive/PID Controller5'
 * '<S13>'  : 'codegen_drive/control_drive/PID Controller6'
 * '<S14>'  : 'codegen_drive/control_drive/getting the steering angles and turing radii'
 * '<S15>'  : 'codegen_drive/control_drive/getting the wheel speeds from the turning radii'
 * '<S16>'  : 'codegen_drive/control_drive/setting desired speed dependant  on the distance to the goal'
 * '<S17>'  : 'codegen_drive/control_drive/PID Controller1/Anti-windup'
 * '<S18>'  : 'codegen_drive/control_drive/PID Controller1/D Gain'
 * '<S19>'  : 'codegen_drive/control_drive/PID Controller1/External Derivative'
 * '<S20>'  : 'codegen_drive/control_drive/PID Controller1/Filter'
 * '<S21>'  : 'codegen_drive/control_drive/PID Controller1/Filter ICs'
 * '<S22>'  : 'codegen_drive/control_drive/PID Controller1/I Gain'
 * '<S23>'  : 'codegen_drive/control_drive/PID Controller1/Ideal P Gain'
 * '<S24>'  : 'codegen_drive/control_drive/PID Controller1/Ideal P Gain Fdbk'
 * '<S25>'  : 'codegen_drive/control_drive/PID Controller1/Integrator'
 * '<S26>'  : 'codegen_drive/control_drive/PID Controller1/Integrator ICs'
 * '<S27>'  : 'codegen_drive/control_drive/PID Controller1/N Copy'
 * '<S28>'  : 'codegen_drive/control_drive/PID Controller1/N Gain'
 * '<S29>'  : 'codegen_drive/control_drive/PID Controller1/P Copy'
 * '<S30>'  : 'codegen_drive/control_drive/PID Controller1/Parallel P Gain'
 * '<S31>'  : 'codegen_drive/control_drive/PID Controller1/Reset Signal'
 * '<S32>'  : 'codegen_drive/control_drive/PID Controller1/Saturation'
 * '<S33>'  : 'codegen_drive/control_drive/PID Controller1/Saturation Fdbk'
 * '<S34>'  : 'codegen_drive/control_drive/PID Controller1/Sum'
 * '<S35>'  : 'codegen_drive/control_drive/PID Controller1/Sum Fdbk'
 * '<S36>'  : 'codegen_drive/control_drive/PID Controller1/Tracking Mode'
 * '<S37>'  : 'codegen_drive/control_drive/PID Controller1/Tracking Mode Sum'
 * '<S38>'  : 'codegen_drive/control_drive/PID Controller1/Tsamp - Integral'
 * '<S39>'  : 'codegen_drive/control_drive/PID Controller1/Tsamp - Ngain'
 * '<S40>'  : 'codegen_drive/control_drive/PID Controller1/postSat Signal'
 * '<S41>'  : 'codegen_drive/control_drive/PID Controller1/preInt Signal'
 * '<S42>'  : 'codegen_drive/control_drive/PID Controller1/preSat Signal'
 * '<S43>'  : 'codegen_drive/control_drive/PID Controller1/Anti-windup/Passthrough'
 * '<S44>'  : 'codegen_drive/control_drive/PID Controller1/D Gain/Disabled'
 * '<S45>'  : 'codegen_drive/control_drive/PID Controller1/External Derivative/Disabled'
 * '<S46>'  : 'codegen_drive/control_drive/PID Controller1/Filter/Disabled'
 * '<S47>'  : 'codegen_drive/control_drive/PID Controller1/Filter ICs/Disabled'
 * '<S48>'  : 'codegen_drive/control_drive/PID Controller1/I Gain/Internal Parameters'
 * '<S49>'  : 'codegen_drive/control_drive/PID Controller1/Ideal P Gain/Passthrough'
 * '<S50>'  : 'codegen_drive/control_drive/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S51>'  : 'codegen_drive/control_drive/PID Controller1/Integrator/Discrete'
 * '<S52>'  : 'codegen_drive/control_drive/PID Controller1/Integrator ICs/Internal IC'
 * '<S53>'  : 'codegen_drive/control_drive/PID Controller1/N Copy/Disabled wSignal Specification'
 * '<S54>'  : 'codegen_drive/control_drive/PID Controller1/N Gain/Disabled'
 * '<S55>'  : 'codegen_drive/control_drive/PID Controller1/P Copy/Disabled'
 * '<S56>'  : 'codegen_drive/control_drive/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S57>'  : 'codegen_drive/control_drive/PID Controller1/Reset Signal/Disabled'
 * '<S58>'  : 'codegen_drive/control_drive/PID Controller1/Saturation/Passthrough'
 * '<S59>'  : 'codegen_drive/control_drive/PID Controller1/Saturation Fdbk/Disabled'
 * '<S60>'  : 'codegen_drive/control_drive/PID Controller1/Sum/Sum_PI'
 * '<S61>'  : 'codegen_drive/control_drive/PID Controller1/Sum Fdbk/Disabled'
 * '<S62>'  : 'codegen_drive/control_drive/PID Controller1/Tracking Mode/Disabled'
 * '<S63>'  : 'codegen_drive/control_drive/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S64>'  : 'codegen_drive/control_drive/PID Controller1/Tsamp - Integral/External Ts'
 * '<S65>'  : 'codegen_drive/control_drive/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S66>'  : 'codegen_drive/control_drive/PID Controller1/postSat Signal/Forward_Path'
 * '<S67>'  : 'codegen_drive/control_drive/PID Controller1/preInt Signal/Internal PreInt'
 * '<S68>'  : 'codegen_drive/control_drive/PID Controller1/preSat Signal/Forward_Path'
 * '<S69>'  : 'codegen_drive/control_drive/PID Controller2/Anti-windup'
 * '<S70>'  : 'codegen_drive/control_drive/PID Controller2/D Gain'
 * '<S71>'  : 'codegen_drive/control_drive/PID Controller2/External Derivative'
 * '<S72>'  : 'codegen_drive/control_drive/PID Controller2/Filter'
 * '<S73>'  : 'codegen_drive/control_drive/PID Controller2/Filter ICs'
 * '<S74>'  : 'codegen_drive/control_drive/PID Controller2/I Gain'
 * '<S75>'  : 'codegen_drive/control_drive/PID Controller2/Ideal P Gain'
 * '<S76>'  : 'codegen_drive/control_drive/PID Controller2/Ideal P Gain Fdbk'
 * '<S77>'  : 'codegen_drive/control_drive/PID Controller2/Integrator'
 * '<S78>'  : 'codegen_drive/control_drive/PID Controller2/Integrator ICs'
 * '<S79>'  : 'codegen_drive/control_drive/PID Controller2/N Copy'
 * '<S80>'  : 'codegen_drive/control_drive/PID Controller2/N Gain'
 * '<S81>'  : 'codegen_drive/control_drive/PID Controller2/P Copy'
 * '<S82>'  : 'codegen_drive/control_drive/PID Controller2/Parallel P Gain'
 * '<S83>'  : 'codegen_drive/control_drive/PID Controller2/Reset Signal'
 * '<S84>'  : 'codegen_drive/control_drive/PID Controller2/Saturation'
 * '<S85>'  : 'codegen_drive/control_drive/PID Controller2/Saturation Fdbk'
 * '<S86>'  : 'codegen_drive/control_drive/PID Controller2/Sum'
 * '<S87>'  : 'codegen_drive/control_drive/PID Controller2/Sum Fdbk'
 * '<S88>'  : 'codegen_drive/control_drive/PID Controller2/Tracking Mode'
 * '<S89>'  : 'codegen_drive/control_drive/PID Controller2/Tracking Mode Sum'
 * '<S90>'  : 'codegen_drive/control_drive/PID Controller2/Tsamp - Integral'
 * '<S91>'  : 'codegen_drive/control_drive/PID Controller2/Tsamp - Ngain'
 * '<S92>'  : 'codegen_drive/control_drive/PID Controller2/postSat Signal'
 * '<S93>'  : 'codegen_drive/control_drive/PID Controller2/preInt Signal'
 * '<S94>'  : 'codegen_drive/control_drive/PID Controller2/preSat Signal'
 * '<S95>'  : 'codegen_drive/control_drive/PID Controller2/Anti-windup/Passthrough'
 * '<S96>'  : 'codegen_drive/control_drive/PID Controller2/D Gain/Disabled'
 * '<S97>'  : 'codegen_drive/control_drive/PID Controller2/External Derivative/Disabled'
 * '<S98>'  : 'codegen_drive/control_drive/PID Controller2/Filter/Disabled'
 * '<S99>'  : 'codegen_drive/control_drive/PID Controller2/Filter ICs/Disabled'
 * '<S100>' : 'codegen_drive/control_drive/PID Controller2/I Gain/Internal Parameters'
 * '<S101>' : 'codegen_drive/control_drive/PID Controller2/Ideal P Gain/Passthrough'
 * '<S102>' : 'codegen_drive/control_drive/PID Controller2/Ideal P Gain Fdbk/Disabled'
 * '<S103>' : 'codegen_drive/control_drive/PID Controller2/Integrator/Discrete'
 * '<S104>' : 'codegen_drive/control_drive/PID Controller2/Integrator ICs/Internal IC'
 * '<S105>' : 'codegen_drive/control_drive/PID Controller2/N Copy/Disabled wSignal Specification'
 * '<S106>' : 'codegen_drive/control_drive/PID Controller2/N Gain/Disabled'
 * '<S107>' : 'codegen_drive/control_drive/PID Controller2/P Copy/Disabled'
 * '<S108>' : 'codegen_drive/control_drive/PID Controller2/Parallel P Gain/Internal Parameters'
 * '<S109>' : 'codegen_drive/control_drive/PID Controller2/Reset Signal/Disabled'
 * '<S110>' : 'codegen_drive/control_drive/PID Controller2/Saturation/Passthrough'
 * '<S111>' : 'codegen_drive/control_drive/PID Controller2/Saturation Fdbk/Disabled'
 * '<S112>' : 'codegen_drive/control_drive/PID Controller2/Sum/Sum_PI'
 * '<S113>' : 'codegen_drive/control_drive/PID Controller2/Sum Fdbk/Disabled'
 * '<S114>' : 'codegen_drive/control_drive/PID Controller2/Tracking Mode/Disabled'
 * '<S115>' : 'codegen_drive/control_drive/PID Controller2/Tracking Mode Sum/Passthrough'
 * '<S116>' : 'codegen_drive/control_drive/PID Controller2/Tsamp - Integral/External Ts'
 * '<S117>' : 'codegen_drive/control_drive/PID Controller2/Tsamp - Ngain/Passthrough'
 * '<S118>' : 'codegen_drive/control_drive/PID Controller2/postSat Signal/Forward_Path'
 * '<S119>' : 'codegen_drive/control_drive/PID Controller2/preInt Signal/Internal PreInt'
 * '<S120>' : 'codegen_drive/control_drive/PID Controller2/preSat Signal/Forward_Path'
 * '<S121>' : 'codegen_drive/control_drive/PID Controller3/Anti-windup'
 * '<S122>' : 'codegen_drive/control_drive/PID Controller3/D Gain'
 * '<S123>' : 'codegen_drive/control_drive/PID Controller3/External Derivative'
 * '<S124>' : 'codegen_drive/control_drive/PID Controller3/Filter'
 * '<S125>' : 'codegen_drive/control_drive/PID Controller3/Filter ICs'
 * '<S126>' : 'codegen_drive/control_drive/PID Controller3/I Gain'
 * '<S127>' : 'codegen_drive/control_drive/PID Controller3/Ideal P Gain'
 * '<S128>' : 'codegen_drive/control_drive/PID Controller3/Ideal P Gain Fdbk'
 * '<S129>' : 'codegen_drive/control_drive/PID Controller3/Integrator'
 * '<S130>' : 'codegen_drive/control_drive/PID Controller3/Integrator ICs'
 * '<S131>' : 'codegen_drive/control_drive/PID Controller3/N Copy'
 * '<S132>' : 'codegen_drive/control_drive/PID Controller3/N Gain'
 * '<S133>' : 'codegen_drive/control_drive/PID Controller3/P Copy'
 * '<S134>' : 'codegen_drive/control_drive/PID Controller3/Parallel P Gain'
 * '<S135>' : 'codegen_drive/control_drive/PID Controller3/Reset Signal'
 * '<S136>' : 'codegen_drive/control_drive/PID Controller3/Saturation'
 * '<S137>' : 'codegen_drive/control_drive/PID Controller3/Saturation Fdbk'
 * '<S138>' : 'codegen_drive/control_drive/PID Controller3/Sum'
 * '<S139>' : 'codegen_drive/control_drive/PID Controller3/Sum Fdbk'
 * '<S140>' : 'codegen_drive/control_drive/PID Controller3/Tracking Mode'
 * '<S141>' : 'codegen_drive/control_drive/PID Controller3/Tracking Mode Sum'
 * '<S142>' : 'codegen_drive/control_drive/PID Controller3/Tsamp - Integral'
 * '<S143>' : 'codegen_drive/control_drive/PID Controller3/Tsamp - Ngain'
 * '<S144>' : 'codegen_drive/control_drive/PID Controller3/postSat Signal'
 * '<S145>' : 'codegen_drive/control_drive/PID Controller3/preInt Signal'
 * '<S146>' : 'codegen_drive/control_drive/PID Controller3/preSat Signal'
 * '<S147>' : 'codegen_drive/control_drive/PID Controller3/Anti-windup/Passthrough'
 * '<S148>' : 'codegen_drive/control_drive/PID Controller3/D Gain/Disabled'
 * '<S149>' : 'codegen_drive/control_drive/PID Controller3/External Derivative/Disabled'
 * '<S150>' : 'codegen_drive/control_drive/PID Controller3/Filter/Disabled'
 * '<S151>' : 'codegen_drive/control_drive/PID Controller3/Filter ICs/Disabled'
 * '<S152>' : 'codegen_drive/control_drive/PID Controller3/I Gain/Internal Parameters'
 * '<S153>' : 'codegen_drive/control_drive/PID Controller3/Ideal P Gain/Passthrough'
 * '<S154>' : 'codegen_drive/control_drive/PID Controller3/Ideal P Gain Fdbk/Disabled'
 * '<S155>' : 'codegen_drive/control_drive/PID Controller3/Integrator/Discrete'
 * '<S156>' : 'codegen_drive/control_drive/PID Controller3/Integrator ICs/Internal IC'
 * '<S157>' : 'codegen_drive/control_drive/PID Controller3/N Copy/Disabled wSignal Specification'
 * '<S158>' : 'codegen_drive/control_drive/PID Controller3/N Gain/Disabled'
 * '<S159>' : 'codegen_drive/control_drive/PID Controller3/P Copy/Disabled'
 * '<S160>' : 'codegen_drive/control_drive/PID Controller3/Parallel P Gain/Internal Parameters'
 * '<S161>' : 'codegen_drive/control_drive/PID Controller3/Reset Signal/Disabled'
 * '<S162>' : 'codegen_drive/control_drive/PID Controller3/Saturation/Passthrough'
 * '<S163>' : 'codegen_drive/control_drive/PID Controller3/Saturation Fdbk/Disabled'
 * '<S164>' : 'codegen_drive/control_drive/PID Controller3/Sum/Sum_PI'
 * '<S165>' : 'codegen_drive/control_drive/PID Controller3/Sum Fdbk/Disabled'
 * '<S166>' : 'codegen_drive/control_drive/PID Controller3/Tracking Mode/Disabled'
 * '<S167>' : 'codegen_drive/control_drive/PID Controller3/Tracking Mode Sum/Passthrough'
 * '<S168>' : 'codegen_drive/control_drive/PID Controller3/Tsamp - Integral/External Ts'
 * '<S169>' : 'codegen_drive/control_drive/PID Controller3/Tsamp - Ngain/Passthrough'
 * '<S170>' : 'codegen_drive/control_drive/PID Controller3/postSat Signal/Forward_Path'
 * '<S171>' : 'codegen_drive/control_drive/PID Controller3/preInt Signal/Internal PreInt'
 * '<S172>' : 'codegen_drive/control_drive/PID Controller3/preSat Signal/Forward_Path'
 * '<S173>' : 'codegen_drive/control_drive/PID Controller4/Anti-windup'
 * '<S174>' : 'codegen_drive/control_drive/PID Controller4/D Gain'
 * '<S175>' : 'codegen_drive/control_drive/PID Controller4/External Derivative'
 * '<S176>' : 'codegen_drive/control_drive/PID Controller4/Filter'
 * '<S177>' : 'codegen_drive/control_drive/PID Controller4/Filter ICs'
 * '<S178>' : 'codegen_drive/control_drive/PID Controller4/I Gain'
 * '<S179>' : 'codegen_drive/control_drive/PID Controller4/Ideal P Gain'
 * '<S180>' : 'codegen_drive/control_drive/PID Controller4/Ideal P Gain Fdbk'
 * '<S181>' : 'codegen_drive/control_drive/PID Controller4/Integrator'
 * '<S182>' : 'codegen_drive/control_drive/PID Controller4/Integrator ICs'
 * '<S183>' : 'codegen_drive/control_drive/PID Controller4/N Copy'
 * '<S184>' : 'codegen_drive/control_drive/PID Controller4/N Gain'
 * '<S185>' : 'codegen_drive/control_drive/PID Controller4/P Copy'
 * '<S186>' : 'codegen_drive/control_drive/PID Controller4/Parallel P Gain'
 * '<S187>' : 'codegen_drive/control_drive/PID Controller4/Reset Signal'
 * '<S188>' : 'codegen_drive/control_drive/PID Controller4/Saturation'
 * '<S189>' : 'codegen_drive/control_drive/PID Controller4/Saturation Fdbk'
 * '<S190>' : 'codegen_drive/control_drive/PID Controller4/Sum'
 * '<S191>' : 'codegen_drive/control_drive/PID Controller4/Sum Fdbk'
 * '<S192>' : 'codegen_drive/control_drive/PID Controller4/Tracking Mode'
 * '<S193>' : 'codegen_drive/control_drive/PID Controller4/Tracking Mode Sum'
 * '<S194>' : 'codegen_drive/control_drive/PID Controller4/Tsamp - Integral'
 * '<S195>' : 'codegen_drive/control_drive/PID Controller4/Tsamp - Ngain'
 * '<S196>' : 'codegen_drive/control_drive/PID Controller4/postSat Signal'
 * '<S197>' : 'codegen_drive/control_drive/PID Controller4/preInt Signal'
 * '<S198>' : 'codegen_drive/control_drive/PID Controller4/preSat Signal'
 * '<S199>' : 'codegen_drive/control_drive/PID Controller4/Anti-windup/Passthrough'
 * '<S200>' : 'codegen_drive/control_drive/PID Controller4/D Gain/Disabled'
 * '<S201>' : 'codegen_drive/control_drive/PID Controller4/External Derivative/Disabled'
 * '<S202>' : 'codegen_drive/control_drive/PID Controller4/Filter/Disabled'
 * '<S203>' : 'codegen_drive/control_drive/PID Controller4/Filter ICs/Disabled'
 * '<S204>' : 'codegen_drive/control_drive/PID Controller4/I Gain/Internal Parameters'
 * '<S205>' : 'codegen_drive/control_drive/PID Controller4/Ideal P Gain/Passthrough'
 * '<S206>' : 'codegen_drive/control_drive/PID Controller4/Ideal P Gain Fdbk/Disabled'
 * '<S207>' : 'codegen_drive/control_drive/PID Controller4/Integrator/Discrete'
 * '<S208>' : 'codegen_drive/control_drive/PID Controller4/Integrator ICs/Internal IC'
 * '<S209>' : 'codegen_drive/control_drive/PID Controller4/N Copy/Disabled wSignal Specification'
 * '<S210>' : 'codegen_drive/control_drive/PID Controller4/N Gain/Disabled'
 * '<S211>' : 'codegen_drive/control_drive/PID Controller4/P Copy/Disabled'
 * '<S212>' : 'codegen_drive/control_drive/PID Controller4/Parallel P Gain/Internal Parameters'
 * '<S213>' : 'codegen_drive/control_drive/PID Controller4/Reset Signal/Disabled'
 * '<S214>' : 'codegen_drive/control_drive/PID Controller4/Saturation/Passthrough'
 * '<S215>' : 'codegen_drive/control_drive/PID Controller4/Saturation Fdbk/Disabled'
 * '<S216>' : 'codegen_drive/control_drive/PID Controller4/Sum/Sum_PI'
 * '<S217>' : 'codegen_drive/control_drive/PID Controller4/Sum Fdbk/Disabled'
 * '<S218>' : 'codegen_drive/control_drive/PID Controller4/Tracking Mode/Disabled'
 * '<S219>' : 'codegen_drive/control_drive/PID Controller4/Tracking Mode Sum/Passthrough'
 * '<S220>' : 'codegen_drive/control_drive/PID Controller4/Tsamp - Integral/External Ts'
 * '<S221>' : 'codegen_drive/control_drive/PID Controller4/Tsamp - Ngain/Passthrough'
 * '<S222>' : 'codegen_drive/control_drive/PID Controller4/postSat Signal/Forward_Path'
 * '<S223>' : 'codegen_drive/control_drive/PID Controller4/preInt Signal/Internal PreInt'
 * '<S224>' : 'codegen_drive/control_drive/PID Controller4/preSat Signal/Forward_Path'
 * '<S225>' : 'codegen_drive/control_drive/PID Controller5/Anti-windup'
 * '<S226>' : 'codegen_drive/control_drive/PID Controller5/D Gain'
 * '<S227>' : 'codegen_drive/control_drive/PID Controller5/External Derivative'
 * '<S228>' : 'codegen_drive/control_drive/PID Controller5/Filter'
 * '<S229>' : 'codegen_drive/control_drive/PID Controller5/Filter ICs'
 * '<S230>' : 'codegen_drive/control_drive/PID Controller5/I Gain'
 * '<S231>' : 'codegen_drive/control_drive/PID Controller5/Ideal P Gain'
 * '<S232>' : 'codegen_drive/control_drive/PID Controller5/Ideal P Gain Fdbk'
 * '<S233>' : 'codegen_drive/control_drive/PID Controller5/Integrator'
 * '<S234>' : 'codegen_drive/control_drive/PID Controller5/Integrator ICs'
 * '<S235>' : 'codegen_drive/control_drive/PID Controller5/N Copy'
 * '<S236>' : 'codegen_drive/control_drive/PID Controller5/N Gain'
 * '<S237>' : 'codegen_drive/control_drive/PID Controller5/P Copy'
 * '<S238>' : 'codegen_drive/control_drive/PID Controller5/Parallel P Gain'
 * '<S239>' : 'codegen_drive/control_drive/PID Controller5/Reset Signal'
 * '<S240>' : 'codegen_drive/control_drive/PID Controller5/Saturation'
 * '<S241>' : 'codegen_drive/control_drive/PID Controller5/Saturation Fdbk'
 * '<S242>' : 'codegen_drive/control_drive/PID Controller5/Sum'
 * '<S243>' : 'codegen_drive/control_drive/PID Controller5/Sum Fdbk'
 * '<S244>' : 'codegen_drive/control_drive/PID Controller5/Tracking Mode'
 * '<S245>' : 'codegen_drive/control_drive/PID Controller5/Tracking Mode Sum'
 * '<S246>' : 'codegen_drive/control_drive/PID Controller5/Tsamp - Integral'
 * '<S247>' : 'codegen_drive/control_drive/PID Controller5/Tsamp - Ngain'
 * '<S248>' : 'codegen_drive/control_drive/PID Controller5/postSat Signal'
 * '<S249>' : 'codegen_drive/control_drive/PID Controller5/preInt Signal'
 * '<S250>' : 'codegen_drive/control_drive/PID Controller5/preSat Signal'
 * '<S251>' : 'codegen_drive/control_drive/PID Controller5/Anti-windup/Passthrough'
 * '<S252>' : 'codegen_drive/control_drive/PID Controller5/D Gain/Disabled'
 * '<S253>' : 'codegen_drive/control_drive/PID Controller5/External Derivative/Disabled'
 * '<S254>' : 'codegen_drive/control_drive/PID Controller5/Filter/Disabled'
 * '<S255>' : 'codegen_drive/control_drive/PID Controller5/Filter ICs/Disabled'
 * '<S256>' : 'codegen_drive/control_drive/PID Controller5/I Gain/Internal Parameters'
 * '<S257>' : 'codegen_drive/control_drive/PID Controller5/Ideal P Gain/Passthrough'
 * '<S258>' : 'codegen_drive/control_drive/PID Controller5/Ideal P Gain Fdbk/Disabled'
 * '<S259>' : 'codegen_drive/control_drive/PID Controller5/Integrator/Discrete'
 * '<S260>' : 'codegen_drive/control_drive/PID Controller5/Integrator ICs/Internal IC'
 * '<S261>' : 'codegen_drive/control_drive/PID Controller5/N Copy/Disabled wSignal Specification'
 * '<S262>' : 'codegen_drive/control_drive/PID Controller5/N Gain/Disabled'
 * '<S263>' : 'codegen_drive/control_drive/PID Controller5/P Copy/Disabled'
 * '<S264>' : 'codegen_drive/control_drive/PID Controller5/Parallel P Gain/Internal Parameters'
 * '<S265>' : 'codegen_drive/control_drive/PID Controller5/Reset Signal/Disabled'
 * '<S266>' : 'codegen_drive/control_drive/PID Controller5/Saturation/Passthrough'
 * '<S267>' : 'codegen_drive/control_drive/PID Controller5/Saturation Fdbk/Disabled'
 * '<S268>' : 'codegen_drive/control_drive/PID Controller5/Sum/Sum_PI'
 * '<S269>' : 'codegen_drive/control_drive/PID Controller5/Sum Fdbk/Disabled'
 * '<S270>' : 'codegen_drive/control_drive/PID Controller5/Tracking Mode/Disabled'
 * '<S271>' : 'codegen_drive/control_drive/PID Controller5/Tracking Mode Sum/Passthrough'
 * '<S272>' : 'codegen_drive/control_drive/PID Controller5/Tsamp - Integral/External Ts'
 * '<S273>' : 'codegen_drive/control_drive/PID Controller5/Tsamp - Ngain/Passthrough'
 * '<S274>' : 'codegen_drive/control_drive/PID Controller5/postSat Signal/Forward_Path'
 * '<S275>' : 'codegen_drive/control_drive/PID Controller5/preInt Signal/Internal PreInt'
 * '<S276>' : 'codegen_drive/control_drive/PID Controller5/preSat Signal/Forward_Path'
 * '<S277>' : 'codegen_drive/control_drive/PID Controller6/Anti-windup'
 * '<S278>' : 'codegen_drive/control_drive/PID Controller6/D Gain'
 * '<S279>' : 'codegen_drive/control_drive/PID Controller6/External Derivative'
 * '<S280>' : 'codegen_drive/control_drive/PID Controller6/Filter'
 * '<S281>' : 'codegen_drive/control_drive/PID Controller6/Filter ICs'
 * '<S282>' : 'codegen_drive/control_drive/PID Controller6/I Gain'
 * '<S283>' : 'codegen_drive/control_drive/PID Controller6/Ideal P Gain'
 * '<S284>' : 'codegen_drive/control_drive/PID Controller6/Ideal P Gain Fdbk'
 * '<S285>' : 'codegen_drive/control_drive/PID Controller6/Integrator'
 * '<S286>' : 'codegen_drive/control_drive/PID Controller6/Integrator ICs'
 * '<S287>' : 'codegen_drive/control_drive/PID Controller6/N Copy'
 * '<S288>' : 'codegen_drive/control_drive/PID Controller6/N Gain'
 * '<S289>' : 'codegen_drive/control_drive/PID Controller6/P Copy'
 * '<S290>' : 'codegen_drive/control_drive/PID Controller6/Parallel P Gain'
 * '<S291>' : 'codegen_drive/control_drive/PID Controller6/Reset Signal'
 * '<S292>' : 'codegen_drive/control_drive/PID Controller6/Saturation'
 * '<S293>' : 'codegen_drive/control_drive/PID Controller6/Saturation Fdbk'
 * '<S294>' : 'codegen_drive/control_drive/PID Controller6/Sum'
 * '<S295>' : 'codegen_drive/control_drive/PID Controller6/Sum Fdbk'
 * '<S296>' : 'codegen_drive/control_drive/PID Controller6/Tracking Mode'
 * '<S297>' : 'codegen_drive/control_drive/PID Controller6/Tracking Mode Sum'
 * '<S298>' : 'codegen_drive/control_drive/PID Controller6/Tsamp - Integral'
 * '<S299>' : 'codegen_drive/control_drive/PID Controller6/Tsamp - Ngain'
 * '<S300>' : 'codegen_drive/control_drive/PID Controller6/postSat Signal'
 * '<S301>' : 'codegen_drive/control_drive/PID Controller6/preInt Signal'
 * '<S302>' : 'codegen_drive/control_drive/PID Controller6/preSat Signal'
 * '<S303>' : 'codegen_drive/control_drive/PID Controller6/Anti-windup/Passthrough'
 * '<S304>' : 'codegen_drive/control_drive/PID Controller6/D Gain/Disabled'
 * '<S305>' : 'codegen_drive/control_drive/PID Controller6/External Derivative/Disabled'
 * '<S306>' : 'codegen_drive/control_drive/PID Controller6/Filter/Disabled'
 * '<S307>' : 'codegen_drive/control_drive/PID Controller6/Filter ICs/Disabled'
 * '<S308>' : 'codegen_drive/control_drive/PID Controller6/I Gain/Internal Parameters'
 * '<S309>' : 'codegen_drive/control_drive/PID Controller6/Ideal P Gain/Passthrough'
 * '<S310>' : 'codegen_drive/control_drive/PID Controller6/Ideal P Gain Fdbk/Disabled'
 * '<S311>' : 'codegen_drive/control_drive/PID Controller6/Integrator/Discrete'
 * '<S312>' : 'codegen_drive/control_drive/PID Controller6/Integrator ICs/Internal IC'
 * '<S313>' : 'codegen_drive/control_drive/PID Controller6/N Copy/Disabled wSignal Specification'
 * '<S314>' : 'codegen_drive/control_drive/PID Controller6/N Gain/Disabled'
 * '<S315>' : 'codegen_drive/control_drive/PID Controller6/P Copy/Disabled'
 * '<S316>' : 'codegen_drive/control_drive/PID Controller6/Parallel P Gain/Internal Parameters'
 * '<S317>' : 'codegen_drive/control_drive/PID Controller6/Reset Signal/Disabled'
 * '<S318>' : 'codegen_drive/control_drive/PID Controller6/Saturation/Passthrough'
 * '<S319>' : 'codegen_drive/control_drive/PID Controller6/Saturation Fdbk/Disabled'
 * '<S320>' : 'codegen_drive/control_drive/PID Controller6/Sum/Sum_PI'
 * '<S321>' : 'codegen_drive/control_drive/PID Controller6/Sum Fdbk/Disabled'
 * '<S322>' : 'codegen_drive/control_drive/PID Controller6/Tracking Mode/Disabled'
 * '<S323>' : 'codegen_drive/control_drive/PID Controller6/Tracking Mode Sum/Passthrough'
 * '<S324>' : 'codegen_drive/control_drive/PID Controller6/Tsamp - Integral/External Ts'
 * '<S325>' : 'codegen_drive/control_drive/PID Controller6/Tsamp - Ngain/Passthrough'
 * '<S326>' : 'codegen_drive/control_drive/PID Controller6/postSat Signal/Forward_Path'
 * '<S327>' : 'codegen_drive/control_drive/PID Controller6/preInt Signal/Internal PreInt'
 * '<S328>' : 'codegen_drive/control_drive/PID Controller6/preSat Signal/Forward_Path'
 */
#endif                                 /* control_drive_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
