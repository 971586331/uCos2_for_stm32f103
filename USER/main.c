#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
/************************************************
 ALIENTEK Mini STM32F103开发板实验32
 UOSII实验1-任务调度-HAL库版本
 技术支持：www.openedv.com
 淘宝店铺： http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//START 任务
//设置任务优先级
#define START_TASK_PRIO			0  ///开始任务的优先级为最低
//设置任务堆栈大小
#define START_STK_SIZE			128
//任务任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO			3
//设置任务堆栈大小
#define LED0_STK_SIZE			128
//任务堆栈
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);

//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO			4
//设置任务堆栈大小
#define LED1_STK_SIZE			128
//任务堆栈
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
//任务函数
void led1_task(void *pdata);

OS_EVENT *str_q;
void *MsgGrp[128];

int main(void)
{ 
    HAL_Init();                    	 	//初始化HAL库    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//设置时钟,72M
	delay_init(72);               			//初始化延时函数
	LED_Init();							//初始化LED	
	uart_init(115200);
	
	OSInit();                       	//UCOS初始化
    OSTaskCreateExt((void(*)(void*) )start_task,                //任务函数
                    (void*          )0,                         //传递给任务函数的参数
                    (OS_STK*        )&START_TASK_STK[START_STK_SIZE-1],//任务堆栈栈顶
                    (INT8U          )START_TASK_PRIO,           //任务优先级
                    (INT16U         )START_TASK_PRIO,           //任务ID，这里设置为和优先级一样
                    (OS_STK*        )&START_TASK_STK[0],        //任务堆栈栈底
                    (INT32U         )START_STK_SIZE,            //任务堆栈大小
                    (void*          )0,                         //用户补充的存储区
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);//任务选项,为了保险起见，所有任务都保存浮点寄存器的值
	OSStart(); //开始任务
}

//开始任务
void start_task(void *pdata)
{
    INT8U err;
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //开启统计任务
	
    str_q = OSQCreate(MsgGrp, 128);

	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
    //LED0任务
    OSTaskCreateExt((void(*)(void*) )led0_task,                 
                    (void*          )0,
                    (OS_STK*        )&LED0_TASK_STK[LED0_STK_SIZE-1],
                    (INT8U          )LED0_TASK_PRIO,            
                    (INT16U         )LED0_TASK_PRIO,            
                    (OS_STK*        )&LED0_TASK_STK[0],         
                    (INT32U         )LED0_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);
	//LED1任务
    OSTaskCreateExt((void(*)(void*) )led1_task,                 
                    (void*          )0,
                    (OS_STK*        )&LED1_TASK_STK[LED1_STK_SIZE-1],
                    (INT8U          )LED1_TASK_PRIO,            
                    (INT16U         )LED1_TASK_PRIO,            
                    (OS_STK*        )&LED1_TASK_STK[0],         
                    (INT32U         )LED1_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);

    OS_EXIT_CRITICAL();             //退出临界区(开中断)
	// OSTaskSuspend(START_TASK_PRIO); //挂起开始任务

    static char *s1 = "aaa";
    OSQPostFront(str_q, s1);
    OSQPost(str_q, s1);
    OSQFlush(str_q);
    OSQDel(str_q, 0, &err);
    while (1)
    {
        if( OSTimeGet() > 100 && OSTimeGet() < 200 ) {
            static char *s2 = "bbb";
            OSQPostFront(str_q, s2);
            static char *s3 = "ccc";
            OSQPostFront(str_q, s3);
        }

        if( OSTimeGet() > 500 && OSTimeGet() < 1000 ) {
            static char *s4 = "ddd";
            OSQPostFront(str_q, s4);
            break;
        }
    }
    
}

//LED0任务
void led0_task(void *pdata)
{
    INT8U err;
	while(1)
	{
        char *ss = OSQPend(str_q, 0, &err);
        printf("led0 = %s\n", ss);
        OSTimeDlyHMSM(0, 0, 1, 0);  // 锁调度器后delya不起作用
	};
}

//LED1任务
void led1_task(void *pdata)
{
    INT8U err;
	while(1)
	{
        char *ss = OSQPend(str_q, 0, &err);
        printf("led1 = %s\n", ss);
        OSTimeDlyHMSM(0, 0, 1, 0);
	}
}
