#include "sht30.h"
#include "i2c.h"
#include "delay.h"
#include "usart.h"

static u8 Sht30_WriteCommand(u16 command);
static u8 Sht30_CheckCRC(u8 data[], u8 num);
static u8 Sht30_Read2BytesAndCrc(u16* data, u8 myack);
static float Sht30_CalcTemperature(u16 raw_temp);
static float Sht30_CalcTemperature(u16 raw_temp);

/********************************************************
*���ܣ�д16λ����
*����ֵ���ɹ�0��ʧ��1
********************************************************/
static u8 Sht30_WriteCommand(u16 command)
{
	u8 res;
	res = IIC_Sht30_Write_Byte(command >> 8);
	res |= IIC_Sht30_Write_Byte(command &0x00ff);
	
	return res;
}

/********************************************************
*���ܣ�����ʪ�ȵ������ֽ�+CRC�ֽڽ���У��
*����ֵ���ɹ�0��ʧ��1
********************************************************/
static u8 Sht30_CheckCRC(u8 data[], u8 num)
{
	u8 bit, i;
	u8 crc = 0xff;
	
	for(i = 0; i < (num-1); i++)
	{
		crc ^= data[i];
		for(bit = 8; bit > 0; --bit)
		{
			if(crc & 0x80){
				crc = (crc << 1) ^ POLYNOMIAL;
			}
			else{
				crc <<= 1;
			}
		}
	}
	
	if(crc == data[num-1]){
		return 0;
	}
	else{
		return 1;
	}
}

/********************************************************
*���ܣ�����ʪ�ȵ������ֽں�CRC�ֽ�
*����ֵ���ɹ�0��ʧ��1
********************************************************/
static u8 Sht30_Read2BytesAndCrc(u16* data, u8 myack)
{
	u8 res;
	u8 bytes[3];
	
	bytes[0] = IIC_Sht30_Read_Byte(ACK);
	bytes[1] = IIC_Sht30_Read_Byte(ACK);
	bytes[2] = IIC_Sht30_Read_Byte(myack);
	
	res = 0;//Sht30_CheckCRC(bytes, 3);
	
	*data = (bytes[0] << 8) | bytes[1];
	
	return res;
}

/********************************************************
*���ܣ������¶�
********************************************************/
static float Sht30_CalcTemperature(u16 raw_temp)
{
	return 175.0f * (float)raw_temp /65535.0f - 45.0f;
}

/********************************************************
*���ܣ�����ʪ��
********************************************************/
static float Sht30_CalcHumidity(u16 raw_humi)
{
	return 100.0f * (float)raw_humi / 65535.0f;
}

/********************************************************
*���ܣ�Singleshotģʽsht30��ʼ��
********************************************************/
void Sht30_Singleshot_Init(void)
{
	IIC_Sht30_Init();
	delay_ms(100);
}

/********************************************************
*���ܣ�Periodic����ģʽsht30��ʼ��
********************************************************/
void Sht30_Periodic_Init(etCommands command)
{
	u8 res;
	
	IIC_Sht30_Init();
	
	IIC_Sht30_Start();
	res = IIC_Sht30_Write_Byte(SHT30_ADR_W);
	if(res != 0){
		IIC_Sht30_Stop();
		return ;
	}
	res = Sht30_WriteCommand(command);
	if(res != 0){
		IIC_Sht30_Stop();
		return ;
	}
	IIC_Sht30_Stop();
	
	delay_ms(150);
}

