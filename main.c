#include <stdint.h>
#include "main.h"

int osCurrentTask = 0;
uint32_t osTickCount = 0;
osTaskControlBlock_t osUserThreads[MAX_TASKS];

/**
 * @brief   Creates are new task thread.
 *
 * @param[in]  osTCB_TaskHandler      Pointer to function that is starting point to this task thread.
 * 
 * @return      Task number from the task Control block or -1 if no more task can b created.
 */
void osInitializeTCB(void){
    for(int i=0;i<MAX_TASKS;i++){
        osUserThreads[i].osTCB_CurrentState = TASK_INVALID_STATE;
    }
}

/**
 * @brief   Creates are new task thread.
 *
 * @param[in]  osTCB_TaskHandler      Pointer to function that is starting point to this task thread.
 * 
 * @return      Task number from the task Control block or -1 if no more task can b created.
 */
uint32_t osCreateThread(void (*osTCB_TaskHandler)(void)){
    uint32_t *pPSP;
    for(int i=0;i<MAX_TASKS;i++){
        if(osUserThreads[i].osTCB_CurrentState == TASK_INVALID_STATE){
            osUserThreads[i].osTCB_TaskHandler = osTCB_TaskHandler;
            osUserThreads[i].osTCB_CurrentState = TASK_READY_STATE;
            osUserThreads[i].osTCB_PSPValue = SRAM_END - (i+1)*TASK_STACK_SIZE;
            pPSP = (uint32_t*) osUserThreads[i].osTCB_PSPValue;
            pPSP--; //XPSR
            *pPSP = (uint32_t)0x01000000;
            pPSP--; //PC
            *pPSP = (uint32_t)osUserThreads[i].osTCB_TaskHandler;
            pPSP--; //LR
            *pPSP = (uint32_t)0xFFFFFFFD;
            for(int j=0;j<13;j++){
                 pPSP--;
                *pPSP = 0;
            }
            osUserThreads[i].osTCB_PSPValue=(uint32_t)pPSP;
            return i;
        }
    }
    return -1;
}

/**
 * @brief   Deletes current task thread. Needs to be called from the thread that has to terminate.
 */
void osDeleteThread(){
    osUserThreads[osCurrentTask].osTCB_CurrentState = TASK_INVALID_STATE;
    //trigger pendsv exception
    *SCB_ICSR |= (1<<28);
}

/**
 * @brief   Scans the character input on USART2 until newline is hit by user.
 *
 * @param[out]  string      Pointer to character array.
 * @param[in]   length      Max length of character array.
 * 
 * @return      length of the chracter array from user input.
 */
uint32_t osScan(char *string, uint32_t length){
    //
    uint32_t i=0;
    while (i < length){
        if(((*USART2)>>5)&0x1){         /* If data received*/   
            string[i] = *USART2_DR;
            *USART2 &= ~(1<<5);         /* clear the receive bit.*/
            while(1){    
                if((((*USART2>>6)&0x1)&((*USART2>>7)&0x1))){    /* If ready to transmit*/
                *USART2_DR = string[i];
                break;
                }
            }
            //delay(1);
            if(string[i]=='\r'){
                while(1){      
                    if((((*USART2>>6)&0x1)&((*USART2>>7)&0x1))){    /* If ready to transmit*/
                    *USART2_DR = '\n';
                    break;
                    }
                }
                return i;
            }
            i++;
        }
    }
    return i;
}

/**
 * @brief   Prints the character array by transmitting it on USART2.
 *
 * @param[out]  string      Pointer to character array.
 * @param[in]   length      Length of character array.
 */
void osPrint(char *string, uint32_t length){
    //
    uint32_t i=0;
    while (i < length){
        while(1){    
            if((((*USART2>>6)&0x1)&((*USART2>>7)&0x1))){    /* If ready to transmit*/
            *USART2_DR = string[i];
            break;
            }
        }
        i++;
    }
}

/**
 * @brief   Prints linebreak on user terminal by transmitting it on USART2.
 */
void osPrintLineBreak(){
    while(1){      
        if((((*USART2>>6)&0x1)&((*USART2>>7)&0x1))){    /* If ready to transmit*/
        *USART2_DR = '\r';
        break;
        }
    }
    while(1){      
        if((((*USART2>>6)&0x1)&((*USART2>>7)&0x1))){    /* If ready to transmit*/
        *USART2_DR = '\n';
        break;
        }
    }
}

/**
 * @brief   Initialize USART2. Steps are to 
 *              Enable APB1 bus 
 *              Set GPIO pins to AF 
 *              Specify AF as USARTs 
 *              Enable USART
 *              Set-up Bitrate
 *              Enable both transmit ahd receive functionality of USART2
 */
void osInitializeUART2(){
    *RCC_APB1ENR |= (1<<17);    /*Enable APB1*/
    *RCC_APB1RSTR |= (1<<17);   /*Reset APB1*/
    *RCC_APB1RSTR &= ~(1<<17);

    *GPIOA_MODER |= (0b10 << 4);   /*Set GPIOA pin2 AF*/
    *GPIOA_MODER |= (0b10 << 6);   /*Set GPIOA pin3 AF*/

    *GPIOA_AFRL |= (0b0111 << 8);   /*AF7 for A2*/
    *GPIOA_AFRL |= (0b0111 << 12);  /*AF7 for A3*/

    *USART2_CR1 |= (1<<13);         /*Enable USART2*/

    *USART2_BRR |= (0x8B);          /*set bit rate 16 000 000/(16*115200)=8.6875=0x8.B */
    *USART2_CR1 |= (0b11<<2);       /*Enable transmit and receive in USART2*/
}

/**
 * @brief   Enable different processor faults. And lower the priority of PendSV so that it does not pre-empt any other interrupt.
 *          This would help in appropraiet tailchaining of interrupts.
 */
void osEnableProcessorFaults(void){
    *SCB_SHCSR |=(1<<16); //mem manage
    *SCB_SHCSR |=(1<<17); //bus fault
    *SCB_SHCSR |=(1<<18); //usage fault

    //Lower ythe priority of PendSV
    *SCB_SHPR3 |=(1<<23);
    *SCB_SHPR3 |=(1<<22);
    *SCB_SHPR3 |=(1<<21);
    *SCB_SHPR3 |=(1<<20);
}

/**
 * @brief   Gets the PSP value of current task from the Task Control block.
 * 
 * @return      PSP value of current task.
 */
uint32_t osGetPSPValue(void){
    return osUserThreads[osCurrentTask].osTCB_PSPValue;
}

/**
 * @brief   Saves the PSP value of current task to the Task Control block.
 *
 * @param[in]  stack_addr      PSP value of current task.
 */
void osSavePSPValue(uint32_t stack_addr){
    osUserThreads[osCurrentTask].osTCB_PSPValue = stack_addr;
}

/**
 * @brief   Selects the next task. global variable osCurrentTask is updated to indicate the next task. 
 *          This is round-robin pre-emption algorithm. You may specify different algorithm here.
 */
void osGetNextTask(void){    
    //Make sure to pick up ready tasks
    for(int i=0;i<MAX_TASKS;i++){
        osCurrentTask++;
        osCurrentTask %= MAX_TASKS;
        if(osUserThreads[osCurrentTask].osTCB_CurrentState == TASK_READY_STATE){
            break;
        }
    }
}

/**
 * @brief   Sets up Process Stack Pointer(PSP) value. This will be different from Master Stack Pointer(MSP) and will enable to use threads.
 */
__attribute__((naked)) void osSwitchSPtoPSP(void){
    //initialize PSP to task1
    __asm volatile("PUSH {LR}");
    __asm volatile("BL osGetPSPValue");
    __asm volatile("POP {LR}");
    __asm volatile("MSR PSP,R0");
    

    //change SP to PSP using CONTROL
    __asm volatile("MOV R0,#0X02");
    __asm volatile("MSR CONTROL,R0");   
    __asm volatile("BX LR");
}

/**
 * @brief   Sets up systick counter and enables it to create exceptions.
 *
 * @param[in]   tick_hz      Specifies the tick frequency. Currently set to 1000Hz or 1 millisecond.
 */
void osInitializeSystick(uint32_t tick_hz){
    uint32_t count_value = (SYSTICK_TIM_CLK/tick_hz)-1;

    //load the value in SVR
    *STK_LOAD = count_value;

    //update SCSR to trigger systick interrupts
    *STK_SCSR |= (1 << 1); //Enables systick exception request
    *STK_SCSR |= (1 << 2); //Indicates internal processor clock source
    *STK_SCSR |= (1 << 0); //Enables the counter
}

/**
 * @brief   Creates delays for specified tick count. Each tick count is a millisecond
 *
 * @param[in]   tick_count      Tick count until which the thread is delayed.
 */
