.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

.word _sdata
.word _edata

.word _sbss
.word _ebss

.global vectors

.section .isr_vector,"a",%progbits
  .type vectors, %object
  .size vectors, .-vectors

vectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0  
  .word 0
  .word SVC_Handler
  .word 0
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0



.section .text.Reset_Handler
  .weak Reset_Handler
  .type Reset_Handler, %function

Reset_Handler:
  movs r1, #0
  b loop_copy_data

copy_data:
  ldr r3, =_sidata
  ldr r3, [r3, r1]
  str r3, [r0, r1]
  adds r1, r1, #4

loop_copy_data:
  ldr r0, =_sdata
  ldr r3, =_edata
  adds r2,r0, r1
  cmp r2, r3
  bcc copy_data
  b loop_clear_bss

clear_bss:
  movs r3, #0
  str r3, [r2], #4

loop_clear_bss:
   ldr r3, = _ebss 
   cmp r2, r3
   bcc clear_bss

   bl system_init

   bl main

.size Reset_Handler, .-Reset_Handler
