
/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*                   Knowledge of the source code may not be used to write a similar
*                   product.  This file may only be used in accordance with a license
*                   and should not be redistributed in any way.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       IAR Development Kits
*                                              on the
*
*                                    STM32F429II-SK KICKSTART KIT
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : YS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include  <includes.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx.h"

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_TASK_EQ_0_ITERATION_NBR              16u

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
static  void  AppTaskStart          (void     *p_arg);
static  void  AppTaskCreate         (void);
static  void  AppObjCreate          (void);

static void AppTask_usart(void *p_arg);
static void AppTask_ADC(void *p_arg);
static void AppTask_LED(void *p_arg);
static void AppTask_buzzer(void *p_arg);

static void Setup_Gpio(void);
void Buzzer_On();
void Buzzer_off();
int Get_ADC_Converted_Value();


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
/* ----------------- APPLICATION GLOBALS -------------- */
static  OS_TCB   AppTaskStartTCB;
static  CPU_STK  AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB       Task_usart_TCB;
static	OS_TCB		 Task_ADC_TCB;
static	OS_TCB		 Task_LED_TCB;
static	OS_TCB		 Task_buzzer_TCB;

static  CPU_STK  Task_usart_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_ADC_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_LED_Stack[APP_CFG_TASK_START_STK_SIZE];
static  CPU_STK  Task_buzzer_Stack[APP_CFG_TASK_START_STK_SIZE];

OS_Q	ADC_Q;
CPU_INT32U value;

/* ------------ FLOATING POINT TEST TASK -------------- */
/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int main(void)
{
    OS_ERR  err;

    /* Basic Init */
    RCC_DeInit();
//    SystemCoreClockUpdate();
    Setup_Gpio();

    /* BSP Init */
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    CPU_Init();                                                 /* Initialize the uC/CPU Services                       */
    Mem_Init();                                                 /* Initialize Memory Management Module                  */
    Math_Init();                                                /* Initialize Mathematical Module                       */


    /* OS Init */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSQCreate((OS_Q *)&ADC_Q,
    		(CPU_CHAR *)"MY Queue",
			(OS_MSG_QTY)10,
			(OS_ERR *)&err);

    OSTaskCreate((OS_TCB       *)&AppTaskStartTCB,              /* Create the start task                                */
                 (CPU_CHAR     *)"App Task Start",
                 (OS_TASK_PTR   )AppTaskStart,
                 (void         *)0u,
                 (OS_PRIO       )APP_CFG_TASK_START_PRIO,
                 (CPU_STK      *)&AppTaskStartStk[0u],
                 (CPU_STK_SIZE  )AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE / 10u],
                 (CPU_STK_SIZE  )APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY    )0u,
                 (OS_TICK       )0u,
                 (void         *)0u,
                 (OS_OPT        )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR       *)&err);

   OSStart(&err);   /* Start multitasking (i.e. give control to uC/OS-III). */

   (void)&err;

   return (0u);
}
/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/
static  void  AppTaskStart (void *p_arg)
{
    OS_ERR  err;

   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                             */
    BSP_Tick_Init();                                            /* Initialize Tick Services.                            */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running            */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

   // BSP_LED_Off(0u);                                            /* Turn Off LEDs after initialization                   */

   APP_TRACE_DBG(("Creating Application Kernel Objects\n\r"));
   AppObjCreate();                                             /* Create Applicaiton kernel objects                    */

   APP_TRACE_DBG(("Creating Application Tasks\n\r"));
   AppTaskCreate();                                            /* Create Application tasks                             */
}


/*
*********************************************************************************************************
*                                          AppTask_usart

*********************************************************************************************************
*/

static void AppTask_usart(void *p_arg)
{
	OS_ERR err;
		CPU_INT32U adc_value;
		OS_MSG_SIZE size;
		CPU_TS ts;

   char temp[10];
   while(DEF_TRUE){
	   adc_value = (CPU_INT08U)OSQPend((OS_Q *)&ADC_Q,
	   						(OS_TICK)0,
	   						(OS_OPT)OS_OPT_PEND_BLOCKING,
	   						(OS_MSG_SIZE *)&size,
	   						(CPU_TS *)&ts,
	   						(OS_ERR *)&err);

	   send_string("\n\r");
	   send_string("humidity: ");
	   send_string(itoa(adc_value, temp, 10));
	   send_string("\n\r");


      OSTimeDlyHMSM(0u, 0u, 0u, 10u,
                        OS_OPT_TIME_HMSM_STRICT,
                        &err);

   }
   send_string("\n\rReceive end \n\r");
}

/*
*********************************************************************************************************
*                                          AppTask_ADC

*********************************************************************************************************
*/
static void AppTask_ADC(void *p_arg)
{
	OS_ERR err;

	while(DEF_TRUE){
		value = Get_ADC_Converted_Value();

		OSQPost((OS_Q *)&ADC_Q,
				(void *)value,
				(OS_MSG_SIZE)sizeof(void *),
				(OS_OPT)OS_OPT_POST_ALL,
				(OS_ERR *)&err);

		OSTimeDlyHMSM(0u, 0u, 2u, 0u,
						OS_OPT_TIME_HMSM_STRICT,
						&err);
	}
}

