/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: motorLF1.h
 *
 * Code generated for Simulink model 'motorLF1'.
 *
 * Model version                  : 3.26
 * Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
 * C/C++ source code generated on : Tue Jan  6 12:48:30 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: STMicroelectronics->ST10/Super10
 * Code generation objectives:
 *    1. Execution efficiency
 *    2. RAM efficiency
 * Validation result: Not run
 */

#ifndef motorLF1_h_
#define motorLF1_h_
#ifndef motorLF1_COMMON_INCLUDES_
#define motorLF1_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "math.h"
#include "nesl_rtw.h"
#include "motorLF1_446c021e_1_gateway.h"
#endif                                 /* motorLF1_COMMON_INCLUDES_ */

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
  real_T INPUT_1_1_1[4];               /* '<S13>/INPUT_1_1_1' */
  real_T STATE_1[3];                   /* '<S13>/STATE_1' */
  real_T INPUT_1_1_1_Discrete_4052416841[2];/* '<S13>/INPUT_1_1_1' */
  real_T STATE_1_Discrete_4271124629[3];/* '<S13>/STATE_1' */
  real_T STATE_1_ZcValueStore;         /* '<S13>/STATE_1' */
  real_T OUTPUT_1_0_Discrete;          /* '<S13>/OUTPUT_1_0' */
  real_T OUTPUT_1_0_ZcValueStore;      /* '<S13>/OUTPUT_1_0' */
  real_T OUTPUT_1_1_Discrete;          /* '<S13>/OUTPUT_1_1' */
  real_T OUTPUT_1_1_ZcValueStore;      /* '<S13>/OUTPUT_1_1' */
  int_T STATE_1_Modes;                 /* '<S13>/STATE_1' */
  int_T OUTPUT_1_0_Modes;              /* '<S13>/OUTPUT_1_0' */
  int_T OUTPUT_1_1_Modes;              /* '<S13>/OUTPUT_1_1' */
  void* STATE_1_Simulator;             /* '<S13>/STATE_1' */
  void* STATE_1_SimData;               /* '<S13>/STATE_1' */
  void* STATE_1_DiagMgr;               /* '<S13>/STATE_1' */
  void* STATE_1_ZcLogger;              /* '<S13>/STATE_1' */
  void* STATE_1_TsInfo;                /* '<S13>/STATE_1' */
  void* OUTPUT_1_0_Simulator;          /* '<S13>/OUTPUT_1_0' */
  void* OUTPUT_1_0_SimData;            /* '<S13>/OUTPUT_1_0' */
  void* OUTPUT_1_0_DiagMgr;            /* '<S13>/OUTPUT_1_0' */
  void* OUTPUT_1_0_ZcLogger;           /* '<S13>/OUTPUT_1_0' */
  void* OUTPUT_1_0_TsInfo;             /* '<S13>/OUTPUT_1_0' */
  void* OUTPUT_1_1_Simulator;          /* '<S13>/OUTPUT_1_1' */
  void* OUTPUT_1_1_SimData;            /* '<S13>/OUTPUT_1_1' */
  void* OUTPUT_1_1_DiagMgr;            /* '<S13>/OUTPUT_1_1' */
  void* OUTPUT_1_1_ZcLogger;           /* '<S13>/OUTPUT_1_1' */
  void* OUTPUT_1_1_TsInfo;             /* '<S13>/OUTPUT_1_1' */
  uint8_T STATE_1_ZcSignalDir;         /* '<S13>/STATE_1' */
  uint8_T STATE_1_ZcStateStore;        /* '<S13>/STATE_1' */
  uint8_T OUTPUT_1_0_ZcSignalDir;      /* '<S13>/OUTPUT_1_0' */
  uint8_T OUTPUT_1_0_ZcStateStore;     /* '<S13>/OUTPUT_1_0' */
  uint8_T OUTPUT_1_1_ZcSignalDir;      /* '<S13>/OUTPUT_1_1' */
  uint8_T OUTPUT_1_1_ZcStateStore;     /* '<S13>/OUTPUT_1_1' */
  boolean_T STATE_1_FirstOutput;       /* '<S13>/STATE_1' */
  boolean_T OUTPUT_1_0_FirstOutput;    /* '<S13>/OUTPUT_1_0' */
  boolean_T OUTPUT_1_1_FirstOutput;    /* '<S13>/OUTPUT_1_1' */
} DW;

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T control;                      /* '<Root>/control' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T volts;                        /* '<Root>/volts' */
  real_T w;                            /* '<Root>/w' */
  real_T amps;                         /* '<Root>/amps' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    uint32_T clockTick0;
  } Timing;
};

/* Block signals and states (default storage) */
extern DW rtDW;

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/* Model entry point functions */
extern void motorLF1_initialize(void);
extern void motorLF1_step(void);

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
 * hilite_system('simulinksim/motorLF1')    - opens subsystem simulinksim/motorLF1
 * hilite_system('simulinksim/motorLF1/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'simulinksim'
 * '<S1>'   : 'simulinksim/motorLF1'
 * '<S2>'   : 'simulinksim/motorLF1/PS-Simulink Converter1'
 * '<S3>'   : 'simulinksim/motorLF1/PS-Simulink Converter2'
 * '<S4>'   : 'simulinksim/motorLF1/PS-Simulink Converter3'
 * '<S5>'   : 'simulinksim/motorLF1/PS-Simulink Converter4'
 * '<S6>'   : 'simulinksim/motorLF1/Simulink-PS Converter'
 * '<S7>'   : 'simulinksim/motorLF1/Solver Configuration'
 * '<S8>'   : 'simulinksim/motorLF1/PS-Simulink Converter1/EVAL_KEY'
 * '<S9>'   : 'simulinksim/motorLF1/PS-Simulink Converter2/EVAL_KEY'
 * '<S10>'  : 'simulinksim/motorLF1/PS-Simulink Converter3/EVAL_KEY'
 * '<S11>'  : 'simulinksim/motorLF1/PS-Simulink Converter4/EVAL_KEY'
 * '<S12>'  : 'simulinksim/motorLF1/Simulink-PS Converter/EVAL_KEY'
 * '<S13>'  : 'simulinksim/motorLF1/Solver Configuration/EVAL_KEY'
 */
#endif                                 /* motorLF1_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
