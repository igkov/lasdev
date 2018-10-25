;/**************************************************************************//**
; * @file     startup_LPC13xx.s
; * @brief    CMSIS Cortex-M3 Core Device Startup File
; *           for the NXP LPC13xx Device Series
; * @version  V1.20
; * @date     03. May 2011
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * @note
; * Copyright (C) 2009-2010 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors

__Vectors       DCD     0xAB6E37F4                ; 000: TAG
                DCD     0x00000000                ; 004: Size
                DCD     0x00000000                ; 008: CRC32
                DCD     0x00000001                ; 012: HW Version
                DCD     __initial_sp              ; 016: Stack Pointer
                DCD     0x00000000                ; 020: RES1
                DCD     0x00000000                ; 024: RES2
                DCD     0x00000000                ; 028: RES3

                DCD     Reset_Handler             ; 032: Reset Handler
                DCD     SysTick_Handler           ; 036: SysTick Handler
                DCD     NMI_Handler               ; 040: NMI Handler
                DCD     HardFault_Handler         ; 044: Hard Fault Handler
                DCD     MemManage_Handler         ; 048: MPU Fault Handler
                DCD     BusFault_Handler          ; 052: Bus Fault Handler
                DCD     UsageFault_Handler        ; 056: Usage Fault Handler
                DCD     SVC_Handler               ; 060: SVCall Handler
                DCD     DebugMon_Handler          ; 064: Debug Monitor Handler
                DCD     PendSV_Handler            ; 068: PendSV Handler
                DCD     WAKEUP_IRQHandler         ; 072: Wakeup PIO0.0
                DCD     I2C_IRQHandler            ; 076: I2C
                DCD     TIMER16_0_IRQHandler      ; 080: 16-bit Counter-Timer 0
                DCD     TIMER16_1_IRQHandler      ; 084: 16-bit Counter-Timer 1
                DCD     TIMER32_0_IRQHandler      ; 088: 32-bit Counter-Timer 0
                DCD     TIMER32_1_IRQHandler      ; 092: 32-bit Counter-Timer 1
                DCD     SSP0_IRQHandler           ; 096: SSP0
                DCD     UART_IRQHandler           ; 100: UART
                DCD     USB_IRQHandler            ; 104: USB IRQ
                DCD     USB_FIQHandler            ; 108: USB FIQ
                DCD     ADC_IRQHandler            ; 112: A/D Converter
                DCD     WDT_IRQHandler            ; 116: Watchdog Timer
                DCD     BOD_IRQHandler            ; 120: Brown Out Detect
                DCD     FMC_IRQHandler            ; 124: IP2111 Flash Memory Controller
                DCD     PIOINT3_IRQHandler        ; 128: PIO INT3
                DCD     PIOINT2_IRQHandler        ; 132: PIO INT2
                DCD     PIOINT1_IRQHandler        ; 136: PIO INT1
                DCD     PIOINT0_IRQHandler        ; 140: PIO INT0
                DCD     0x00000000                ; 144: PIO INT0
                DCD     0x00000000                ; 148: PIO INT0
                DCD     0x00000000                ; 152: PIO INT0
                DCD     0x00000000                ; 156: PIO INT0
													
                DCD     0x00000000                ; 160: PIO INT0
                DCD     0x00000000                ; 164: PIO INT0
                DCD     0x00000000                ; 168: PIO INT0
                DCD     0x00000000                ; 172: PIO INT0
                DCD     0x00000000                ; 176: PIO INT0
                DCD     0x00000000                ; 180: PIO INT0
                DCD     0x00000000                ; 184: PIO INT0
                DCD     0x00000000                ; 188: PIO INT0
                DCD     0x00000000                ; 192: PIO INT0
                DCD     0x00000000                ; 196: PIO INT0
                DCD     0x00000000                ; 200: PIO INT0
                DCD     0x00000000                ; 204: PIO INT0
                DCD     0x00000000                ; 208: PIO INT0
                DCD     0x00000000                ; 212: PIO INT0
                DCD     0x00000000                ; 216: PIO INT0
                DCD     0x00000000                ; 220: PIO INT0
                DCD     0x00000000                ; 224: PIO INT0
                DCD     0x00000000                ; 228: PIO INT0
                DCD     0x00000000                ; 232: PIO INT0
                DCD     0x00000000                ; 236: PIO INT0
                DCD     0x00000000                ; 240: PIO INT0
                DCD     0x00000000                ; 244: PIO INT0
                DCD     0x00000000                ; 248: PIO INT0
                DCD     0x00000000                ; 252: PIO INT0

                AREA    |.text|, CODE, READONLY

; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  __main
                LDR     R0, =__main
                BX      R0
                ENDP

SoftwareReset   PROC
                EXPORT  SoftwareReset
				MOV     R0, #0x00
                LDR     SP, [R0]
                MOV     R0, #0x04
                LDR     R0, [R0]
				BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)                

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC

                EXPORT  WAKEUP_IRQHandler         [WEAK]
                EXPORT  I2C_IRQHandler            [WEAK]
                EXPORT  TIMER16_0_IRQHandler      [WEAK]
                EXPORT  TIMER16_1_IRQHandler      [WEAK]
                EXPORT  TIMER32_0_IRQHandler      [WEAK]
                EXPORT  TIMER32_1_IRQHandler      [WEAK]
                EXPORT  SSP0_IRQHandler           [WEAK]
                EXPORT  UART_IRQHandler           [WEAK]
                EXPORT  USB_IRQHandler            [WEAK]
                EXPORT  USB_FIQHandler            [WEAK]
                EXPORT  ADC_IRQHandler            [WEAK]
                EXPORT  WDT_IRQHandler            [WEAK]
                EXPORT  BOD_IRQHandler            [WEAK]
                EXPORT  FMC_IRQHandler            [WEAK]
                EXPORT  PIOINT3_IRQHandler        [WEAK]
                EXPORT  PIOINT2_IRQHandler        [WEAK]
                EXPORT  PIOINT1_IRQHandler        [WEAK]
                EXPORT  PIOINT0_IRQHandler        [WEAK]
                EXPORT  SSP1_IRQHandler           [WEAK]

WAKEUP_IRQHandler
I2C_IRQHandler
TIMER16_0_IRQHandler
TIMER16_1_IRQHandler
TIMER32_0_IRQHandler
TIMER32_1_IRQHandler
SSP0_IRQHandler
UART_IRQHandler
USB_IRQHandler
USB_FIQHandler
ADC_IRQHandler
WDT_IRQHandler
BOD_IRQHandler
FMC_IRQHandler
PIOINT3_IRQHandler
PIOINT2_IRQHandler
PIOINT1_IRQHandler
PIOINT0_IRQHandler
SSP1_IRQHandler

                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB
                
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                
                ELSE
                
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap
__user_initial_stackheap

                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR

                ALIGN

                ENDIF


                END
