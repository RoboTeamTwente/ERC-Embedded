
//common libraries
#include "result.h"

// //packetdispatcher
#include "packet_dispatcher.h"
#include "packet_dispatcher_macros.h"


/* Config for 1 pbmessage: ArmBoardControlSignals */
static result_t Callback_ArmBoardControlSignals(void *buffer) {
    if (buffer == NULL) {
        return RESULT_ERR_INVALID_ARG;
    }
    
    ArmBoardControlSignals* pckt = (ArmBoardControlSignals *)buffer;
    pckt->control_base; //bldc
    pckt->control_gripper_pitch; //bldc
    pckt->control_gripper_rotation; //bldc
    pckt->control_jaw; //bldc
    pckt->stepper_bottom_freq; //ignore
    pckt->stepper_top_freq; //ignore
    pckt->stepper_bottom_rev;
    pckt->stepper_top_rev;
    return RESULT_OK;
}

static packet_handler_config_t Handler_ArmBoardControlSignals;
PACKET_HANDLER_CONFIG_STATIC(Handler_ArmBoardControlSignals, PBEnvelope_arm_ctrl_tag, ArmBoardActualPositions_size, Callback_ArmBoardControlSignals);

// static uint8_t Buffer_ArmBoardControlSignals[ArmBoardActualPositions_size * 5];
// static packet_handler_config_t Handler_ArmBoardControlSignals = {
//     .handler = Callback_ArmBoardControlSignals,
//     .task_name = "Armboard Control Signals",
//     .packet_type = PBEnvelope_arm_ctrl_tag,
//     .item_size = ArmBoardControlSignals_size,
//     .task_priority = tskIDLE_PRIORITY + 2U,
//     .queue_length = 5,
//     .queue_buffer = Buffer_ArmBoardControlSignals};


/* Config for 2 pbmessage: ArmBoardMovementFeedback */
static result_t Callback_ArmBoardMovementFeedback(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  ArmBoardMovementFeedback* pckt = (ArmBoardMovementFeedback *)buffer;

  return RESULT_OK;
}

static uint8_t Buffer_ArmBoardMovementFeedback[ArmBoardActualPositions_size * 5];
static packet_handler_config_t Handler_ArmBoardMovementFeedback = {
    .handler = Callback_ArmBoardMovementFeedback,
    .task_name = "Armboard Movement Feedback",
    .packet_type = PBEnvelope_arm_feedback_tag,
    .item_size = ArmBoardMovementFeedback_size,
    .task_priority = tskIDLE_PRIORITY + 2U,
    .queue_length = 5,
    .queue_buffer = Buffer_ArmBoardMovementFeedback};



/* Config for 3 pbmessage: ArmBoardActualPositions */
//Idk if we need this
static result_t Callback_ArmBoardActualPositions(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  ArmBoardActualPositions* pckt = (ArmBoardActualPositions *)buffer;

  return RESULT_OK;
}

static uint8_t Buffer_ArmBoardActualPositions[ArmBoardActualPositions_size * 5];
static packet_handler_config_t Handler_ArmBoardActualPositions = {
    .handler = Callback_ArmBoardActualPositions,
    .task_name = "Armboard Actual Positions",
    .packet_type = PBEnvelope_arm_pos_tag,
    .item_size = ArmBoardActualPositions_size,
    .task_priority = tskIDLE_PRIORITY + 2U,
    .queue_length = 5,
    .queue_buffer = Buffer_ArmBoardActualPositions};



/* Config for 4 pbmessage: ArmBoardTargetMovement */
static result_t Callback_ArmBoardTargetMovement(void *buffer) {
  if (buffer == NULL) {
    return RESULT_ERR_INVALID_ARG;
  }

  ArmBoardTargetMovement* pckt = (ArmBoardTargetMovement *)buffer;

  return RESULT_OK;
}

static uint8_t Buffer_ArmBoardTargetMovement[ArmBoardTargetMovement_size * 5];
static packet_handler_config_t Handler_ArmBoardTargetMovement = {
    .handler = Callback_ArmBoardTargetMovement,
    .task_name = "Armboard Actual Positions",
    .packet_type = PBEnvelope_arm_pos_tag,
    .item_size = ArmBoardTargetMovement_size,
    .task_priority = tskIDLE_PRIORITY + 2U,
    .queue_length = 5,
    .queue_buffer = Buffer_ArmBoardTargetMovement};



static packet_handler_config_t handler_configs[] = { 
    // Callback_ArmBoardActualPositions
    Callback_ArmBoardControlSignals
    , Callback_ArmBoardMovementFeedback
    // , Callback_ArmBoardTargetMovement
};