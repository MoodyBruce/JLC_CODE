#include "timer.h"
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "main.h"
#include "sht30.h"

TIM_HandleTypeDef htim3;

/***************************************************************
*
*���ܣ���ʱ��3��ʼ��	 
*��������
*
*****************************************************************/ 
void TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 4799;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  HAL_TIM_Base_Init(&htim3);
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
  
  HAL_TIM_Base_Start_IT(&htim3);    //ʹ�ܶ�ʱ�������жϣ�ʹ�ܶ�ʱ��
}

/***************************************************************
*
*���ܣ���ʱ���ײ���������HAL_TIM_Base_Init�ص���	 
*��������
*
*****************************************************************/ 
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM3)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM3_CLK_ENABLE();
    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
  }

}
/***************************************************************
*
*���ܣ���ʱ��3�жϷ�����	 
*��������
*
*****************************************************************/ 
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3); //����HAL�⴦���жϱ�־λ������Ȳ���
}

/***************************************************************
*
*���ܣ���ʱ���жϻص�����	 
*��������
*
*****************************************************************/ 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static u8 times = 0;
    if(htim==(&htim3))
    {
        times ++;
        if(times == 5)
        {
            sensor_flag = 1;
            times = 0;
        }            
    }
}


