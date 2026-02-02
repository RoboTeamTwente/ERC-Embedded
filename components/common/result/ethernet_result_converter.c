const result_t DMA_err_to_result(uint32_t dma_err) {

  switch (dma_err) {
  case dma_err &ETH_DMACSR_FBE == 1: // fatal bus error
    return                           // TODO: Hard fault
  }
case dma_err &ETH_DMACSR_TPS:
} // TODO: return watchdog timeout
  // TODO: return RBU if not whatdog timeout
  // TODO: return dma error otherwise
