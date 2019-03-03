
#include "dfu_types.h"

#define ALIAS(f) __attribute__ ((alias (#f)))
  //  register void *user_program_offset asm("r2") = (void *) 0x14000;
void NMI_Handler(void) ALIAS(BranchIntHandler);
void HardFault_Handler(void) ALIAS(BranchIntHandler);
void SVC_Handler(void) ALIAS(BranchIntHandler);
void PendSV_Handler(void) ALIAS(BranchIntHandler);
void SysTick_Handler(void) ALIAS(BranchIntHandler);
void POWER_CLOCK_IRQHandler(void) ALIAS(BranchIntHandler);
void RADIO_IRQHandler(void) ALIAS(BranchIntHandler);

void SPI0_TWI0_IRQHandler(void) ALIAS(BranchIntHandler);
void SPI1_TWI1_IRQHandler(void) ALIAS(BranchIntHandler);

void ADC_IRQHandler(void) ALIAS(BranchIntHandler);
void RTC0_IRQHandler(void) ALIAS(BranchIntHandler);
void TIMER0_IRQHandler(void) ALIAS(BranchIntHandler);
void TIMER1_IRQHandler(void) ALIAS(BranchIntHandler);
void TIMER2_IRQHandler(void) ALIAS(BranchIntHandler);
void TEMP_IRQHandler(void) ALIAS(BranchIntHandler);
void RNG_IRQHandler(void) ALIAS(BranchIntHandler);
void ECB_IRQHandler(void) ALIAS(BranchIntHandler);
void CCM_AAR_IRQHandler(void) ALIAS(BranchIntHandler);
void WDT_IRQHandler(void) ALIAS(BranchIntHandler);

void QDEC_IRQHandler(void) ALIAS(BranchIntHandler);
void LPCOMP_COMP_IRQHandler(void) ALIAS(BranchIntHandler);

void SWI1_IRQHandler(void) ALIAS(BranchIntHandler);
void SWI2_IRQHandler(void) ALIAS(BranchIntHandler);
void SWI3_IRQHandler(void) ALIAS(BranchIntHandler);
void SWI4_IRQHandler(void) ALIAS(BranchIntHandler);
void SWI5_IRQHandler(void) ALIAS(BranchIntHandler);

void UART0_IRQHandler(void) ALIAS(BranchIntHandler);
void RTC1_IRQHandler(void) ALIAS(BranchIntHandler);
void GPIOTE_IRQHandler(void) ALIAS(BranchIntHandler);
void SWI0_IRQHandler(void) ALIAS(BranchIntHandler);


#if defined ( __CC_ARM )
__asm void BranchIntHandler(void) {
            MRS    R0, PSR ;
            /* Mask the interrupt number only */
            MOVS   R1, #0x3F  ; 
            ANDS   R0, R1     ; /*R0 keeps the interrupt number*/
            extern bootloader_active;
            LDR R1 , =bootloader_active;
            LDR R2, [R1];
            CMP R2,#1;
            BEQ bootload;
            LDR     R2, =CODE_REGION_1_START;
            /* Irq address position = IRQ No * 4 */
            LSLS   R0, R0, #2 ;
            /* Fetch the user vector offset */
            LDR    R0, [R0, R2];
            /* Jump to user interrupt vector */
            BX     R0  ;
            
bootload 
            CMP R0, #0x24;
            BEQ swi0
            CMP R0, #0x21;
            BEQ rtc1
            CMP R0, #0x16;
            BEQ gpiote
            CMP R0, #0x12;
            BEQ uart0
            //RETURN code needed
swi0            
            EXTERN SWI0_IRQHandler_Bootloader
            LDR    R0,=SWI0_IRQHandler_Bootloader
            B execute ;    
            
rtc1            
            EXTERN  RTC1_IRQHandler_Bootloader
            LDR    R0,=RTC1_IRQHandler_Bootloader
            B execute ;
            
uart0
            EXTERN  UART0_IRQHandler_Bootloader
            LDR    R0,=UART0_IRQHandler_Bootloader
            B execute ;            
gpiote            
            EXTERN  GPIOTE_IRQHandler_Bootloader
            LDR    R0,=GPIOTE_IRQHandler_Bootloader
                                 
execute            
            BX     R0  ;
            
}
#elif defined ( __GNUC__ )
extern uint8_t bootloader_active;
extern void SWI0_IRQHandler_Bootloader(void);
extern void RTC1_IRQHandler_Bootloader(void);
extern void UART0_IRQHandler_Bootloader(void);
extern void GPIOTE_IRQHandler_Bootloader(void);

