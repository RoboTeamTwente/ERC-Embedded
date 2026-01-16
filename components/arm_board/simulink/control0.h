/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: control0.h
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

#ifndef control0_h_
#define control0_h_
#ifndef control0_COMMON_INCLUDES_
#define control0_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#endif                                 /* control0_COMMON_INCLUDES_ */

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

/* Forward declaration for rtModel */
typedef struct tag_RTM RT_MODEL;

/* Block signals and states (default storage) for system '<S1>/ramp' */
typedef struct {
  real_T v;                            /* '<S1>/ramp' */
} DW_ramp;

/* Block signals and states (default storage) for system '<Root>' */
typedef struct {
  DW_ramp sf_ramp3;                    /* '<S1>/ramp3' */
  DW_ramp sf_ramp2;                    /* '<S1>/ramp2' */
  DW_ramp sf_ramp1;                    /* '<S1>/ramp1' */
  DW_ramp sf_ramp;                     /* '<S1>/ramp' */
  real_T UnitDelay8_DSTATE;            /* '<S1>/Unit Delay8' */
  real_T UnitDelay6_DSTATE;            /* '<S1>/Unit Delay6' */
  real_T UnitDelay9_DSTATE;            /* '<S1>/Unit Delay9' */
  real_T Integrator_DSTATE;            /* '<S202>/Integrator' */
  real_T UnitDelay4_DSTATE;            /* '<S1>/Unit Delay4' */
  real_T UnitDelay1_DSTATE;            /* '<S1>/Unit Delay1' */
  real_T UnitDelay7_DSTATE;            /* '<S1>/Unit Delay7' */
  real_T Integrator_DSTATE_k;          /* '<S150>/Integrator' */
  real_T UnitDelay_DSTATE;             /* '<S1>/Unit Delay' */
  real_T UnitDelay5_DSTATE;            /* '<S1>/Unit Delay5' */
  real_T Integrator_DSTATE_l;          /* '<S98>/Integrator' */
  real_T UnitDelay2_DSTATE;            /* '<S1>/Unit Delay2' */
  real_T Integrator_DSTATE_b;          /* '<S46>/Integrator' */
  real_T UnitDelay3_DSTATE;            /* '<S1>/Unit Delay3' */
  int32_T clockTickCounter;            /* '<S1>/Pulse Generator' */
  int32_T clockTickCounter_h;          /* '<S1>/Pulse Generator1' */
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T x;                            /* '<Root>/x' */
  real_T y;                            /* '<Root>/y' */
  real_T z;                            /* '<Root>/z' */
  real_T thetaX;                       /* '<Root>/thetaX' */
  real_T thetaY;                       /* '<Root>/thetaY' */
  real_T thetaZ;                       /* '<Root>/thetaZ' */
  real_T jawActualPosition;            /* '<Root>/jawActualPosition' */
  real_T gripperRotationActualPosition;
                                    /* '<Root>/gripperRotationActualPosition' */
  real_T gripperPitchActualPosition;   /* '<Root>/gripperPitchActualPosition' */
  real_T baseActualPosition;           /* '<Root>/baseActualPosition' */
  real_T stepperTopActualPosition;     /* '<Root>/stepperTopActualPosition' */
  real_T stepperBottomActualPosition; /* '<Root>/stepperBottomActualPosition' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T pointReached;                 /* '<Root>/pointReached' */
  real_T pointInRange;                 /* '<Root>/pointInRange' */
  real_T controlJaw1;                  /* '<Root>/controlJaw1' */
  real_T controlWristRotation;         /* '<Root>/controlWristRotation' */
  real_T controlGripperPitch;          /* '<Root>/controlGripperPitch' */
  real_T controlBase;                  /* '<Root>/controlBase' */
  real_T stepperTopENA;                /* '<Root>/stepperTopENA' */
  real_T stepperTopREV;                /* '<Root>/stepperTopREV' */
  real_T stepperBottomENA;             /* '<Root>/stepperBottomENA' */
  real_T stepperBottomREV;             /* '<Root>/stepperBottomREV' */
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
extern void control0_initialize(void);
extern void control0_step(void);

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
 * hilite_system('codegen/control')    - opens subsystem codegen/control
 * hilite_system('codegen/control/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'codegen'
 * '<S1>'   : 'codegen/control'
 * '<S2>'   : 'codegen/control/Compare To Constant'
 * '<S3>'   : 'codegen/control/Compare To Constant1'
 * '<S4>'   : 'codegen/control/PID Controller1'
 * '<S5>'   : 'codegen/control/PID Controller2'
 * '<S6>'   : 'codegen/control/PID Controller3'
 * '<S7>'   : 'codegen/control/PID Controller4'
 * '<S8>'   : 'codegen/control/ramp'
 * '<S9>'   : 'codegen/control/ramp1'
 * '<S10>'  : 'codegen/control/ramp2'
 * '<S11>'  : 'codegen/control/ramp3'
 * '<S12>'  : 'codegen/control/PID Controller1/Anti-windup'
 * '<S13>'  : 'codegen/control/PID Controller1/D Gain'
 * '<S14>'  : 'codegen/control/PID Controller1/External Derivative'
 * '<S15>'  : 'codegen/control/PID Controller1/Filter'
 * '<S16>'  : 'codegen/control/PID Controller1/Filter ICs'
 * '<S17>'  : 'codegen/control/PID Controller1/I Gain'
 * '<S18>'  : 'codegen/control/PID Controller1/Ideal P Gain'
 * '<S19>'  : 'codegen/control/PID Controller1/Ideal P Gain Fdbk'
 * '<S20>'  : 'codegen/control/PID Controller1/Integrator'
 * '<S21>'  : 'codegen/control/PID Controller1/Integrator ICs'
 * '<S22>'  : 'codegen/control/PID Controller1/N Copy'
 * '<S23>'  : 'codegen/control/PID Controller1/N Gain'
 * '<S24>'  : 'codegen/control/PID Controller1/P Copy'
 * '<S25>'  : 'codegen/control/PID Controller1/Parallel P Gain'
 * '<S26>'  : 'codegen/control/PID Controller1/Reset Signal'
 * '<S27>'  : 'codegen/control/PID Controller1/Saturation'
 * '<S28>'  : 'codegen/control/PID Controller1/Saturation Fdbk'
 * '<S29>'  : 'codegen/control/PID Controller1/Sum'
 * '<S30>'  : 'codegen/control/PID Controller1/Sum Fdbk'
 * '<S31>'  : 'codegen/control/PID Controller1/Tracking Mode'
 * '<S32>'  : 'codegen/control/PID Controller1/Tracking Mode Sum'
 * '<S33>'  : 'codegen/control/PID Controller1/Tsamp - Integral'
 * '<S34>'  : 'codegen/control/PID Controller1/Tsamp - Ngain'
 * '<S35>'  : 'codegen/control/PID Controller1/postSat Signal'
 * '<S36>'  : 'codegen/control/PID Controller1/preInt Signal'
 * '<S37>'  : 'codegen/control/PID Controller1/preSat Signal'
 * '<S38>'  : 'codegen/control/PID Controller1/Anti-windup/Passthrough'
 * '<S39>'  : 'codegen/control/PID Controller1/D Gain/Disabled'
 * '<S40>'  : 'codegen/control/PID Controller1/External Derivative/Disabled'
 * '<S41>'  : 'codegen/control/PID Controller1/Filter/Disabled'
 * '<S42>'  : 'codegen/control/PID Controller1/Filter ICs/Disabled'
 * '<S43>'  : 'codegen/control/PID Controller1/I Gain/Internal Parameters'
 * '<S44>'  : 'codegen/control/PID Controller1/Ideal P Gain/Passthrough'
 * '<S45>'  : 'codegen/control/PID Controller1/Ideal P Gain Fdbk/Disabled'
 * '<S46>'  : 'codegen/control/PID Controller1/Integrator/Discrete'
 * '<S47>'  : 'codegen/control/PID Controller1/Integrator ICs/Internal IC'
 * '<S48>'  : 'codegen/control/PID Controller1/N Copy/Disabled wSignal Specification'
 * '<S49>'  : 'codegen/control/PID Controller1/N Gain/Disabled'
 * '<S50>'  : 'codegen/control/PID Controller1/P Copy/Disabled'
 * '<S51>'  : 'codegen/control/PID Controller1/Parallel P Gain/Internal Parameters'
 * '<S52>'  : 'codegen/control/PID Controller1/Reset Signal/Disabled'
 * '<S53>'  : 'codegen/control/PID Controller1/Saturation/Passthrough'
 * '<S54>'  : 'codegen/control/PID Controller1/Saturation Fdbk/Disabled'
 * '<S55>'  : 'codegen/control/PID Controller1/Sum/Sum_PI'
 * '<S56>'  : 'codegen/control/PID Controller1/Sum Fdbk/Disabled'
 * '<S57>'  : 'codegen/control/PID Controller1/Tracking Mode/Disabled'
 * '<S58>'  : 'codegen/control/PID Controller1/Tracking Mode Sum/Passthrough'
 * '<S59>'  : 'codegen/control/PID Controller1/Tsamp - Integral/TsSignalSpecification'
 * '<S60>'  : 'codegen/control/PID Controller1/Tsamp - Ngain/Passthrough'
 * '<S61>'  : 'codegen/control/PID Controller1/postSat Signal/Forward_Path'
 * '<S62>'  : 'codegen/control/PID Controller1/preInt Signal/Internal PreInt'
 * '<S63>'  : 'codegen/control/PID Controller1/preSat Signal/Forward_Path'
 * '<S64>'  : 'codegen/control/PID Controller2/Anti-windup'
 * '<S65>'  : 'codegen/control/PID Controller2/D Gain'
 * '<S66>'  : 'codegen/control/PID Controller2/External Derivative'
 * '<S67>'  : 'codegen/control/PID Controller2/Filter'
 * '<S68>'  : 'codegen/control/PID Controller2/Filter ICs'
 * '<S69>'  : 'codegen/control/PID Controller2/I Gain'
 * '<S70>'  : 'codegen/control/PID Controller2/Ideal P Gain'
 * '<S71>'  : 'codegen/control/PID Controller2/Ideal P Gain Fdbk'
 * '<S72>'  : 'codegen/control/PID Controller2/Integrator'
 * '<S73>'  : 'codegen/control/PID Controller2/Integrator ICs'
 * '<S74>'  : 'codegen/control/PID Controller2/N Copy'
 * '<S75>'  : 'codegen/control/PID Controller2/N Gain'
 * '<S76>'  : 'codegen/control/PID Controller2/P Copy'
 * '<S77>'  : 'codegen/control/PID Controller2/Parallel P Gain'
 * '<S78>'  : 'codegen/control/PID Controller2/Reset Signal'
 * '<S79>'  : 'codegen/control/PID Controller2/Saturation'
 * '<S80>'  : 'codegen/control/PID Controller2/Saturation Fdbk'
 * '<S81>'  : 'codegen/control/PID Controller2/Sum'
 * '<S82>'  : 'codegen/control/PID Controller2/Sum Fdbk'
 * '<S83>'  : 'codegen/control/PID Controller2/Tracking Mode'
 * '<S84>'  : 'codegen/control/PID Controller2/Tracking Mode Sum'
 * '<S85>'  : 'codegen/control/PID Controller2/Tsamp - Integral'
 * '<S86>'  : 'codegen/control/PID Controller2/Tsamp - Ngain'
 * '<S87>'  : 'codegen/control/PID Controller2/postSat Signal'
 * '<S88>'  : 'codegen/control/PID Controller2/preInt Signal'
 * '<S89>'  : 'codegen/control/PID Controller2/preSat Signal'
 * '<S90>'  : 'codegen/control/PID Controller2/Anti-windup/Passthrough'
 * '<S91>'  : 'codegen/control/PID Controller2/D Gain/Disabled'
 * '<S92>'  : 'codegen/control/PID Controller2/External Derivative/Disabled'
 * '<S93>'  : 'codegen/control/PID Controller2/Filter/Disabled'
 * '<S94>'  : 'codegen/control/PID Controller2/Filter ICs/Disabled'
 * '<S95>'  : 'codegen/control/PID Controller2/I Gain/Internal Parameters'
 * '<S96>'  : 'codegen/control/PID Controller2/Ideal P Gain/Passthrough'
 * '<S97>'  : 'codegen/control/PID Controller2/Ideal P Gain Fdbk/Disabled'
 * '<S98>'  : 'codegen/control/PID Controller2/Integrator/Discrete'
 * '<S99>'  : 'codegen/control/PID Controller2/Integrator ICs/Internal IC'
 * '<S100>' : 'codegen/control/PID Controller2/N Copy/Disabled wSignal Specification'
 * '<S101>' : 'codegen/control/PID Controller2/N Gain/Disabled'
 * '<S102>' : 'codegen/control/PID Controller2/P Copy/Disabled'
 * '<S103>' : 'codegen/control/PID Controller2/Parallel P Gain/Internal Parameters'
 * '<S104>' : 'codegen/control/PID Controller2/Reset Signal/Disabled'
 * '<S105>' : 'codegen/control/PID Controller2/Saturation/Passthrough'
 * '<S106>' : 'codegen/control/PID Controller2/Saturation Fdbk/Disabled'
 * '<S107>' : 'codegen/control/PID Controller2/Sum/Sum_PI'
 * '<S108>' : 'codegen/control/PID Controller2/Sum Fdbk/Disabled'
 * '<S109>' : 'codegen/control/PID Controller2/Tracking Mode/Disabled'
 * '<S110>' : 'codegen/control/PID Controller2/Tracking Mode Sum/Passthrough'
 * '<S111>' : 'codegen/control/PID Controller2/Tsamp - Integral/TsSignalSpecification'
 * '<S112>' : 'codegen/control/PID Controller2/Tsamp - Ngain/Passthrough'
 * '<S113>' : 'codegen/control/PID Controller2/postSat Signal/Forward_Path'
 * '<S114>' : 'codegen/control/PID Controller2/preInt Signal/Internal PreInt'
 * '<S115>' : 'codegen/control/PID Controller2/preSat Signal/Forward_Path'
 * '<S116>' : 'codegen/control/PID Controller3/Anti-windup'
 * '<S117>' : 'codegen/control/PID Controller3/D Gain'
 * '<S118>' : 'codegen/control/PID Controller3/External Derivative'
 * '<S119>' : 'codegen/control/PID Controller3/Filter'
 * '<S120>' : 'codegen/control/PID Controller3/Filter ICs'
 * '<S121>' : 'codegen/control/PID Controller3/I Gain'
 * '<S122>' : 'codegen/control/PID Controller3/Ideal P Gain'
 * '<S123>' : 'codegen/control/PID Controller3/Ideal P Gain Fdbk'
 * '<S124>' : 'codegen/control/PID Controller3/Integrator'
 * '<S125>' : 'codegen/control/PID Controller3/Integrator ICs'
 * '<S126>' : 'codegen/control/PID Controller3/N Copy'
 * '<S127>' : 'codegen/control/PID Controller3/N Gain'
 * '<S128>' : 'codegen/control/PID Controller3/P Copy'
 * '<S129>' : 'codegen/control/PID Controller3/Parallel P Gain'
 * '<S130>' : 'codegen/control/PID Controller3/Reset Signal'
 * '<S131>' : 'codegen/control/PID Controller3/Saturation'
 * '<S132>' : 'codegen/control/PID Controller3/Saturation Fdbk'
 * '<S133>' : 'codegen/control/PID Controller3/Sum'
 * '<S134>' : 'codegen/control/PID Controller3/Sum Fdbk'
 * '<S135>' : 'codegen/control/PID Controller3/Tracking Mode'
 * '<S136>' : 'codegen/control/PID Controller3/Tracking Mode Sum'
 * '<S137>' : 'codegen/control/PID Controller3/Tsamp - Integral'
 * '<S138>' : 'codegen/control/PID Controller3/Tsamp - Ngain'
 * '<S139>' : 'codegen/control/PID Controller3/postSat Signal'
 * '<S140>' : 'codegen/control/PID Controller3/preInt Signal'
 * '<S141>' : 'codegen/control/PID Controller3/preSat Signal'
 * '<S142>' : 'codegen/control/PID Controller3/Anti-windup/Passthrough'
 * '<S143>' : 'codegen/control/PID Controller3/D Gain/Disabled'
 * '<S144>' : 'codegen/control/PID Controller3/External Derivative/Disabled'
 * '<S145>' : 'codegen/control/PID Controller3/Filter/Disabled'
 * '<S146>' : 'codegen/control/PID Controller3/Filter ICs/Disabled'
 * '<S147>' : 'codegen/control/PID Controller3/I Gain/Internal Parameters'
 * '<S148>' : 'codegen/control/PID Controller3/Ideal P Gain/Passthrough'
 * '<S149>' : 'codegen/control/PID Controller3/Ideal P Gain Fdbk/Disabled'
 * '<S150>' : 'codegen/control/PID Controller3/Integrator/Discrete'
 * '<S151>' : 'codegen/control/PID Controller3/Integrator ICs/Internal IC'
 * '<S152>' : 'codegen/control/PID Controller3/N Copy/Disabled wSignal Specification'
 * '<S153>' : 'codegen/control/PID Controller3/N Gain/Disabled'
 * '<S154>' : 'codegen/control/PID Controller3/P Copy/Disabled'
 * '<S155>' : 'codegen/control/PID Controller3/Parallel P Gain/Internal Parameters'
 * '<S156>' : 'codegen/control/PID Controller3/Reset Signal/Disabled'
 * '<S157>' : 'codegen/control/PID Controller3/Saturation/Passthrough'
 * '<S158>' : 'codegen/control/PID Controller3/Saturation Fdbk/Disabled'
 * '<S159>' : 'codegen/control/PID Controller3/Sum/Sum_PI'
 * '<S160>' : 'codegen/control/PID Controller3/Sum Fdbk/Disabled'
 * '<S161>' : 'codegen/control/PID Controller3/Tracking Mode/Disabled'
 * '<S162>' : 'codegen/control/PID Controller3/Tracking Mode Sum/Passthrough'
 * '<S163>' : 'codegen/control/PID Controller3/Tsamp - Integral/TsSignalSpecification'
 * '<S164>' : 'codegen/control/PID Controller3/Tsamp - Ngain/Passthrough'
 * '<S165>' : 'codegen/control/PID Controller3/postSat Signal/Forward_Path'
 * '<S166>' : 'codegen/control/PID Controller3/preInt Signal/Internal PreInt'
 * '<S167>' : 'codegen/control/PID Controller3/preSat Signal/Forward_Path'
 * '<S168>' : 'codegen/control/PID Controller4/Anti-windup'
 * '<S169>' : 'codegen/control/PID Controller4/D Gain'
 * '<S170>' : 'codegen/control/PID Controller4/External Derivative'
 * '<S171>' : 'codegen/control/PID Controller4/Filter'
 * '<S172>' : 'codegen/control/PID Controller4/Filter ICs'
 * '<S173>' : 'codegen/control/PID Controller4/I Gain'
 * '<S174>' : 'codegen/control/PID Controller4/Ideal P Gain'
 * '<S175>' : 'codegen/control/PID Controller4/Ideal P Gain Fdbk'
 * '<S176>' : 'codegen/control/PID Controller4/Integrator'
 * '<S177>' : 'codegen/control/PID Controller4/Integrator ICs'
 * '<S178>' : 'codegen/control/PID Controller4/N Copy'
 * '<S179>' : 'codegen/control/PID Controller4/N Gain'
 * '<S180>' : 'codegen/control/PID Controller4/P Copy'
 * '<S181>' : 'codegen/control/PID Controller4/Parallel P Gain'
 * '<S182>' : 'codegen/control/PID Controller4/Reset Signal'
 * '<S183>' : 'codegen/control/PID Controller4/Saturation'
 * '<S184>' : 'codegen/control/PID Controller4/Saturation Fdbk'
 * '<S185>' : 'codegen/control/PID Controller4/Sum'
 * '<S186>' : 'codegen/control/PID Controller4/Sum Fdbk'
 * '<S187>' : 'codegen/control/PID Controller4/Tracking Mode'
 * '<S188>' : 'codegen/control/PID Controller4/Tracking Mode Sum'
 * '<S189>' : 'codegen/control/PID Controller4/Tsamp - Integral'
 * '<S190>' : 'codegen/control/PID Controller4/Tsamp - Ngain'
 * '<S191>' : 'codegen/control/PID Controller4/postSat Signal'
 * '<S192>' : 'codegen/control/PID Controller4/preInt Signal'
 * '<S193>' : 'codegen/control/PID Controller4/preSat Signal'
 * '<S194>' : 'codegen/control/PID Controller4/Anti-windup/Passthrough'
 * '<S195>' : 'codegen/control/PID Controller4/D Gain/Disabled'
 * '<S196>' : 'codegen/control/PID Controller4/External Derivative/Disabled'
 * '<S197>' : 'codegen/control/PID Controller4/Filter/Disabled'
 * '<S198>' : 'codegen/control/PID Controller4/Filter ICs/Disabled'
 * '<S199>' : 'codegen/control/PID Controller4/I Gain/Internal Parameters'
 * '<S200>' : 'codegen/control/PID Controller4/Ideal P Gain/Passthrough'
 * '<S201>' : 'codegen/control/PID Controller4/Ideal P Gain Fdbk/Disabled'
 * '<S202>' : 'codegen/control/PID Controller4/Integrator/Discrete'
 * '<S203>' : 'codegen/control/PID Controller4/Integrator ICs/Internal IC'
 * '<S204>' : 'codegen/control/PID Controller4/N Copy/Disabled wSignal Specification'
 * '<S205>' : 'codegen/control/PID Controller4/N Gain/Disabled'
 * '<S206>' : 'codegen/control/PID Controller4/P Copy/Disabled'
 * '<S207>' : 'codegen/control/PID Controller4/Parallel P Gain/Internal Parameters'
 * '<S208>' : 'codegen/control/PID Controller4/Reset Signal/Disabled'
 * '<S209>' : 'codegen/control/PID Controller4/Saturation/Passthrough'
 * '<S210>' : 'codegen/control/PID Controller4/Saturation Fdbk/Disabled'
 * '<S211>' : 'codegen/control/PID Controller4/Sum/Sum_PI'
 * '<S212>' : 'codegen/control/PID Controller4/Sum Fdbk/Disabled'
 * '<S213>' : 'codegen/control/PID Controller4/Tracking Mode/Disabled'
 * '<S214>' : 'codegen/control/PID Controller4/Tracking Mode Sum/Passthrough'
 * '<S215>' : 'codegen/control/PID Controller4/Tsamp - Integral/TsSignalSpecification'
 * '<S216>' : 'codegen/control/PID Controller4/Tsamp - Ngain/Passthrough'
 * '<S217>' : 'codegen/control/PID Controller4/postSat Signal/Forward_Path'
 * '<S218>' : 'codegen/control/PID Controller4/preInt Signal/Internal PreInt'
 * '<S219>' : 'codegen/control/PID Controller4/preSat Signal/Forward_Path'
 */
#endif                                 /* control0_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */