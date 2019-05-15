#ifndef IRQ_HANDLER_H
#define IRQ_HANDLER_H

void HW_NMI_Handler( void );
void HW_HardFault_Handler( void );
void HW_MemManage_Handler( void );
void HW_BusFault_Handler( void );
void HW_UsageFault_Handler( void );
void HW_DebugMon_Handler( void );
void HW_FLASH_IRQHandler( void );
void HW_PVD_PVM_IRQHandler( void );	

#endif
