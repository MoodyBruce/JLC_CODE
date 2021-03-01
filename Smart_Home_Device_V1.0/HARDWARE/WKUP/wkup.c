#include "wkup.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
//#include "rtc.h"

//ϵͳ�������ģʽ
//ϵͳ�������ģʽ
void Sys_Enter_Standby(void)
{
    __HAL_RCC_AHB_FORCE_RESET();       //��λ����IO�� 
	
	while(WK_UP_R);                     //�ȴ�WK_UP�����ɿ�(����RTC�ж�ʱ,�����WK_UP�ɿ��ٽ������)
   
	__HAL_RCC_PWR_CLK_ENABLE();         //ʹ��PWRʱ��
    __HAL_RCC_BACKUPRESET_FORCE();      //��λ��������
    HAL_PWR_EnableBkUpAccess();         //���������ʹ��  
	
	//STM32F4,��������RTC����жϺ�,�����ȹر�RTC�ж�,�����жϱ�־λ,Ȼ����������
	//RTC�ж�,�ٽ������ģʽ�ſ�����������,�����������.	
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
//    __HAL_RTC_WRITEPROTECTION_DISABLE(&RTC_Handler);//�ر�RTCд����
//    
//    //�ر�RTC����жϣ�������RTCʵ�����
//    __HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RTC_Handler,RTC_IT_WUT);
//    __HAL_RTC_TIMESTAMP_DISABLE_IT(&RTC_Handler,RTC_IT_TS);
//    __HAL_RTC_ALARM_DISABLE_IT(&RTC_Handler,RTC_IT_ALRA|RTC_IT_ALRB);
//    
//    //���RTC����жϱ�־λ
//    __HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
//    __HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_TSF); 
//    __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_WUTF);
    
    __HAL_RCC_BACKUPRESET_RELEASE();                    //��������λ����
//    __HAL_RTC_WRITEPROTECTION_ENABLE(&RTC_Handler);     //ʹ��RTCд����
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);                  //���Wake_UP��־
	
		
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);           //����WKUP���ڻ���
    HAL_PWR_EnterSTANDBYMode();                         //�������ģʽ     
}
 
     
//���WKUP�ŵ��ź�
//����ֵ1:��������3s����
//      0:����Ĵ���	
u8 Check_WKUP(void) 
{
	u8 t=0;
	u8 tx=0;//��¼�ɿ��Ĵ���
	LED(0); //����DS0 
	while(1)
	{
		if(WK_UP_R)//�Ѿ�������
		{
			t++;
			tx=0;
		}else 
		{
			tx++; 
			if(tx>3)//����90ms��û��WKUP�ź�
			{
				LED(1);
				return 0;//����İ���,���´�������
			}
		}
		delay_ms(30);
		if(t>=100)//���³���3����
		{
			LED(0);	  //����DS0 
			return 1; //����3s������
		}
	}
}  

//PA0 WKUP���ѳ�ʼ��
void WKUP_Init(void)
{
    //����Ƿ�����������
    if(Check_WKUP()==0)
    {
        Sys_Enter_Standby();//���ǿ������������ģʽ
    }
}
