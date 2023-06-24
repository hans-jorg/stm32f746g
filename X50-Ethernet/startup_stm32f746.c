
/**
 * @file     startup_stm32f746.c
 * @brief    startup code according CMSIS
 * @version  V1.0
 * @date     03/10/2020
 *
 * @note     Provides an Interrupt Vector Table to be stored at address 0
 * @note     Provides default routines for interrupts
 * @note     Copy initial values from flash to RAM
 * @note     Calls SystemInit
 * @note     Calls _main (It provides one, but it is automatically redefined)
 * @note     Calls main
 * @note     This code must be adapted for processor and compiler
 * @note     Not tested for C++
 *
 ******************************************************************************/

#include "stm32f746xx.h"

/* main : codigo do usuario */
extern void main(void);

#ifdef __GNUC__
#define WEAK_DEFAULT_ATTRIBUTE  __attribute__((weak,alias("Default_Handler")))
#define WEAK_ATTRIBUTE __attribute__((weak))
#else
#define WEAK_DEFAULT_ATTRIBUTE
#define WEAK_ATTRIBUTE
#endif

/* _main: inicializacao da biblioteca (newlib?) */
void _main(void)                          WEAK_ATTRIBUTE;

/* inicializacao CMSIS  */
void SystemInit(void)                     WEAK_ATTRIBUTE;

/* rotina de interrupcao default */
void Default_Handler(void)                WEAK_ATTRIBUTE;

/* Rotinas para tratamento de excecoes definidas em CMSIS */
/* Devem poder ser redefinidos */
void Reset_Handler(void)                  WEAK_ATTRIBUTE;           /* M0/M0+/M3/M4/M7 */
void NMI_Handler(void)                    WEAK_DEFAULT_ATTRIBUTE;   /* M0/M0+/M3/M4/M7 */
#if 1
void HardFault_Handler(void);                                       /* M0/M0+/M3/M4/M7 */
void HardFault2_Handler(uint32_t *pnt);                             
#else
void HardFault_Handler(void)              WEAK_DEFAULT_ATTRIBUTE;   /* M0/M0+/M3/M4/M7 */
#endif
void SVC_Handler(void)                    WEAK_DEFAULT_ATTRIBUTE;   /* M0/M0+/M3/M4/M7 */
void PendSV_Handler(void)                 WEAK_DEFAULT_ATTRIBUTE;   /* M0/M0+/M3/M4/M7 */
void SysTick_Handler(void)                WEAK_DEFAULT_ATTRIBUTE;   /* M0/M0+/M3/M4/M7 */
void MemManage_Handler(void)              WEAK_DEFAULT_ATTRIBUTE;   /* M3/M4/M7 */
void BusFault_Handler(void)               WEAK_DEFAULT_ATTRIBUTE;   /* M3/M4/M7 */
void UsageFault_Handler(void)             WEAK_DEFAULT_ATTRIBUTE;   /* M3/M4/M7 */
void DebugMon_Handler(void)               WEAK_DEFAULT_ATTRIBUTE;   /* M3/M4/M7 */

/*
 * Implementation dependent interrupt routines
 */
void WWDG_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void PVD_IRQHandler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void RTC_TAMP_STAMP_IRQHandler(void)      WEAK_DEFAULT_ATTRIBUTE;
void RTC_WKUP_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void FLASH_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void RCC_IRQHandler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void EXTI0_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void EXTI1_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void EXTI2_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void EXTI3_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void EXTI4_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream0_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream1_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream2_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream3_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream4_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream5_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA1_Stream6_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void ADC_IRQHandler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void CAN1_TX_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void CAN1_RX0_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void CAN1_RX1_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void CAN1_SCE_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void EXTI9_5_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void TIM1_BRK_TIM9_IRQHandler(void)       WEAK_DEFAULT_ATTRIBUTE;
void TIM1_UP_TIM10_IRQHandler(void)       WEAK_DEFAULT_ATTRIBUTE;
void TIM1_TRG_COM_TIM11_IRQHandler(void)  WEAK_DEFAULT_ATTRIBUTE;
void TIM1_CC_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void TIM2_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void TIM3_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void TIM4_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void I2C1_EV_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void I2C1_ER_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void I2C2_EV_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void I2C2_ER_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void SPI1_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SPI2_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void USART1_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void USART2_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void USART3_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void EXTI15_10_IRQHandler(void)           WEAK_DEFAULT_ATTRIBUTE;
void RTC_Alarm_IRQHandler(void)           WEAK_DEFAULT_ATTRIBUTE;
void OTG_FS_WKUP_IRQHandler(void)         WEAK_DEFAULT_ATTRIBUTE;
void TIM8_BRK_TIM12_IRQHandler(void)      WEAK_DEFAULT_ATTRIBUTE;
void TIM8_UP_TIM13_IRQHandler(void)       WEAK_DEFAULT_ATTRIBUTE;
void TIM8_TRG_COM_TIM14_IRQHandler(void)  WEAK_DEFAULT_ATTRIBUTE;
void TIM8_CC_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void DAM1_Stream7_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void FSMC_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SDMMC1_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void TIM5_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SPI3_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void UART4_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void UART5_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void TIM6_DAC_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void TIM7_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream0_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream1_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream2_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream3_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream4_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void ETH_IRQHandler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void ETH_WKUP_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void CAN2_TX_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void CAN2_RX0_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void CAN2_RX1_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void CAN2_SCE_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void OTG_FS_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream5_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream6_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void DMA2_Stream7_IRQHandler(void)        WEAK_DEFAULT_ATTRIBUTE;
void USART6_IRQHandler(void)              WEAK_DEFAULT_ATTRIBUTE;
void I2C3_EV_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void I2C3_ER_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void OTG_HS_EP1_OUT_IRQHandler(void)      WEAK_DEFAULT_ATTRIBUTE;
void OTG_HS_EP1_IN_IRQHandler(void)       WEAK_DEFAULT_ATTRIBUTE;
void OTG_HS_WKUP_Handler(void)            WEAK_DEFAULT_ATTRIBUTE;
void OTG_HS_Handler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void DCMI_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void CRYP_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void HASH_RNG_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void FPU_IRQHandler(void)                 WEAK_DEFAULT_ATTRIBUTE;
void UART7_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void UART8_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void SPI4_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SPI5_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SPI6_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void SAI1_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void LCD_TFT_EV_IRQHandler(void)          WEAK_DEFAULT_ATTRIBUTE;
void LCD_TFT_ER_IRQHandler(void)          WEAK_DEFAULT_ATTRIBUTE;
void DMA2D_IRQHandler(void)               WEAK_DEFAULT_ATTRIBUTE;
void SAI2_IRQHandler(void)                WEAK_DEFAULT_ATTRIBUTE;
void QUADSPI_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void LP_TIMER1_IRQHandler(void)           WEAK_DEFAULT_ATTRIBUTE;
void HDMI_CEC_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;
void I2C4_EV_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void I2C4_ER_IRQHandler(void)             WEAK_DEFAULT_ATTRIBUTE;
void SPDIF_RX_IRQHandler(void)            WEAK_DEFAULT_ATTRIBUTE;


/**
 * @brief Symbols defined by loader
 *
 */
extern unsigned long _text_start;
extern unsigned long _text_end;
extern unsigned long _data_start;
extern unsigned long _data_end;
extern unsigned long _bss_start;
extern unsigned long _bss_end;
extern unsigned long _stack_start;
//extern unsigned long _stack_end;
extern void(_stack_end)(void);

/**
 * @brief Interrupt vector table
 *
 * @note Must be in section isr_vector, so the loader store it at address 0
 * @note Every routine can be redefined in other module
 * @note All routines must return void and have no parameter
 *
 */

__attribute__ ((weak,section(".isr_vector")))
void(*nvictable[])(void) = {
    _stack_end,                     /* 0 : SP = endereco de stack_top */
    Reset_Handler,                  /* 1 : PC = endereco execucao     */
    NMI_Handler,                    /* 2 : NMI Handler Exception      */
    HardFault_Handler,              /* 3 : Hard Fault Exception       */
#if __CORTEX_M == 0x03 && __CORTEX_M == 0x04
    0,                              /* 4 : reserved                   */
    0,                              /* 5 : reserved                   */
    0,                              /* 6 : reserved                   */
#else
    MemManage_Handler,              /* 4 : Memory Management Exception*/
    BusFault_Handler,               /* 5 : Bus Fault Exception        */
    UsageFault_Handler,             /* 6 : Usage Fault Exception      */
#endif
    0,                              /* 7 : reserved                   */
    0,                              /* 8 : reserved                   */
    0,                              /* 9 : reserved                   */
    0,                              /*10 : reserved                   */
    SVC_Handler,                    /*11 : Software Interrupt         */
    DebugMon_Handler,               /*12 : Debug Monitor              */
    0,                              /*13 : reserved                   */
    PendSV_Handler,                 /*14 : PendSV                     */
    SysTick_Handler,                /*15 : SysTick                    */
    
   /* Implementation dependent interrupt routines                     */
    WWDG_IRQHandler,                /* IRQ =  0 : Window Watchdog interrupt */
    PVD_IRQHandler,                 /* IRQ =  1 : PVD through the EXTI line detection interrupt */
    RTC_TAMP_STAMP_IRQHandler,      /* IRQ =  2 : Tamper and TimeStamp interrupts through the EXTI line */
    RTC_WKUP_IRQHandler,            /* IRQ =  3 : RTC Wakeup interrupt through the EXTI line */
    FLASH_IRQHandler,               /* IRQ =  4 : Flash global interrupt */
    RCC_IRQHandler,                 /* IRQ =  5 : RCC global interrupt */
    EXTI0_IRQHandler,               /* IRQ =  6 : EXTI Line0 interrupt */
    EXTI1_IRQHandler,               /* IRQ =  7 : EXTI Line1 interrupt */
    EXTI2_IRQHandler,               /* IRQ =  8 : EXTI Line2 interrupt */
    EXTI3_IRQHandler,               /* IRQ =  9 : EXTI Line3 interrupt */
    EXTI4_IRQHandler,               /* IRQ = 10 : EXTI Line4 interrupt */
    DMA1_Stream0_IRQHandler,        /* IRQ = 11 : DMA1 Stream0 global interrupt */
    DMA1_Stream1_IRQHandler,        /* IRQ = 12 : DMA1 Stream1 global interrupt */
    DMA1_Stream2_IRQHandler,        /* IRQ = 13 : DMA1 Stream global interrupt */
    DMA1_Stream3_IRQHandler,        /* IRQ = 14 : DMA1 Stream global interrupt */
    DMA1_Stream4_IRQHandler,        /* IRQ = 15 : DMA1 Stream global interrupt */
    DMA1_Stream5_IRQHandler,        /* IRQ = 16 : DMA1 Stream global interrupt */
    DMA1_Stream6_IRQHandler,        /* IRQ = 17 : DMA1 Stream global interrupt */
    ADC_IRQHandler,                 /* IRQ = 18 : ADC1, ADC2 and ADC3 global interrupts */
    CAN1_TX_IRQHandler,             /* IRQ = 19 : CAN1 TX interrupts */
    CAN1_RX0_IRQHandler,            /* IRQ = 20 : CAN1 RX0 interrupts */
    CAN1_RX1_IRQHandler,            /* IRQ = 21 : CAN1 RX1 interrupt */
    CAN1_SCE_IRQHandler,            /* IRQ = 22 : CAN1 SCE interrupt */
    EXTI9_5_IRQHandler,             /* IRQ = 23 : EXTI Line[9:5] interrupts */
    TIM1_BRK_TIM9_IRQHandler,       /* IRQ = 24 : TIM1 Break interrupt and TIM9 global interrupt */
    TIM1_UP_TIM10_IRQHandler,       /* IRQ = 25 : TIM1 Update interrupt and TIM10 global interrupt */
    TIM1_TRG_COM_TIM11_IRQHandler,  /* IRQ = 26 : TIM1 Trigger and Commutation interrupt and TIM11 global interrupt */
    TIM1_CC_IRQHandler,             /* IRQ = 27 : TIM1 Capture Compare interrupt */
    TIM2_IRQHandler,                /* IRQ = 28 : TIM2 global interrupt */
    TIM3_IRQHandler,                /* IRQ = 29 : TIM3 global interrupt */
    TIM4_IRQHandler,                /* IRQ = 30 : TIM4 global interrupt */
    I2C1_EV_IRQHandler,             /* IRQ = 31 : I2C1 event interrupt */
    I2C1_ER_IRQHandler,             /* IRQ = 32 : I2C1 error interrupt */
    I2C2_EV_IRQHandler,             /* IRQ = 33 : I2C2 event interrupt */
    I2C2_ER_IRQHandler,             /* IRQ = 34 : I2C2 error interrupt */
    SPI1_IRQHandler,                /* IRQ = 35 : SPI1 global interrupt */
    SPI2_IRQHandler,                /* IRQ = 36 : SPI2 global interrupt */
    USART1_IRQHandler,              /* IRQ = 37 : USART1 global interrupt */
    USART2_IRQHandler,              /* IRQ = 38 : USART2 global interrupt */
    USART3_IRQHandler,              /* IRQ = 39 : USART3 global interrupt */
    EXTI15_10_IRQHandler,           /* IRQ = 40 : EXTI Line[15:10] interrupts */
    RTC_Alarm_IRQHandler,           /* IRQ = 41 : RTC Alarms(A and B) trhrough EXTI line interrupt */
    OTG_FS_WKUP_IRQHandler,         /* IRQ = 42 : USB On-The-Go FS Wakeup through EXTI line interrupt */
    TIM8_BRK_TIM12_IRQHandler,      /* IRQ = 43 : TIM8 Break and TIM12 global interrupt  */
    TIM8_UP_TIM13_IRQHandler,       /* IRQ = 44 : TIM8 Update and TIM13 global interrupt */
    TIM8_TRG_COM_TIM14_IRQHandler,  /* IRQ = 45 : TIM8 Trigger and Commutation and TIM14 interrupt */
    TIM8_CC_IRQHandler,             /* IRQ = 46 : TIM8 Capture Compare interrupt */
    DAM1_Stream7_IRQHandler,        /* IRQ = 47 : DMA1 Stream7 global interrupt */
    FSMC_IRQHandler,                /* IRQ = 48 : FSMC global interrupt */
    SDMMC1_IRQHandler,              /* IRQ = 49 : SDIO global interrupt */
    TIM5_IRQHandler,                /* IRQ = 50 : TIM5 global interrupt */
    SPI3_IRQHandler,                /* IRQ = 51 : SPI3 global interrupt */
    UART4_IRQHandler,               /* IRQ = 52 : UART4 global interrupt */
    UART5_IRQHandler,               /* IRQ = 53 : UART5 global interrupt */
    TIM6_DAC_IRQHandler,            /* IRQ = 54 : TIM6 interrupt and DAC1 and DAC2 underrun error */
    TIM7_IRQHandler,                /* IRQ = 55 : TIM7 global interrupt */
    DMA2_Stream0_IRQHandler,        /* IRQ = 56 : DMA2 Stream0 global interrupt */
    DMA2_Stream1_IRQHandler,        /* IRQ = 57 : DMA2 Stream1 global interrupt */
    DMA2_Stream2_IRQHandler,        /* IRQ = 58 : DMA2 Stream2 global interrupt */
    DMA2_Stream3_IRQHandler,        /* IRQ = 59 : DMA2 Stream3 global interrupt */
    DMA2_Stream4_IRQHandler,        /* IRQ = 60 : DMA2 Stream4 global interrupt */
    ETH_IRQHandler,                 /* IRQ = 61 : Ethernet global interrupt */
    ETH_WKUP_IRQHandler,            /* IRQ = 62 : Ethernet Wakeup through EXTI global interrupt */
    CAN2_TX_IRQHandler,             /* IRQ = 63 : CAN2 TX interrupts */
    CAN2_RX0_IRQHandler,            /* IRQ = 64 : CAN2 RX0 interrupts */
    CAN2_RX1_IRQHandler,            /* IRQ = 65 : CAN2 RX1 interrupt */
    CAN2_SCE_IRQHandler,            /* IRQ = 66 : CAN2 SCE interrupt */
    OTG_FS_IRQHandler,              /* IRQ = 67 : USB On The Go FS global interrupt */
    DMA2_Stream5_IRQHandler,        /* IRQ = 68 : DMA2 Stream5 global interrupt */
    DMA2_Stream6_IRQHandler,        /* IRQ = 69 : DMA2 Stream6 global interrupt */
    DMA2_Stream7_IRQHandler,        /* IRQ = 70 : DMA2 Stream7 global interrupt */
    USART6_IRQHandler,              /* IRQ = 71 : USART6 global interrupt */
    I2C3_EV_IRQHandler,             /* IRQ = 72 : I2C3 event interrupt */
    I2C3_ER_IRQHandler,             /* IRQ = 73 : I2C3 error interrupt */
    OTG_HS_EP1_OUT_IRQHandler,      /* IRQ = 74 : USB On the Go HS End Point 1 Out global interrupt */
    OTG_HS_EP1_IN_IRQHandler,       /* IRQ = 75 : USB On the Go HS End Point 1 In global interrupt */
    OTG_HS_WKUP_Handler,            /* IRQ = 76 : USB On the Go HS Wakeup through EXTI interrupt */
    OTG_HS_Handler,                 /* IRQ = 77 : USB On the Go HS global interrupt */
    DCMI_IRQHandler,                /* IRQ = 78 : DCMI global interrupt */
    CRYP_IRQHandler,                /* IRQ = 79 : CRYP global interrupt */
    HASH_RNG_IRQHandler,            /* IRQ = 80 : Hash and RNG global interrupt */
    FPU_IRQHandler,                  /* IRQ = 81 : FPU global interrupt */
    UART7_IRQHandler,               /* IRQ = 82 : UART4 global interrupt */
    UART8_IRQHandler,               /* IRQ = 83 : UART5 global interrupt */
    SPI4_IRQHandler,                /* IRQ = 84 : SPI4 global interrupt */
    SPI5_IRQHandler,                /* IRQ = 85 : SPI5 global interrupt */
    SPI6_IRQHandler,                /* IRQ = 86 : SPI6 global interrupt */
    SAI1_IRQHandler,                /* IRQ = 87 : SAI1 global interrupt */
    LCD_TFT_EV_IRQHandler,          /* IRQ = 88 : LCD_TFT_Event global interrupt */
    LCD_TFT_ER_IRQHandler,          /* IRQ = 89 : LCD_TFT Error global interrupt */
    DMA2D_IRQHandler,               /* IRQ = 90 : DMA2D global interrupt */
    SAI2_IRQHandler,                /* IRQ = 91 : SAI2 global interrupt */
    QUADSPI_IRQHandler,             /* IRQ = 92 : QuadSPI global interrupt */
    LP_TIMER1_IRQHandler,           /* IRQ = 93 : LP TImer1 global interrupt */
    HDMI_CEC_IRQHandler,            /* IRQ = 94 : HDMI CEC global interrupt */
    I2C4_EV_IRQHandler,             /* IRQ = 95 : I2C4 Event global interrupt */
    I2C4_ER_IRQHandler,             /* IRQ = 96 : I2C4 Error global interrupt */
    SPDIF_RX_IRQHandler,            /* IRQ = 97 : SPDIFRX global interrupt */
};


