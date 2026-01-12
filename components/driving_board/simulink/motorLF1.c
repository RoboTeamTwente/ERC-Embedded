/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: motorLF1.c
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

#include "motorLF1.h"
#include "rtwtypes.h"
#include <stddef.h>

/* Block signals and states (default storage) */
DW rtDW;

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
static RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function */
void motorLF1_step(void)
{
  NeslSimulationData *simulationData;
  NeuDiagnosticManager *diagnosticManager;
  NeuDiagnosticTree *diagnosticTree;
  NeuDiagnosticTree *diagnosticTree_0;
  NeuDiagnosticTree *diagnosticTree_1;
  NeuDiagnosticTree *diagnosticTree_2;
  char *msg;
  char *msg_0;
  char *msg_1;
  char *msg_2;
  real_T tmp_3[7];
  real_T tmp_5[7];
  real_T tmp_0[4];
  real_T tmp_7[4];
  real_T rtb_OUTPUT_1_1[2];
  real_T time;
  real_T time_0;
  real_T time_1;
  real_T time_2;
  real_T time_3;
  real_T time_4;
  real_T time_5;
  real_T time_tmp;
  real_T time_tmp_0;
  int32_T isHit;
  int32_T isHit_0;
  int32_T isHit_1;
  int32_T tmp_2;
  int_T tmp_4[3];
  int_T tmp_6[3];
  int_T tmp_1[2];
  int_T tmp_8[2];
  boolean_T tmp;

  /* SimscapeInputBlock: '<S13>/INPUT_1_1_1' incorporates:
   *  Inport: '<Root>/control'
   */
  rtDW.INPUT_1_1_1[0] = rtU.control;
  rtDW.INPUT_1_1_1[1] = 0.0;
  rtDW.INPUT_1_1_1[2] = 0.0;
  rtDW.INPUT_1_1_1_Discrete_4052416841[0] = !(rtDW.INPUT_1_1_1[0] ==
    rtDW.INPUT_1_1_1_Discrete_4052416841[1]);
  rtDW.INPUT_1_1_1_Discrete_4052416841[1] = rtDW.INPUT_1_1_1[0];
  rtDW.INPUT_1_1_1[0] = rtDW.INPUT_1_1_1_Discrete_4052416841[1];
  rtDW.INPUT_1_1_1[3] = rtDW.INPUT_1_1_1_Discrete_4052416841[0];

  /* SimscapeExecutionBlock: '<S13>/STATE_1' incorporates:
   *  SimscapeExecutionBlock: '<S13>/OUTPUT_1_0'
   *  SimscapeExecutionBlock: '<S13>/OUTPUT_1_1'
   */
  simulationData = (NeslSimulationData *)rtDW.STATE_1_SimData;
  time_tmp = ((rtM->Timing.clockTick0) * 0.001);
  time = time_tmp;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time;
  simulationData->mData->mContStates.mN = 0;
  simulationData->mData->mContStates.mX = NULL;
  simulationData->mData->mDiscStates.mN = 3;
  simulationData->mData->mDiscStates.mX = &rtDW.STATE_1_Discrete_4271124629[0];
  simulationData->mData->mModeVector.mN = 0;
  simulationData->mData->mModeVector.mX = (int32_T *)&rtDW.STATE_1_Modes;
  tmp = false;
  simulationData->mData->mFoundZcEvents = tmp;
  simulationData->mData->mHadEvents = false;
  simulationData->mData->mIsMajorTimeStep = true;
  tmp = false;
  simulationData->mData->mIsSolverAssertCheck = tmp;
  simulationData->mData->mIsSolverCheckingCIC = false;
  simulationData->mData->mIsComputingJacobian = false;
  simulationData->mData->mIsEvaluatingF0 = false;
  simulationData->mData->mIsSolverRequestingReset = false;
  simulationData->mData->mIsModeUpdateTimeStep = true;
  tmp_1[0] = 0;
  tmp_0[0] = rtDW.INPUT_1_1_1[0];
  tmp_0[1] = rtDW.INPUT_1_1_1[1];
  tmp_0[2] = rtDW.INPUT_1_1_1[2];
  tmp_0[3] = rtDW.INPUT_1_1_1[3];
  tmp_1[1] = 4;
  simulationData->mData->mInputValues.mN = 4;
  simulationData->mData->mInputValues.mX = &tmp_0[0];
  simulationData->mData->mInputOffsets.mN = 2;
  simulationData->mData->mInputOffsets.mX = (int32_T *)&tmp_1[0];
  simulationData->mData->mOutputs.mN = 3;
  simulationData->mData->mOutputs.mX = &rtDW.STATE_1[0];
  simulationData->mData->mTolerances.mN = 0;
  simulationData->mData->mTolerances.mX = NULL;
  simulationData->mData->mCstateHasChanged = false;
  simulationData->mData->mDstateHasChanged = false;
  time_tmp_0 = ((rtM->Timing.clockTick0) * 0.001);
  time_0 = time_tmp_0;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_0;
  isHit = 0L;
  simulationData->mData->mSampleHits.mN = 1;
  simulationData->mData->mSampleHits.mX = &isHit;
  simulationData->mData->mIsFundamentalSampleHit = true;
  simulationData->mData->mHadEvents = false;
  diagnosticManager = (NeuDiagnosticManager *)rtDW.STATE_1_DiagMgr;
  diagnosticTree = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
  tmp_2 = ne_simulator_method((NeslSimulator *)rtDW.STATE_1_Simulator,
    NESL_SIM_OUTPUTS, simulationData, diagnosticManager);
  if (tmp_2 != 0L) {
    tmp = error_buffer_is_empty(rtmGetErrorStatus(rtM));
    if (tmp) {
      msg = rtw_diagnostics_msg(diagnosticTree);
      rtmSetErrorStatus(rtM, msg);
    }
  }

  /* End of SimscapeExecutionBlock: '<S13>/STATE_1' */

  /* SimscapeExecutionBlock: '<S13>/OUTPUT_1_0' */
  simulationData = (NeslSimulationData *)rtDW.OUTPUT_1_0_SimData;
  time_1 = time_tmp;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_1;
  simulationData->mData->mContStates.mN = 0;
  simulationData->mData->mContStates.mX = NULL;
  simulationData->mData->mDiscStates.mN = 0;
  simulationData->mData->mDiscStates.mX = &rtDW.OUTPUT_1_0_Discrete;
  simulationData->mData->mModeVector.mN = 0;
  simulationData->mData->mModeVector.mX = (int32_T *)&rtDW.OUTPUT_1_0_Modes;
  tmp = false;
  simulationData->mData->mFoundZcEvents = tmp;
  simulationData->mData->mHadEvents = false;
  simulationData->mData->mIsMajorTimeStep = true;
  tmp = false;
  simulationData->mData->mIsSolverAssertCheck = tmp;
  simulationData->mData->mIsSolverCheckingCIC = false;
  simulationData->mData->mIsComputingJacobian = false;
  simulationData->mData->mIsEvaluatingF0 = false;
  simulationData->mData->mIsSolverRequestingReset = false;
  simulationData->mData->mIsModeUpdateTimeStep = true;
  tmp_4[0] = 0;
  tmp_3[0] = rtDW.INPUT_1_1_1[0];
  tmp_3[1] = rtDW.INPUT_1_1_1[1];
  tmp_3[2] = rtDW.INPUT_1_1_1[2];
  tmp_3[3] = rtDW.INPUT_1_1_1[3];
  tmp_4[1] = 4;
  tmp_3[4] = rtDW.STATE_1[0];
  tmp_3[5] = rtDW.STATE_1[1];
  tmp_3[6] = rtDW.STATE_1[2];
  tmp_4[2] = 7;
  simulationData->mData->mInputValues.mN = 7;
  simulationData->mData->mInputValues.mX = &tmp_3[0];
  simulationData->mData->mInputOffsets.mN = 3;
  simulationData->mData->mInputOffsets.mX = (int32_T *)&tmp_4[0];
  simulationData->mData->mOutputs.mN = 2;
  simulationData->mData->mOutputs.mX = &rtb_OUTPUT_1_1[0];
  simulationData->mData->mTolerances.mN = 0;
  simulationData->mData->mTolerances.mX = NULL;
  simulationData->mData->mCstateHasChanged = false;
  simulationData->mData->mDstateHasChanged = false;
  time_2 = time_tmp_0;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_2;
  isHit_0 = 0L;
  simulationData->mData->mSampleHits.mN = 1;
  simulationData->mData->mSampleHits.mX = &isHit_0;
  simulationData->mData->mIsFundamentalSampleHit = true;
  simulationData->mData->mHadEvents = false;
  diagnosticManager = (NeuDiagnosticManager *)rtDW.OUTPUT_1_0_DiagMgr;
  diagnosticTree_0 = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
  tmp_2 = ne_simulator_method((NeslSimulator *)rtDW.OUTPUT_1_0_Simulator,
    NESL_SIM_OUTPUTS, simulationData, diagnosticManager);
  if (tmp_2 != 0L) {
    tmp = error_buffer_is_empty(rtmGetErrorStatus(rtM));
    if (tmp) {
      msg_0 = rtw_diagnostics_msg(diagnosticTree_0);
      rtmSetErrorStatus(rtM, msg_0);
    }
  }

  /* Outport: '<Root>/volts' */
  rtY.volts = rtb_OUTPUT_1_1[1];

  /* Outport: '<Root>/amps' */
  rtY.amps = rtb_OUTPUT_1_1[0];

  /* SimscapeExecutionBlock: '<S13>/OUTPUT_1_1' */
  simulationData = (NeslSimulationData *)rtDW.OUTPUT_1_1_SimData;
  time_3 = time_tmp;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_3;
  simulationData->mData->mContStates.mN = 0;
  simulationData->mData->mContStates.mX = NULL;
  simulationData->mData->mDiscStates.mN = 0;
  simulationData->mData->mDiscStates.mX = &rtDW.OUTPUT_1_1_Discrete;
  simulationData->mData->mModeVector.mN = 0;
  simulationData->mData->mModeVector.mX = (int32_T *)&rtDW.OUTPUT_1_1_Modes;
  tmp = false;
  simulationData->mData->mFoundZcEvents = tmp;
  simulationData->mData->mHadEvents = false;
  simulationData->mData->mIsMajorTimeStep = true;
  tmp = false;
  simulationData->mData->mIsSolverAssertCheck = tmp;
  simulationData->mData->mIsSolverCheckingCIC = false;
  simulationData->mData->mIsComputingJacobian = false;
  simulationData->mData->mIsEvaluatingF0 = false;
  simulationData->mData->mIsSolverRequestingReset = false;
  simulationData->mData->mIsModeUpdateTimeStep = true;
  tmp_6[0] = 0;
  tmp_5[0] = rtDW.INPUT_1_1_1[0];
  tmp_5[1] = rtDW.INPUT_1_1_1[1];
  tmp_5[2] = rtDW.INPUT_1_1_1[2];
  tmp_5[3] = rtDW.INPUT_1_1_1[3];
  tmp_6[1] = 4;
  tmp_5[4] = rtDW.STATE_1[0];
  tmp_5[5] = rtDW.STATE_1[1];
  tmp_5[6] = rtDW.STATE_1[2];
  tmp_6[2] = 7;
  simulationData->mData->mInputValues.mN = 7;
  simulationData->mData->mInputValues.mX = &tmp_5[0];
  simulationData->mData->mInputOffsets.mN = 3;
  simulationData->mData->mInputOffsets.mX = (int32_T *)&tmp_6[0];
  simulationData->mData->mOutputs.mN = 2;
  simulationData->mData->mOutputs.mX = &rtb_OUTPUT_1_1[0];
  simulationData->mData->mTolerances.mN = 0;
  simulationData->mData->mTolerances.mX = NULL;
  simulationData->mData->mCstateHasChanged = false;
  simulationData->mData->mDstateHasChanged = false;
  time_4 = time_tmp_0;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_4;
  isHit_1 = 0L;
  simulationData->mData->mSampleHits.mN = 1;
  simulationData->mData->mSampleHits.mX = &isHit_1;
  simulationData->mData->mIsFundamentalSampleHit = true;
  simulationData->mData->mHadEvents = false;
  diagnosticManager = (NeuDiagnosticManager *)rtDW.OUTPUT_1_1_DiagMgr;
  diagnosticTree_1 = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
  tmp_2 = ne_simulator_method((NeslSimulator *)rtDW.OUTPUT_1_1_Simulator,
    NESL_SIM_OUTPUTS, simulationData, diagnosticManager);
  if (tmp_2 != 0L) {
    tmp = error_buffer_is_empty(rtmGetErrorStatus(rtM));
    if (tmp) {
      msg_1 = rtw_diagnostics_msg(diagnosticTree_1);
      rtmSetErrorStatus(rtM, msg_1);
    }
  }

  /* Outport: '<Root>/w' */
  rtY.w = rtb_OUTPUT_1_1[1];

  /* Update for SimscapeExecutionBlock: '<S13>/STATE_1' */
  simulationData = (NeslSimulationData *)rtDW.STATE_1_SimData;
  time_5 = time_tmp;
  simulationData->mData->mTime.mN = 1;
  simulationData->mData->mTime.mX = &time_5;
  simulationData->mData->mContStates.mN = 0;
  simulationData->mData->mContStates.mX = NULL;
  simulationData->mData->mDiscStates.mN = 3;
  simulationData->mData->mDiscStates.mX = &rtDW.STATE_1_Discrete_4271124629[0];
  simulationData->mData->mModeVector.mN = 0;
  simulationData->mData->mModeVector.mX = (int32_T *)&rtDW.STATE_1_Modes;
  tmp = false;
  simulationData->mData->mFoundZcEvents = tmp;
  simulationData->mData->mHadEvents = false;
  simulationData->mData->mIsMajorTimeStep = true;
  tmp = false;
  simulationData->mData->mIsSolverAssertCheck = tmp;
  simulationData->mData->mIsSolverCheckingCIC = false;
  simulationData->mData->mIsComputingJacobian = false;
  simulationData->mData->mIsEvaluatingF0 = false;
  simulationData->mData->mIsSolverRequestingReset = false;
  simulationData->mData->mIsModeUpdateTimeStep = true;
  tmp_8[0] = 0;
  tmp_7[0] = rtDW.INPUT_1_1_1[0];
  tmp_7[1] = rtDW.INPUT_1_1_1[1];
  tmp_7[2] = rtDW.INPUT_1_1_1[2];
  tmp_7[3] = rtDW.INPUT_1_1_1[3];
  tmp_8[1] = 4;
  simulationData->mData->mInputValues.mN = 4;
  simulationData->mData->mInputValues.mX = &tmp_7[0];
  simulationData->mData->mInputOffsets.mN = 2;
  simulationData->mData->mInputOffsets.mX = (int32_T *)&tmp_8[0];
  diagnosticManager = (NeuDiagnosticManager *)rtDW.STATE_1_DiagMgr;
  diagnosticTree_2 = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
  tmp_2 = ne_simulator_method((NeslSimulator *)rtDW.STATE_1_Simulator,
    NESL_SIM_UPDATE, simulationData, diagnosticManager);
  if (tmp_2 != 0L) {
    tmp = error_buffer_is_empty(rtmGetErrorStatus(rtM));
    if (tmp) {
      msg_2 = rtw_diagnostics_msg(diagnosticTree_2);
      rtmSetErrorStatus(rtM, msg_2);
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The resolution of this integer timer is 0.001, which is the step size
   * of the task. Size of "clockTick0" ensures timer will not overflow during the
   * application lifespan selected.
   */
  rtM->Timing.clockTick0++;
}

/* Model initialize function */
void motorLF1_initialize(void)
{
  {
    NeModelParameters modelParameters;
    NeModelParameters modelParameters_0;
    NeModelParameters modelParameters_1;
    NeslSimulationData *tmp_1;
    NeslSimulator *tmp;
    NeuDiagnosticManager *diagnosticManager;
    NeuDiagnosticTree *diagnosticTree;
    NeuDiagnosticTree *diagnosticTree_0;
    NeuDiagnosticTree *diagnosticTree_1;
    char *msg;
    char *msg_0;
    char *msg_1;
    real_T tmp_2;
    int32_T tmp_3;
    boolean_T tmp_0;

    /* Start for SimscapeExecutionBlock: '<S13>/STATE_1' */
    tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 0L,
      0L);
    rtDW.STATE_1_Simulator = (void *)tmp;
    tmp_0 = pointer_is_null(rtDW.STATE_1_Simulator);
    if (tmp_0) {
      motorLF1_446c021e_1_gateway();
      tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 0L,
        0L);
      rtDW.STATE_1_Simulator = (void *)tmp;
    }

    tmp_1 = nesl_create_simulation_data();
    rtDW.STATE_1_SimData = (void *)tmp_1;
    diagnosticManager = rtw_create_diagnostics();
    rtDW.STATE_1_DiagMgr = (void *)diagnosticManager;
    modelParameters.mSolverType = NE_SOLVER_TYPE_DAE;
    modelParameters.mSolverTolerance = 0.001;
    modelParameters.mSolverAbsTol = 0.001;
    modelParameters.mSolverRelTol = 0.001;
    modelParameters.mVariableStepSolver = false;
    modelParameters.mIsUsingODEN = false;
    modelParameters.mSolverModifyAbsTol = NE_MODIFY_ABS_TOL_NO;
    modelParameters.mFixedStepSize = 0.001;
    modelParameters.mStartTime = 0.0;
    modelParameters.mLoadInitialState = false;
    modelParameters.mUseSimState = false;
    modelParameters.mLinTrimCompile = false;
    modelParameters.mLoggingMode = SSC_LOGGING_OFF;
    modelParameters.mRTWModifiedTimeStamp = 6.89600899E+8;
    modelParameters.mZcDisabled = true;
    modelParameters.mUseModelRefSolver = false;
    modelParameters.mTargetFPGAHIL = false;
    tmp_2 = 0.001;
    modelParameters.mSolverTolerance = tmp_2;
    tmp_2 = 0.001;
    modelParameters.mFixedStepSize = tmp_2;
    tmp_0 = false;
    modelParameters.mVariableStepSolver = tmp_0;
    tmp_0 = false;
    modelParameters.mIsUsingODEN = tmp_0;
    modelParameters.mZcDisabled = true;
    diagnosticManager = (NeuDiagnosticManager *)rtDW.STATE_1_DiagMgr;
    diagnosticTree = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
    tmp_3 = nesl_initialize_simulator((NeslSimulator *)rtDW.STATE_1_Simulator,
      &modelParameters, diagnosticManager);
    if (tmp_3 != 0L) {
      tmp_0 = error_buffer_is_empty(rtmGetErrorStatus(rtM));
      if (tmp_0) {
        msg = rtw_diagnostics_msg(diagnosticTree);
        rtmSetErrorStatus(rtM, msg);
      }
    }

    /* End of Start for SimscapeExecutionBlock: '<S13>/STATE_1' */

    /* Start for SimscapeExecutionBlock: '<S13>/OUTPUT_1_0' */
    tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 1L,
      0L);
    rtDW.OUTPUT_1_0_Simulator = (void *)tmp;
    tmp_0 = pointer_is_null(rtDW.OUTPUT_1_0_Simulator);
    if (tmp_0) {
      motorLF1_446c021e_1_gateway();
      tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 1L,
        0L);
      rtDW.OUTPUT_1_0_Simulator = (void *)tmp;
    }

    tmp_1 = nesl_create_simulation_data();
    rtDW.OUTPUT_1_0_SimData = (void *)tmp_1;
    diagnosticManager = rtw_create_diagnostics();
    rtDW.OUTPUT_1_0_DiagMgr = (void *)diagnosticManager;
    modelParameters_0.mSolverType = NE_SOLVER_TYPE_DAE;
    modelParameters_0.mSolverTolerance = 0.001;
    modelParameters_0.mSolverAbsTol = 0.001;
    modelParameters_0.mSolverRelTol = 0.001;
    modelParameters_0.mVariableStepSolver = false;
    modelParameters_0.mIsUsingODEN = false;
    modelParameters_0.mSolverModifyAbsTol = NE_MODIFY_ABS_TOL_NO;
    modelParameters_0.mFixedStepSize = 0.001;
    modelParameters_0.mStartTime = 0.0;
    modelParameters_0.mLoadInitialState = false;
    modelParameters_0.mUseSimState = false;
    modelParameters_0.mLinTrimCompile = false;
    modelParameters_0.mLoggingMode = SSC_LOGGING_OFF;
    modelParameters_0.mRTWModifiedTimeStamp = 6.89600899E+8;
    modelParameters_0.mZcDisabled = true;
    modelParameters_0.mUseModelRefSolver = false;
    modelParameters_0.mTargetFPGAHIL = false;
    tmp_2 = 0.001;
    modelParameters_0.mSolverTolerance = tmp_2;
    tmp_2 = 0.001;
    modelParameters_0.mFixedStepSize = tmp_2;
    tmp_0 = false;
    modelParameters_0.mVariableStepSolver = tmp_0;
    tmp_0 = false;
    modelParameters_0.mIsUsingODEN = tmp_0;
    modelParameters_0.mZcDisabled = true;
    diagnosticManager = (NeuDiagnosticManager *)rtDW.OUTPUT_1_0_DiagMgr;
    diagnosticTree_0 = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
    tmp_3 = nesl_initialize_simulator((NeslSimulator *)rtDW.OUTPUT_1_0_Simulator,
      &modelParameters_0, diagnosticManager);
    if (tmp_3 != 0L) {
      tmp_0 = error_buffer_is_empty(rtmGetErrorStatus(rtM));
      if (tmp_0) {
        msg_0 = rtw_diagnostics_msg(diagnosticTree_0);
        rtmSetErrorStatus(rtM, msg_0);
      }
    }

    /* End of Start for SimscapeExecutionBlock: '<S13>/OUTPUT_1_0' */

    /* Start for SimscapeExecutionBlock: '<S13>/OUTPUT_1_1' */
    tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 1L,
      1L);
    rtDW.OUTPUT_1_1_Simulator = (void *)tmp;
    tmp_0 = pointer_is_null(rtDW.OUTPUT_1_1_Simulator);
    if (tmp_0) {
      motorLF1_446c021e_1_gateway();
      tmp = nesl_lease_simulator("motorLF1/motorLF1/Solver Configuration_1", 1L,
        1L);
      rtDW.OUTPUT_1_1_Simulator = (void *)tmp;
    }

    tmp_1 = nesl_create_simulation_data();
    rtDW.OUTPUT_1_1_SimData = (void *)tmp_1;
    diagnosticManager = rtw_create_diagnostics();
    rtDW.OUTPUT_1_1_DiagMgr = (void *)diagnosticManager;
    modelParameters_1.mSolverType = NE_SOLVER_TYPE_DAE;
    modelParameters_1.mSolverTolerance = 0.001;
    modelParameters_1.mSolverAbsTol = 0.001;
    modelParameters_1.mSolverRelTol = 0.001;
    modelParameters_1.mVariableStepSolver = false;
    modelParameters_1.mIsUsingODEN = false;
    modelParameters_1.mSolverModifyAbsTol = NE_MODIFY_ABS_TOL_NO;
    modelParameters_1.mFixedStepSize = 0.001;
    modelParameters_1.mStartTime = 0.0;
    modelParameters_1.mLoadInitialState = false;
    modelParameters_1.mUseSimState = false;
    modelParameters_1.mLinTrimCompile = false;
    modelParameters_1.mLoggingMode = SSC_LOGGING_OFF;
    modelParameters_1.mRTWModifiedTimeStamp = 6.89600899E+8;
    modelParameters_1.mZcDisabled = true;
    modelParameters_1.mUseModelRefSolver = false;
    modelParameters_1.mTargetFPGAHIL = false;
    tmp_2 = 0.001;
    modelParameters_1.mSolverTolerance = tmp_2;
    tmp_2 = 0.001;
    modelParameters_1.mFixedStepSize = tmp_2;
    tmp_0 = false;
    modelParameters_1.mVariableStepSolver = tmp_0;
    tmp_0 = false;
    modelParameters_1.mIsUsingODEN = tmp_0;
    modelParameters_1.mZcDisabled = true;
    diagnosticManager = (NeuDiagnosticManager *)rtDW.OUTPUT_1_1_DiagMgr;
    diagnosticTree_1 = neu_diagnostic_manager_get_initial_tree(diagnosticManager);
    tmp_3 = nesl_initialize_simulator((NeslSimulator *)rtDW.OUTPUT_1_1_Simulator,
      &modelParameters_1, diagnosticManager);
    if (tmp_3 != 0L) {
      tmp_0 = error_buffer_is_empty(rtmGetErrorStatus(rtM));
      if (tmp_0) {
        msg_1 = rtw_diagnostics_msg(diagnosticTree_1);
        rtmSetErrorStatus(rtM, msg_1);
      }
    }

    /* End of Start for SimscapeExecutionBlock: '<S13>/OUTPUT_1_1' */
  }
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
