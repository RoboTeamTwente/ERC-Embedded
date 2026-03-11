#include "cubemx_main.h"
#include "result.h"
#include "stm32h753xx.h"

const result_t DMA_err_to_result(uint32_t dma_err) {

  if (dma_err & ETH_DMACSR_FBE) { // fatal bus error
    return RESULT_ERR_BUFF;
  }

  if (dma_err & (ETH_DMACSR_RBU | ETH_DMACSR_RPS |
                 ETH_DMACSR_TPS)) { // receive buffer unavailable or stopped or
                                    // stopped transmition
    return RESULT_ERR_BUFF;
  }
  if (dma_err & ETH_DMACSR_RWT) { // Watchdog timeout
    return RESULT_ERR_WATCHDOG;
  }
  if (dma_err & (ETH_DMACSR_CDE | ETH_DMACSR_ETI)) { // non terminal error
    return RESULT_ERR_NON_TERMINAL;
  } else {
    return RESULT_FAIL;
  }
}

// TODO: return watchdog timeout
// TODO: return RBU if not whatdog timeout
// TODO: return dma error otherwise

const result_t ETH_err_to_result(uint32_t eth_err) {

  if (eth_err & HAL_ETH_ERROR_TIMEOUT) {
    return RESULT_ERR_TIMEOUT;
  }
  if (eth_err & HAL_ETH_ERROR_PARAM) {
    return RESULT_ERR_INVALID_ARG;
  }
  if (eth_err & HAL_ETH_ERROR_BUSY) {
    return RESULT_ERR_BUSY;
  }
  if (eth_err & HAL_ETH_ERROR_DMA) {
    return RESULT_ERR_NON_TERMINAL;
  } else {
    return RESULT_FAIL;
  }
}