/********************************************************
*���ܣ�Singleshot Pollģʽ������ʪ��ֵ
*����ֵ���ɹ�0��ʧ��1
********************************************************/
u8 Sht30_GetTH_Singleshot_Poll(float* tempmerature, float* humidity, etCommands command)
{
	u8 res;
	u16 raw_temp=0;
	u16 raw_humi=0;
	u8 time = 50;
	
	IIC_Sht30_Start();
	
	res = IIC_Sht30_Write_Byte(SHT30_ADR_W);
	if(res != 0){
        printf("��Ӧ���ź�\r\n");
		ERR();
	}
	
	if((command != CMD_MEAS_POLLING_L) && (command != CMD_MEAS_POLLING_M) && (command != CMD_MEAS_POLLING_H))
	{
        printf("ָ��ʹ���\r\n");
		ERR();
	}
	
	res = Sht30_WriteCommand(command);
	if(res != 0){
        printf("��Ӧ���ź�1\r\n");
		ERR();
	}
	IIC_Sht30_Stop();
	delay_ms(2);
	
    printf("����ָ�����\r\n");
    
	while(time--)
	{
		IIC_Sht30_Start();
		res = IIC_Sht30_Write_Byte(SHT30_ADR_R);
		if(res == ACK){
            printf("�ӻ�Ӧ����ʪ�����ݲ������\r\n");
			break;
		}
		IIC_Sht30_Stop();
		delay_us(1000);
	}
    
	if(res != 0){
        printf("��ȡ��ʱ,��δ���յ�����\r\n");
		ERR();
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_temp, ACK);
	if(res != 0){
        printf("��ȡ�¶�����ʧ��\r\n");
		ERR();
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_humi, NACK);
	if(res != 0){
		ERR();
	}
	printf("%X,%X\r\n",raw_temp,raw_humi);
	*tempmerature = Sht30_CalcTemperature(raw_temp);
	*humidity = Sht30_CalcHumidity(raw_humi);
	
	return 0;
}

/********************************************************
*���ܣ�Singleshot Stretchģʽ������ʪ��ֵ
*����ֵ���ɹ�0��ʧ��1
*�˺�����δ��ͨ
********************************************************/
u8 Sht30_GetTH_Singleshot_Stretch(float* tempmerature, float* humidity, etCommands command)
{
	u8 res;
	u16 raw_temp;
	u16 raw_humi;
	
	IIC_Sht30_Start();
	
	res = IIC_Sht30_Write_Byte(SHT30_ADR_W);
	if(res != 0){
		ERR();
	}
	
	if((command != CMD_MEAS_CLOCKSTR_L) && (command != CMD_MEAS_CLOCKSTR_M) && (command != CMD_MEAS_CLOCKSTR_H))
	{
		ERR();
	}
	
	res = Sht30_WriteCommand(command);
	if(res != 0){
		ERR();
	}
	
	IIC_Sht30_Stop();
	delay_ms(2);
	
	while(IIC_Sht30_Write_Byte(SHT30_ADR_R) == NACK);
//	res = IIC_Sht30_Write_Byte(SHT30_ADR_R);
//	if(res != 0){
//		ERR();
//	}

	while(READ_SCL_SHT30 == 1)
	{
		delay_us(1000);
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_temp, ACK);
	if(res != 0){
		ERR();
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_humi, NACK);
	if(res != 0){
		ERR();
	}
	printf("raw_temp = %X,raw_humi = %X\r\n",raw_temp,raw_humi);
	*tempmerature = Sht30_CalcTemperature(raw_temp);
	*humidity = Sht30_CalcHumidity(raw_humi);
	
	return 0;
}

/********************************************************
*���ܣ�����ģʽ������ʪ��ֵ
*����ֵ���ɹ�0��ʧ��1
********************************************************/
u8 Sht30_GetTH_Periodic(float* tempmerature, float* humidity)
{
	u8 res;
	u16 raw_temp;
	u16 raw_humi;
	
	IIC_Sht30_Start();
	
	res = IIC_Sht30_Write_Byte(SHT30_ADR_W);
	if(res != 0){
		ERR();
	}
	
	res = Sht30_WriteCommand(CMD_FETCH_DATA);
	if(res != 0){
		ERR();
	}
	
	IIC_Sht30_Start();
	
	res = IIC_Sht30_Write_Byte(SHT30_ADR_R);
	if(res != 0){
		ERR();
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_temp, ACK);
	if(res != 0){
		ERR();
	}
	
	res = Sht30_Read2BytesAndCrc(&raw_humi, NACK);
	if(res != 0){
		ERR();
	}
	
	*tempmerature = Sht30_CalcTemperature(raw_temp);
	*humidity = Sht30_CalcHumidity(raw_humi);
	
	return 0;
}

























