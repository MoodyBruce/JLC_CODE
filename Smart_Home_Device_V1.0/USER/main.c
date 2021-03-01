#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "uart.h"
#include "wifi.h"
#include "sht30.h"
#include "oled.h"
#include "timer.h"
#include "text.h"
#include "main.h"
#include "i2c.h"
#include "ri.h"
#include "adc.h"
#include "wkup.h"

temp_enum temp_alarm = TEMP_ALARM_CANCEL;
light_enum light_alarm = LIGHT_ALARM_CANCEL;


u8 light = 0;
u16 temp = 0,humi = 0;
u8 temp_max = 60,temp_min = 0,light_min = 0;
u8 sensor_flag = 0;
u8 device_state = 0x00;    //0x00 ���� 0x01 �ر� 0x02 ����

void Show_TempAndHumi(u8 x1,u8 y1,u8 x2,u8 y2,u8 size)
{
    u8 temp_H,temp_L,humi_H,humi_L;
    temp_H = temp/10; temp_L = temp%10;
    humi_H = humi/10; humi_L = humi%10;
    OLED_ShowNum(x1-16,y1,temp_H,2,size);
    OLED_ShowChar(x1,y1,'.',size,1);
    OLED_ShowNum(x1+8,y1,temp_L,1,size);
    OLED_ShowString(x1+8+(size),y1,"%C",size);
    
    OLED_ShowNum(x2-16,y2,humi_H,2,size);
    OLED_ShowChar(x2,y2,'.',size,1);
    OLED_ShowNum(x2+8,y2,humi_L,1,size);
    OLED_ShowString(x2+8+(size),y2,"RH%",size);
    
   
    
    OLED_Refresh_Gram();
}

/***************************************************************
*
*���ܣ�main����	 
*��������
*
*****************************************************************/ 
int main(void)
{
    uint8_t times = 0;
    float t = 0.0,h = 0.0;
    HAL_Init();                    //��ʼ��HAL��    
    Stm32_Clock_Init();   		   //��ʼ��ʱ��
    delay_init(48);                //��ʼ����ʱ����
    uart_init(115200);             //��ʼ������ͨ��
    
    
    LED_Init();                    //��ʼ��LED	
    KEY_Init();                    //��ʼ������
    USB_Init();                    //��ʼ��USB�̵���
    RI_Init();                     //��ʼ���������
    TIM3_Init();                   //��ʼ����ʱ��3
    OLED_Init();                   //��ʼ��OLED
    Tuya_Uart_Init(9600);          //Ϳѻͨ�Ŵ��ڳ�ʼ��
    wifi_protocol_init();          //��ʼ��Ϳѻģ��
    MY_ADC_Init();                 //��ʼ��ADC����
    Sht30_Singleshot_Init();       //��ʼ��SHT30
    delay_ms(50);
    
                      
    
    OLED_Show_Title(4,0,16);       //����OLED��ʾ����
    
    while(1)
    {
      
      if(sensor_flag == 1)  //ÿ��3S������ʪ��
      {
        //��ձ�־
        sensor_flag = 0;    
        //���������ݲɼ�  
        Sht30_GetTH_Singleshot_Poll(&t, &h, CMD_MEAS_POLLING_H);
        light = Get_Adc_Average();
        //���ݴ����OLED��ʾ
        temp = t*10; humi = h*10;
        Show_TempAndHumi(70,28,70,48,16);
        //�¶����ݱ������
        if(t > temp_max) temp_alarm = TEMP_UPPER_ALARM;
        else if(t < temp_min) temp_alarm = TEMP_LOWER_ALARM;
        else temp_alarm = TEMP_ALARM_CANCEL;
        //������ֵ�������
        if(light < light_min) light_alarm = LIGHT_LOWER_ALARM;
        else light_alarm  = LIGHT_ALARM_CANCEL;
        //�����ϱ�
        mcu_dp_value_update(DPID_TEMP_CURRENT,temp); //VALUE�������ϱ�;
        mcu_dp_value_update(DPID_HUMIDITY_VALUE,humi); //VALUE�������ϱ�;
        mcu_dp_value_update(DPID_LIGHT_VALUE,light); //VALUE�������ϱ�;  
        mcu_dp_enum_update(DPID_TEMP_ALARM,temp_alarm); //ö���������ϱ�;
        mcu_dp_enum_update(DPID_LIGHT_ALARM,light_alarm); //ö���������ϱ�; 
      }
      
      if(device_state != 0x00)
      {
        if(device_state == 0x01) Device_OFF();
        else Device_ON();
        device_state = 0x00;
      }
      

      wifi_uart_service();
      
      switch(mcu_get_wifi_work_state())
      {
          case SMART_CONFIG_STATE:    Light_Alarm(100);break;//Smart
          case AP_STATE          :    Light_Alarm(500);break;//AP
          case WIFI_NOT_CONNECTED:    LED(1);break;//δ����
          case WIFI_CONNECTED    :    LED(0);break;//������
          case WIFI_CONN_CLOUD   :    LED(0);break;//������
          case WIFI_LOW_POWER    :    LED(1);break;//�͹���
          case SMART_AND_AP_STATE:    Light_Alarm(100);break;//Smart/AP����
          default:;break;
      }
      
      times++;


	}
}
