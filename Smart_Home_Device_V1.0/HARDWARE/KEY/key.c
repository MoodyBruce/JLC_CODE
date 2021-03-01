#include "key.h"
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "wifi.h"
#include "main.h"
#include "ri.h"
#include "wkup.h"



/***************************************************************
*
*���ܣ�������ʼ��	 
*��������
*
*****************************************************************/ 
void KEY_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /*Configure GPIO pin : WK_UP_Pin */
  GPIO_InitStruct.Pin = WK_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(WK_UP_GPIO_Port, &GPIO_InitStruct);

  WKUP_Init();                   //��ʼ����������
  
  /*Configure GPIO pins : KEY0_Pin KEY1_Pin */
  GPIO_InitStruct.Pin = KEY0_Pin|KEY1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  
  
  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
  

}


/***************************************************************
*
*���ܣ��жϷ�����	 
*��������
*
*****************************************************************/ 
void EXTI0_1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(WK_UP_Pin);		//�����жϴ����ú���
}

void EXTI4_15_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY0_Pin);		//�����жϴ����ú���
    HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
}



/***************************************************************
*
*���ܣ��жϻص���������д��	 
*���������жϷ�������е��ã���HAL�������е��ⲿ�жϷ�����������ô˺���
*
*****************************************************************/ 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    u8 times = 0;
    delay_ms(100);      //����
    switch(GPIO_Pin)
    {
        case GPIO_PIN_0:
            if(WK_UP_R==1) //����
            {
               if(Check_WKUP())//�ػ�
               {
                 Sys_Enter_Standby();//�������ģʽ
               }
                

            }
            break;
        case GPIO_PIN_4:
            if(KEY0_R==0)  //�ɿ�����������wifi
            {
                while(!KEY0_R);
                mcu_reset_wifi();
                
            }
            break;
        case GPIO_PIN_5:
            if(KEY1_R==0)  //��������WIFI����ģʽ
            {
                while(!KEY1_R)    //����LED0��ʼ��˸
                {
                    delay_ms(500);
                    times++;
                }
                if(times < 3)   mcu_set_wifi_mode(0);   //��˸3������ Smartģʽ
                else            mcu_set_wifi_mode(1);   //��˸3������ APģʽ
            }
            break;
    }
}

/***************************************************************
*
*���ܣ�����ɨ��
*������ģʽ 0����֧������ 1��֧������
*
*****************************************************************/
//uint8_t Key_Scan(uint8_t mode)
//{
//    static uint8_t flag = 1;
//    __IO uint8_t IO_WK_UP = HAL_GPIO_ReadPin(WK_UP_GPIO_Port,WK_UP_Pin);
//    __IO uint8_t IO_KEY0 = HAL_GPIO_ReadPin(KEY0_GPIO_Port,KEY0_Pin);
//    
//    if(mode == 1) flag = 1;
//    if(flag == 1 && ((IO_WK_UP == GPIO_PIN_SET)||(IO_KEY0 == GPIO_PIN_RESET)))
//    {   
//        HAL_DelayMs(10);//����
//        flag = 0;
//        if(IO_WK_UP == GPIO_PIN_SET)   return WK_UP;
//        else if(IO_KEY0 == GPIO_PIN_RESET) return KEY0;
//    }else if((IO_WK_UP == GPIO_PIN_RESET)&&(IO_KEY0 == GPIO_PIN_SET)) flag = 1;
//   
//    return 0xff;
//}