static uint32_t InterruptNumber = 0;

/**
 * @brief Default Interrupt Handler routine
 *
 * @note It halts using an infinite loop
 * @note The interrupt source is stored in InterruptNumber variable
 */

void Default_Handler(void) {

    while(1) {} /* Loop */
    /* NEVER */
}

/**
 * @brief Default SystemInit routine
 *
 * @note It can be redefined in other module
 *
 */

void SystemInit(void) {

}

/**
 * @brief Default _main routine
 *
 * @note It can be redefined in other module
 *
 */

void _main(void) {

}

/**
 * @brief _stop routine
 *
 * @note It halts using an infinite loop
 *
 */

void _stop(void) {

    while(1) {}
    /* NEVER */

}

/**
 * @brief Reset Handler
 *
 * @note Copies initial values of variables from FLASH to RAM
 * @note Zeroes uninitialized variables
 * @note Calls SystemInit
 * @note Calls _main
 * @note Call main
 * @note Call _stop if main returns
 */

void __attribute__((weak,naked)) Reset_Handler(void) {
unsigned long *pSource;
unsigned long *pDest;

    /* Step 1 : Copy  data to initialize variable in RAM from Flash */
    pSource = &_text_end;
    pDest   = &_data_start;
    while( pDest < &_data_end ) {
        *pDest++ = *pSource++;
    }

    /* Step 2 : Zero variables in section BSS (non initialized data) */
    pDest = &_bss_start;
    while( pDest < &_bss_end ) {
        *pDest++ = 0;
    }

    /* Step 3 : Call SystemInit conforme CMSIS */
    SystemInit();

    /* Step 4 : Call _main to initialize library */
    _main();

    /* Step 5 : Call main */
    main();

    _stop();
}


#if 1

void HardFault_Handler(void) {

    __asm volatile
    (
        " tst        lr,#4                          \n"
        " ite        eq                             \n"
        " mrseq      r0,msp                         \n"
        " mrsne      r0,psp                         \n"
        " ldr        r1,[r0,#24]                    \n"
        " ldr        r2,hardfault2                  \n"
        " bx         r2                             \n"
        "hardfault2: .word   HardFault2_Handler     \n"
    );
}

void HardFault2_Handler(uint32_t *pnt) {

}
#endif