void BranchIntHandler(void) __attribute__(( naked ));
void BranchIntHandler(void)
{
    __ASM volatile(
        ".syntax unified\n"
        "mrs    r0, psr\t\n"                            //    MRS    R0, PSR ;
                                                        //    /* Mask the interrupt number only */
        "movs   r1, #0x3f\t\n"                          //    MOVS   R1, #0x3F  ;
        "ands   r0, r1\t\n"                             //    ANDS   R0, R1     ; /*R0 keeps the interrupt number*/
                                                        //    extern bootloader_active;
        "ldr    r1, =bootloader_active\t\n"             //    LDR R1 , =bootloader_active;
        "ldr    r2, [r1]\t\n"                           //    LDR R2, [R1];
        "cmp    r2, #1\t\n"                             //    CMP R2,#1;
        "beq    bootload\t\n"                           //    BEQ bootload;
        "ldr    r2, =%[cd_rgn_st]\t\n"                  //    LDR     R2, =CODE_REGION_1_START;
                                                        //    /* Irq address position = IRQ No * 4 */
        "lsls   r0, r0, #2\t\n"                         //    LSLS   R0, R0, #2 ;
                                                        //    /* Fetch the user vector offset */
        "ldr    r0, [r0, r2]\t\n"                       //    LDR    R0, [R0, R2];
                                                        //    /* Jump to user interrupt vector */
        "bx     r0\t\n"                                 //    BX     R0  ;
                                                        //
        "bootload:\t\n"                                 //bootload
        "cmp    r0, #024\t\n"                           //    CMP R0, #0x24;
        "beq    swi0\t\n"                               //    BEQ swi0
        "cmp    r0, #0x21\t\n"                          //    CMP R0, #0x21;
        "beq    rtc1\t\n"                               //    BEQ rtc1
        "cmp    r0, #0x16\t\n"                          //    CMP R0, #0x16;
        "beq    gpiote\t\n"                             //    BEQ gpiote
        "cmp    r0, #0x12\t\n"                          //    CMP R0, #0x12;
        "beq    uart0\t\n"                              //    BEQ uart0
                                                        //    //RETURN code needed
        "swi0:\t\n"                                     //swi0            
                                                        //    EXTERN SWI0_IRQHandler_Bootloader
        "ldr    r0, =%[swi_irq]\t\n"                    //    LDR    R0,=SWI0_IRQHandler_Bootloader
        "b      execute\t\n"                            //    B execute ;    
                                                        //    
        "rtc1:\t\n"                                     //rtc1            
                                                        //    EXTERN  RTC1_IRQHandler_Bootloader
        "ldr    r0, =%[rtc_irq]\t\n"                    //    LDR    R0,=RTC1_IRQHandler_Bootloader
        "b      execute\t\n"                            //    B execute ;
                                                        //    
        "uart0:\t\n"                                    //uart0
                                                        //    EXTERN  UART0_IRQHandler_Bootloader
        "ldr    r0, =%[uart_irq]\t\n"                   //    LDR    R0,=UART0_IRQHandler_Bootloader
        "b      execute\t\n"                            //    B execute ;            
        "gpiote:\t\n"                                   //gpiote            
                                                        //    EXTERN  GPIOTE_IRQHandler_Bootloader
        "ldr    r0, =%[gpio_irq]\t\n"                   //    LDR    R0,=GPIOTE_IRQHandler_Bootloader
                                                        //                         
        "execute:\t\n"                                  //execute            
        "bx     r0\t\n"                                 //    BX     R0  ;
        ".ltorg\t\n"
                                                        // Argument list for the gcc assembly
        :: [cd_rgn_st] "X" (CODE_REGION_1_START),
           [swi_irq] "X" (SWI0_IRQHandler_Bootloader), 
           [rtc_irq] "X" (RTC1_IRQHandler_Bootloader), 
           [uart_irq] "X" (UART0_IRQHandler_Bootloader), 
           [gpio_irq] "X" (GPIOTE_IRQHandler_Bootloader)
        : "cc", "r0", "r1", "r2"                        // List of register maintained manually.
    );
}
#else
#error Compiler not supported.
#endif

