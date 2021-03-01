#include "ri.h"
#include "delay.h"


u8 CMD_OPEN[] = {0xb2,0x4d,0xbf,0x40,0xdc,0x23};
u8 CMD_CLOSE[] = {0xb2,0x4d,0x7b,0x84,0xe0,0x1f};


void RI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, IR_TXD_Pin|RI_TXD_Pin, GPIO_PIN_SET);
  
  /*Configure GPIO pin : RI_TXD_Pin */
  GPIO_InitStruct.Pin = RI_TXD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(RI_TXD_GPIO_Port, &GPIO_InitStruct);
  
  /*Configure GPIO pin : RI_TXD_Pin */
  GPIO_InitStruct.Pin = IR_TXD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(IR_TXD_GPIO_Port, &GPIO_InitStruct);
}


void Device_ON(void)
{
    Transmition(CMD_OPEN,CMD_SIZE);        //���ع�
    delay_ms(5);
    Transmition(CMD_OPEN,CMD_SIZE);        //���ع�
}

void Device_OFF(void)
{
    Transmition(CMD_CLOSE,CMD_SIZE);        //���ع�
    delay_ms(5);
    Transmition(CMD_CLOSE,CMD_SIZE);        //���ع�
}


/***************************************************************
** ���ܣ�     ���ⷢ���ӳ���
** ������	  *s��ָ��Ҫ���͵�����
**             n�����ݳ���
** ����ֵ��    ��
****************************************************************/
void Transmition(uint8_t *s,uint8_t n)
{
    uint8_t i,temp;
    int j;

    RI_TXD(0);
    delay_ms(4);
    delay_us(500);
    RI_TXD(1);
    delay_ms(4);
	delay_us(500);//�ȷ���4.5ms�ĸߵ�ƽ��4.5ms�͵�ƽ�������룩

	for(i=0;i<n;i++)
	{
		for(j=7;j>=0;j--)
		  {
		     temp=(s[i]>>j)&0x01;
		     if(temp==0)//����0
		        {
		           RI_TXD(0);
		           delay_us(530);
		           RI_TXD(1);
		           delay_us(560);//0.56ms�ߵ�ƽ+0.56ms�͵�ƽ
		         }
		     if(temp==1)//����1
		         {
		           RI_TXD(0);
		           delay_us(530);
		           RI_TXD(1);
				   delay_ms(1);
		           delay_us(690);//0.56ms�ߵ�ƽ+1.69ms�͵�ƽ
		
		         }
		  }
	}
    RI_TXD(0);
    delay_us(560);//�����ź�(ֹͣλ)
    RI_TXD(1);//�رպ��ⷢ��
}