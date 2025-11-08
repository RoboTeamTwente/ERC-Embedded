## Firmware settings ethernet
(Make sure the gpio pins are setup correctly for every periphial)
1. ETH 
    a. MODE -> RMII
    c. NVIC Settings
        i. check Ethernet global interrupt
2. LWIP
    a. LWIP_DHCP -> disabled
        i. choose ip address, network address and gateway address
    b. Platform Settings
        i. both LAN8742
3. TIM
    a. choose a timer
    b. clock source -> internal clock
    c. Set the clock speed/ prescaler to trigger at a time you want
        i. You divide the clock speed by the (prescaler+1) and then it counts until the counter period
    d. Counter mode -> up
    e. NVIC settings -> update interrupt
4.  RCC
    a. High speed clock -> Bypass clock source
    b. Low speed clcok -> Crystal/ceramic Resonator
    (Don't know exactly why, but this comes from a tutorial)
5. Enable Uart
    (for basic easy setup)
    a. choose a USART
    b. mode -> Asynchronous
6. NVIC
    a. Set Timer, EXTI line and Ethernet interupt to 15.