/*
*********************************************************************************************************
*                                          AppTask_LED

*********************************************************************************************************
*/
static void AppTask_LED(void *p_arg)
{
	OS_ERR err;
	BSP_LED_On(3);
	BSP_LED_On(2);
	CPU_INT08U adc_value;
	OS_MSG_SIZE size;
	CPU_TS ts;

	while (DEF_TRUE){
		adc_value = (CPU_INT08U)OSQPend((OS_Q *)&ADC_Q,
						(OS_TICK)0,
						(OS_OPT)OS_OPT_PEND_BLOCKING,
						(OS_MSG_SIZE *)&size,
						(CPU_TS *)&ts,
						(OS_ERR *)&err);

		if(adc_value > 200){
			BSP_LED_On(3);
			BSP_LED_Off(2);
		}
		else{
			BSP_LED_Off(3);
			BSP_LED_On(2);
		}

		OSTimeDlyHMSM(0u, 0u, 0u, 10u,
						OS_OPT_TIME_HMSM_STRICT,
						&err);
	}
}

/*
*********************************************************************************************************
*                                          AppTask_buzzer

*********************************************************************************************************
*/
static void AppTask_buzzer(void *p_arg)
{
	OS_ERR err;
	CPU_INT08U adc_value;
	OS_MSG_SIZE size;
	CPU_TS ts;
	Buzzer_off();

	while (DEF_TRUE){
			Buzzer_off();
			adc_value = (CPU_INT08U)OSQPend((OS_Q *)&ADC_Q,
							(OS_TICK)0,
							(OS_OPT)OS_OPT_PEND_BLOCKING,
							(OS_MSG_SIZE *)&size,
							(CPU_TS *)&ts,
							(OS_ERR *)&err);

			if(adc_value > 200){
				Buzzer_On();
				//Buzzer_off();
			}
			else{
				Buzzer_off();
			}

			OSTimeDlyHMSM(0u, 0u, 1u, 0u,
							OS_OPT_TIME_HMSM_STRICT,
							&err);
		}


}


/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
   OS_ERR  err;
  OSTaskCreate((OS_TCB*)&Task_usart_TCB,
          (CPU_CHAR*)"App Task USART Start",
          (OS_TASK_PTR)AppTask_usart,
          (void*)0,
          (OS_PRIO)3,
          (CPU_STK*)&Task_usart_Stack[0],
          (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE/10,
          (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
          (OS_MSG_QTY)0,
          (OS_TICK)0,
          (void*)0,
          (OS_OPT)(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
          (OS_ERR*)&err
          );
  OSTaskCreate((OS_TCB*)&Task_ADC_TCB,
            (CPU_CHAR*)"App Task ADC Start",
            (OS_TASK_PTR)AppTask_ADC,
            (void*)0,
            (OS_PRIO)1,
            (CPU_STK*)&Task_ADC_Stack[0],
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE/10,
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
            (OS_MSG_QTY)0,
            (OS_TICK)0,
            (void*)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err
            );
  OSTaskCreate((OS_TCB*)&Task_LED_TCB,
            (CPU_CHAR*)"App Task LED Start",
            (OS_TASK_PTR)AppTask_LED,
            (void*)0,
            (OS_PRIO)2,
            (CPU_STK*)&Task_LED_Stack[0],
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE/10,
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
            (OS_MSG_QTY)0,
            (OS_TICK)0,
            (void*)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err
            );
  OSTaskCreate((OS_TCB*)&Task_buzzer_TCB,
            (CPU_CHAR*)"App Task buzzer Start",
            (OS_TASK_PTR)AppTask_buzzer,
            (void*)0,
            (OS_PRIO)2,
            (CPU_STK*)&Task_buzzer_Stack[0],
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE/10,
            (CPU_STK_SIZE)APP_CFG_TASK_START_STK_SIZE,
            (OS_MSG_QTY)0,
            (OS_TICK)0,
            (void*)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err
            );

}


/*
*********************************************************************************************************
*                                          AppObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  AppObjCreate (void)
{

}

/*
*********************************************************************************************************
*                                          Setup_Gpio()
*
* Description : Configure LED GPIOs directly
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     :
*              LED1 PB0
*              LED2 PB7
*              LED3 PB14
*
*********************************************************************************************************
*/
static void Setup_Gpio(void)
{
   GPIO_InitTypeDef led_init = {0};

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
   RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   led_init.GPIO_Mode   = GPIO_Mode_OUT;
   led_init.GPIO_OType  = GPIO_OType_PP;
   led_init.GPIO_Speed  = GPIO_Speed_2MHz;
   led_init.GPIO_PuPd   = GPIO_PuPd_NOPULL;
   led_init.GPIO_Pin    = GPIO_Pin_0 | GPIO_Pin_7 | GPIO_Pin_14;

   GPIO_Init(GPIOB, &led_init);


}

int Get_ADC_Converted_Value()
{
   ADC_SoftwareStartConv(ADC1);
   while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
   return ADC_GetConversionValue(ADC1);
}