void osDelay(uint32_t tick_count){
    if(osCurrentTask){       //osCurrentTask == 0 means it is an idle task
        osUserThreads[osCurrentTask].osTCB_BlockCount = osTickCount + tick_count;
        osUserThreads[osCurrentTask].osTCB_CurrentState = TASK_BLOCKED_STATE;

        //trigger pendsv exception
        *SCB_ICSR |= (1<<28);
    }    
}

/**
 * All Exception or interrupt handlers to be specified below. The function address is used in assembly file.
 */
/**
 * @brief   SysTick Handler. Increments a global tick count and Updates thread status if they are waiting in delay().
 */
void SysTick_Handler(void){
    //Update global tick count
    osTickCount++;

    //unblock tasks
    for(int i=0;i<MAX_TASKS;i++){
        if(osUserThreads[i].osTCB_CurrentState == TASK_BLOCKED_STATE){
            if(osUserThreads[i].osTCB_BlockCount == osTickCount){
                osUserThreads[i].osTCB_CurrentState = TASK_READY_STATE;
            }
        }
    }
    
    //trigger pendsv exception
    *SCB_ICSR |= (1<<28);
}

/**
 * @brief   PendSV Handler. Context switch happens here. This calls function osGetNextTask() to get the next task/thread.
 */
__attribute__((naked)) void PendSV_Handler(void){
    //Disable Interrupts
    __asm volatile("CPSID I");
    //Save Context of current task
    //get current taks PSP
    __asm volatile("MRS R0,PSP");
    //using the psp value store SF2(R4 to R11)
    __asm volatile("STMDB R0!,{R4-R11}");
    //save current value of PSP
    __asm volatile("PUSH {LR}");
    __asm volatile("BL osSavePSPValue");

    //Retrieve context of next task
    //decide next task to run
    __asm volatile("BL osGetNextTask");
    //get next tasks PSP value
    __asm volatile("BL osGetPSPValue");
    __asm volatile("POP {LR}");
    //using PSP retrieve SF2
    __asm volatile("LDMIA R0!,{R4-R11}");
    //update PSP and exit
    __asm volatile("MSR PSP,R0");
    //Enable Interrupts
    __asm volatile("CPSIE I");
    
    __asm volatile("BX LR");
}

//Fault Handlers
void HardFault_Handler(void){
    while(1);
}
void MemManage_Handler(void){
    while(1);
}
void BusFault_Handler(void){
    while(1);
}

void NMI_Handler(void){
    while(1);
}

void UsageFault_Handler(void){
    while(1);
}

void SVC_Handler(void){
    while(1);
}

void DebugMon_Handler(void){
    while(1);
}

/**
 * All functions to create new threads specified below.
 */
void task1(void){
    int i =0;
    //int i=osCreateThread(task2);   
    osDeleteThread(); 
    while(1){
        i++;
        osDelay(1000);
    }
}

void task2(void){
    while(1){
        *GPIOA_ODR |= (1<<5);       /* Turn LED GPIOA 5 on */
        osDelay(1000);
        *GPIOA_ODR &= ~(1<<5);      /* Turn LED GPIOA 5 off */
        osDelay(1000);
    } 
}

void idle_task(void){
    char string[50];
    uint32_t len=0;
    
    osInitializeUART2();
    osPrintLineBreak();
    osPrint("Hello Coder! Welcome to Das OS",30);
    osPrintLineBreak();
    osPrint(">",1);
    

    while(1){
        len=osScan(string,50); 
        osPrint("No command implemented yet : ",29);   
        osPrint(string,len);
        osPrintLineBreak();
        osPrint(">",1);
    }
}

/**
 * C main() function. Entry from assembly.
 */
int main(void){
    *RCC_AHB1ENR = 1;           /*Enable GPIOA*/
    *RCC_AHB1RSTR = 1;          /*Reset GPIOA*/
    *RCC_AHB1RSTR = 0;
    *GPIOA_MODER |= (1 << 10);   /*Set GPIOA pin5 mode to output*/

    *GPIOA_ODR |= (1 << 5);     /*turn LED on*/
    
    osEnableProcessorFaults();
    osCreateThread(idle_task);
    osCreateThread(task1);
    osCreateThread(task2);
    osInitializeSystick(TICK_HZ);
    osSwitchSPtoPSP();
    idle_task();

    while(1);
    return 0;
}