/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* 开发板硬件bsp头文件 */
#include "LED.h"
#include "Serial.h"
#include "Key.h"
#include "queue.h"
#include "semphr.h"

static TaskHandle_t AppTaskCreate_Handle=NULL;

static TaskHandle_t Take_Task_Handle = NULL;/* Take_Task任务句柄 */
static TaskHandle_t Give_Task_Handle = NULL;/* Give_Task任务句柄 */

SemaphoreHandle_t CountSem_Handle =NULL;


static void AppTaskCreate(void);/* 用于创建任务 */
 

static void Take_Task(void* pvParameters);/* Take_Task任务实现 */
static void Give_Task(void* pvParameters);/* Give_Task任务实现 */

static void BSP_Init(void);


int main (void)
{
	BaseType_t xReturn=pdPASS;
	
	BSP_Init();

  printf("这是一个FreeRTOS计数信号量实验！\r\n");
  printf("车位默认值为5个，按下KEY1申请车位，按下KEY2释放车位！\r\n");
	

  xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 

  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */ 

}


static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
  
  taskENTER_CRITICAL();           //进入临界区
  
  /* 创建Test_Queue */
  CountSem_Handle = xSemaphoreCreateCounting(10,10);	 
  if(NULL != CountSem_Handle)
    printf("CountSem_Handle计数信号量创建成功!\r\n");
 
  /* 创建Take_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Take_Task, /* 任务入口函数 */
                        (const char*    )"Take_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Take_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建Take_Task任务成功!\r\n");
  
  /* 创建Give_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Give_Task,  /* 任务入口函数 */
                        (const char*    )"Give_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&Give_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建Give_Task任务成功!\r\n");
  
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区
}

static void Take_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
  /* 任务都是一个无限循环，不能返回 */
  while (1)
  {
    //如果KEY1被单击
		if( Key_GetNum()==1)       
		{
			/* 获取一个计数信号量 */
      xReturn = xSemaphoreTake(CountSem_Handle,	/* 计数信号量句柄 */
                             0); 	/* 等待时间：0 */
			if ( pdTRUE == xReturn ) 
				printf( "KEY1被按下，成功申请到停车位。\r\n" );
			else
				printf( "KEY1被按下，不好意思，现在停车场已满！\r\n" );							
		}
		vTaskDelay(20);     //每20ms扫描一次		
  }
}


static void Give_Task(void* parameter)
{	 
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
  /* 任务都是一个无限循环，不能返回 */
  while (1)
  {
    //如果KEY2被单击
		if( Key_GetNum()==2)       
		{
			/* 获取一个计数信号量 */
      xReturn = xSemaphoreGive(CountSem_Handle);//给出计数信号量                  
			if ( pdTRUE == xReturn ) 
				printf( "KEY2被按下，释放1个停车位。\r\n" );
			else
				printf( "KEY2被按下，但已无车位可以释放！\r\n" );							
		}
		vTaskDelay(20);     //每20ms扫描一次	
  }
}


static void BSP_Init(void)

{

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );
	
	/* LED 初始化 */
	LED_Init();
	
	/* 串口初始化	*/
	Serial_Init();
	Key_Init();
  
}
 
 
