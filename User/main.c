/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"
/* 开发板硬件bsp头文件 */
#include "LED.h"
#include "Serial.h"
#include "Key.h"
#include "queue.h"

static TaskHandle_t AppTaskCreate_Handle=NULL;
static TaskHandle_t Receive_Task_Handle = NULL;/* LED任务句柄 */
static TaskHandle_t Send_Task_Handle = NULL;/* KEY任务句柄 */

QueueHandle_t Test_Queue=NULL;

#define QUEUE_LEN  4
#define QUEUE_SIZE  4

static void AppTaskCreate(void);/* 用于创建任务 */
 
static void Receive_Task(void* pvParameters);/* Receive_Task任务实现 */
static void Send_Task(void* pvParameters);/* Send_Task任务实现 */
 
static void BSP_Init(void);


int main (void)
{
	BaseType_t xReturn=pdPASS;
	BSP_Init();
	printf("这是一个FreeRTOS消息队列实验！\r\n");
  printf("按下KEY1或者KEY2发送队列消息\r\n");
  printf("Receive任务接收到消息在串口回显\r\n");
	
	xReturn=xTaskCreate((TaskFunction_t)AppTaskCreate,
											(const char *)"AppTaskCreate",
											(uint16_t)512,
											(void *)NULL,
												(UBaseType_t)1,
											(TaskHandle_t*)&AppTaskCreate_Handle);
	
  /* 启动任务调度 */           
  if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;  
  
  while(1);   /* 正常不会执行到这里 */ 

}


static void AppTaskCreate(void)
{
	BaseType_t xReturn=pdPASS;
	taskENTER_CRITICAL();
	Test_Queue=xQueueCreate((UBaseType_t) QUEUE_LEN,
													(UBaseType_t)QUEUE_SIZE);
	if(Test_Queue!=NULL)
		printf("创建Test_Queue消息队列成功!\r\n");
	
  /* 创建Receive_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Receive_Task, /* 任务入口函数 */
                        (const char*    )"Receive_Task",/* 任务名字 */
                        (uint16_t       )512,   /* 任务栈大小 */
                        (void*          )NULL,	/* 任务入口函数参数 */
                        (UBaseType_t    )2,	    /* 任务的优先级 */
                        (TaskHandle_t*  )&Receive_Task_Handle);/* 任务控制块指针 */
  if(pdPASS == xReturn)
    printf("创建Receive_Task任务成功!\r\n");
  
  /* 创建Send_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Send_Task,  /* 任务入口函数 */
                        (const char*    )"Send_Task",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )3, /* 任务的优先级 */
                        (TaskHandle_t*  )&Send_Task_Handle);/* 任务控制块指针 */ 
  if(pdPASS == xReturn)
    printf("创建Send_Task任务成功!\r\n");
  
  vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
  
  taskEXIT_CRITICAL();            //退出临界区

}

static void Receive_Task(void* parameter)
{	
  BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
  uint32_t r_queue;	/* 定义一个接收消息的变量 */
  while (1)
  {
    xReturn = xQueueReceive( Test_Queue,    /* 消息队列的句柄 */
                             &r_queue,      /* 发送的消息内容 */
                             portMAX_DELAY); /* 等待时间 一直等 */
    if(pdTRUE == xReturn)
      printf("本次接收到的数据是%d\r\n",r_queue);
    else
      printf("数据接收出错,错误代码0x%lx\r\n",xReturn);
  }
}

//static void Send_Task(void* parameter)
//{	 
//  BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
//  uint32_t send_data1 = 1;
//  uint32_t send_data2 = 2;
//  while (1)
//  {
//    if(Key_GetNum()==1)
//    {/* K1 被按下 */
//      printf("发送消息send_data1！\r\n");
//      xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
//                            &send_data1,/* 发送的消息内容 */
//                            0 );        /* 等待时间 0 */
//      if(pdPASS == xReturn)
//        printf("消息send_data1发送成功!\r\n");
//    } 
//		vTaskDelay(50);
//    if( Key_GetNum()==2)
//    {/* K2 被按下 */
//      printf("发送消息send_data2！\r\n");
//      xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
//                            &send_data2,/* 发送的消息内容 */
//                            0 );        /* 等待时间 0 */
//      if(pdPASS == xReturn)
//        printf("消息send_data2发送成功!\r\n");
//    }
//    vTaskDelay(20);/* 延时20个tick */
//  }
//}

static void Send_Task(void* parameter)
{	 
    BaseType_t xReturn;
    uint32_t send_data1 = 1;
    uint32_t send_data2 = 2;
    uint8_t key;
    while (1)
    {
        key = Key_GetNum();          // 只调用一次
        if(key == 1)
        {
            printf("发送消息send_data1！\r\n");
            xReturn = xQueueSend(Test_Queue, &send_data1, 0);
            if(pdPASS == xReturn) printf("消息send_data1发送成功!\r\n");
        }
        else if(key == 2)
        {
            printf("发送消息send_data2！\r\n");
            xReturn = xQueueSend(Test_Queue, &send_data2, 0);
            if(pdPASS == xReturn) printf("消息send_data2发送成功!\r\n");
        }
        vTaskDelay(20);
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
 
 
