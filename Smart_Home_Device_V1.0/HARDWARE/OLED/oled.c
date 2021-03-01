#include "oled.h"
#include "oledfont.h" 
#include "string.h"
#include "delay.h"

//OLED���Դ�
//��Ÿ�ʽ����.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 		   
volatile static uint8_t OLED_GRAM[128][8];


//д����/ָ�� cmd 1-���ݣ�0-ָ�� 

void OLED_Write(uint8_t byte ,uint8_t cmd)
{ 
    uint8_t i;
	OLED_CS(0);
	OLED_DC(cmd);
    for(i=0;i<8;i++)
       { 
	     OLED_SCK(0);
         
		 OLED_SDIN((byte&0x80)>>7);
         
		 byte=byte<<1;
	     
		 delay_us(10);
		 OLED_SCK(1);
        }
	OLED_DC(1);
	OLED_CS(1);
}

//����OLED��ʾ    
void OLED_Display_On(void)
{
	OLED_Write(0X8D,OLED_CMD);  //SET DCDC����
	OLED_Write(0X14,OLED_CMD);  //DCDC ON
	OLED_Write(0XAF,OLED_CMD);  //DISPLAY ON
}
//�ر�OLED��ʾ     
void OLED_Display_Off(void)
{
	OLED_Write(0X8D,OLED_CMD);  //SET DCDC����
	OLED_Write(0X10,OLED_CMD);  //DCDC OFF
	OLED_Write(0XAE,OLED_CMD);  //DISPLAY OFF
}

//������ʾ�������ı���GRAM����д��1306�Դ�
void OLED_Refresh_Gram()
{

		uint8_t i,n;
		for(i=0;i<8;i++)
	{
		OLED_Write(0xB0+i,OLED_CMD);
		OLED_Write(0x00,OLED_CMD);
		OLED_Write(0x10,OLED_CMD);
		
		for(n=0;n<128;n++)
		OLED_Write(OLED_GRAM[n][i],OLED_DAT);
	}
}

//��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!	  
void OLED_Clear(void)  
{  
	uint8_t i,n;  
	for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0X00;  
	OLED_Refresh_Gram();//������ʾ
}

//���� 
//x:0~127
//y:0~63
//t:1 ��� 0,���	
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
		uint8_t pos,bx,temp = 0;
		if(x>127||y>63)return;//������Ļ����
		pos = 7-y/8;			  //��ȡҳ��ַ
		bx = y%8;				  //��ǰҳ������
		temp = 1<<(7-bx);		  
		if(t) OLED_GRAM[x][pos] |= temp;
		else  OLED_GRAM[x][pos]	&= ~temp;
}

//x1,y1,x2,y2 �������ĶԽ�����
//ȷ��x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63	 	 
//dot:0,���;1,���	  
void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot)  
{  
	uint8_t x,y;  
	for(x=x1;x<=x2;x++)
	{
		for(y=y1;y<=y2;y++)OLED_DrawPoint(x,y,dot);
	}													    
	OLED_Refresh_Gram();//������ʾ
}

//��ָ��λ����ʾһ���ַ�,���������ַ�
//x:0~127
//y:0~63
//mode:0,������ʾ;1,������ʾ				 
//size:ѡ������ 12/16/24
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size,uint8_t mode)
{      			    
	uint8_t temp,t,t1;
	uint8_t y0=y;
	uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//�õ�����һ���ַ���Ӧ������ռ���ֽ���
	chr=chr-' ';//�õ�ƫ�ƺ��ֵ		 
    for(t=0;t<csize;t++)
    {   
		if(size==12)temp=asc2_1206[chr][t]; 	 	//����1206����
		else if(size==16)temp=asc2_1608[chr][t];	//����1608����
		else if(size==24)temp=asc2_2412[chr][t];	//����2412����
		else return;								//û�е��ֿ�
        for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)OLED_DrawPoint(x,y,mode);
			else OLED_DrawPoint(x,y,!mode);
			temp<<=1;
			y++;
			if((y-y0)==size)
			{
				y=y0;
				x++;
				break;
			}
		}  	 
    }          
}	

//m^n����
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//��ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ	0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);	 		  
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size/2)*t,y,temp+'0',size,1); 
	}
} 
//��ʾ�ַ���
//x,y:�������  
//size:�����С 
//*p:�ַ�����ʼ��ַ 
void OLED_ShowString(uint8_t x,uint8_t y,const uint8_t *p,uint8_t size)
{	
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {       
        if(x>(128-(size/2))){x=0;y+=size;}
        if(y>(64-size)){y=x=0;OLED_Clear();}
        OLED_ShowChar(x,y,*p,size,1);	 
        x+=size/2;
        p++;
    }  
	
}	


	

		
void OLED_Init(){
    
  GPIO_InitTypeDef GPIO_InitStruct = {0};
        
  __HAL_RCC_GPIOB_CLK_ENABLE();
        
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OLED_SCK_Pin|OLED_SDIN_Pin|OLED_RES_Pin|OLED_DC_Pin 
                          |OLED_CS_Pin, GPIO_PIN_SET);
        
  /*Configure GPIO pins : OLED_SCL_Pin OLED_MOSI_Pin OLED_RES_Pin OLED_RS_Pin 
                           OLED_CS_Pin */
  GPIO_InitStruct.Pin = OLED_SCK_Pin|OLED_SDIN_Pin|OLED_RES_Pin|OLED_DC_Pin 
                          |OLED_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
		OLED_RES(0);
		delay_ms(100);
		OLED_RES(1);
	
		
		OLED_Write(0xAE,OLED_CMD); //�ر���ʾ
		OLED_Write(0xD5,OLED_CMD); //����ʱ�ӷ�Ƶ����,��Ƶ��
		OLED_Write(80,OLED_CMD);   //[3:0],��Ƶ����;[7:4],��Ƶ��
		OLED_Write(0xA8,OLED_CMD); //��������·��
		OLED_Write(0X3F,OLED_CMD); //Ĭ��0X3F(1/64) 
		OLED_Write(0xD3,OLED_CMD); //������ʾƫ��
		OLED_Write(0X00,OLED_CMD); //Ĭ��Ϊ0

		OLED_Write(0x40,OLED_CMD); //������ʾ��ʼ�� [5:0],����.
															
		OLED_Write(0x8D,OLED_CMD); //��ɱ�����
		OLED_Write(0x14,OLED_CMD); //bit2������/�ر�
		OLED_Write(0x20,OLED_CMD); //�����ڴ��ַģʽ
		OLED_Write(0x02,OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
		OLED_Write(0xA1,OLED_CMD); //���ض�������,bit0:0,0->0;1,0->127;
		OLED_Write(0xC0,OLED_CMD); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��
		OLED_Write(0xDA,OLED_CMD); //����COMӲ����������
		OLED_Write(0x12,OLED_CMD); //[5:4]����
			 
		OLED_Write(0x81,OLED_CMD); //�Աȶ�����
		OLED_Write(0xEF,OLED_CMD); //1~255;Ĭ��0X7F (��������,Խ��Խ��)
		OLED_Write(0xD9,OLED_CMD); //����Ԥ�������
		OLED_Write(0xf1,OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
		OLED_Write(0xDB,OLED_CMD); //����VCOMH ��ѹ����
		OLED_Write(0x30,OLED_CMD); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

		OLED_Write(0xA4,OLED_CMD); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
		OLED_Write(0xA6,OLED_CMD); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ	    						   
		OLED_Write(0xAF,OLED_CMD); //������ʾ
		OLED_Clear();
}

void OLED_SET(){

	
}

//���͵�����Ϣ��OLED��
void Send_Debug(char* Message){

	static uint8_t i = 0;//��һ����ʾ������
	OLED_ShowString(0,i*12,(uint8_t*)Message,12);
	i=i+1;
	OLED_Refresh_Gram();
	if(i>5) i=1;

}

//���͵�����Ϣ��OLED��
void Send_Debug_Two(char* Message)
{
	
	OLED_ScreenUDMove(2,7,0,127,'U',0);
	OLED_Refresh_Gram();
	OLED_ShowString(0,48,(uint8_t*)Message,12);
	OLED_Refresh_Gram();

}

//��ʾͼƬ��ͨ���ı�����ֵ��ͼƬ��Ӧ�����ؿ�����ʾ��ͬ���ص�ͼƬ��
//x��y:ͼƬ���������
//px,py:ͼƬ�����أ���ʵ��ȡģ��ͼƬ����һ�£�
//index:ͼƬ����
void OLED_ShowBMP(uint8_t x, uint8_t y, uint8_t px, uint8_t py,const uint8_t *index, uint16_t size)
{
    u8 temp,t1;
    u16 j,i;
    u8 y0=y;
    
    i = size;
 
    for(j = 0; j < i;j++)
    {
        temp = index[j];    //����ͼƬ                          
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)OLED_DrawPoint(x,y,1);
            else OLED_DrawPoint(x,y,0);
            temp<<= 1;
            y++;
            if((y-y0) == py)
            {
                y=y0;
                x++;
                break;
            }
        }
    }
    OLED_Refresh_Gram();
}


/**
  * @brief	:oled�ֲ�������ʾ
  * @note   :--
  * @param	:PageS , ��ʼҳ
             PageE , ��ֹҳ
             ColS  , ��ʼ��
             ColE  , ��ֹ��
             Direct,ˢ��ģʽ(1���� 0����)
  * @return	:void
  *
  * @date   :2016/12/24
  * @design :
  **/
void OLED_UpdatePart(uint8_t PageS, uint8_t PageE, uint8_t ColS, uint8_t ColE, uint8_t Direct)
{
	uint8_t x=0,y=0;

    if(Direct)//����ˢ��
    {        
        OLED_Write(0x20,OLED_CMD);//����ΪˮƽѰַģʽ����ΪҳѰַ�µ��ƶ�������λ����(ˢ��ʱ��̫����)	
        OLED_Write(0x00,OLED_CMD);
        OLED_Write(0x21,OLED_CMD);//����ˮƽѰַ��ʼ�к���ֹ��
        OLED_Write(ColS,OLED_CMD);
        OLED_Write(ColE,OLED_CMD);
        OLED_Write(0x22,OLED_CMD);//����ˮƽѰַ��ʼҳ����ֹҳ
        OLED_Write(PageS,OLED_CMD);
        OLED_Write(PageE,OLED_CMD);

        for(y=PageS;y<=PageE;y++)
        {		
            for(x=ColS;x<=ColE;x++){		
                OLED_Write(OLED_GRAM[x][y],OLED_DAT);
            }
        }
    }
    else    //����ˢ��
    {
        OLED_Write(0x20,OLED_CMD);//����Ϊ��ֱѰַģʽ����ΪҳѰַ�µ��ƶ�������λ����(ˢ��ʱ��̫����)	
        OLED_Write(0x01,OLED_CMD);
        OLED_Write(0x21,OLED_CMD);//���ô�ֱѰַ��ʼ�к���ֹ��
        OLED_Write(ColS,OLED_CMD);
        OLED_Write(ColE,OLED_CMD);
        OLED_Write(0x22,OLED_CMD);//���ô�ֱѰַ��ʼҳ����ֹҳ
        OLED_Write(PageS,OLED_CMD);
        OLED_Write(PageE,OLED_CMD);
        
        for(x=ColS;x<=ColE;x++)
        {		
            for(y=PageS;y<=PageE;y++){		
                OLED_Write(OLED_GRAM[x][y],OLED_DAT);
            }
        }        
    }
    
    OLED_Write(0x20,OLED_CMD);//�ָ�Ĭ��Ѱַģʽ��ҳѰַģʽ��
    OLED_Write(0x02,OLED_CMD);
}

/**
  * @brief	:������������(��������)
  * @note   :--UorD ��Сд�޷���
  * @param	:PageS, ��ʼҳ
             PageE, ��ֹҳ
             ColS , ��ʼ��
             ColE , ��ֹ��
             UorD , ��Ļ�ƶ�����
             Loop , ģʽѡ��(1,ѭ�� 0,��ѭ��)
  * @return	:void
  *
  * @date   :2016/12/24
  * @design :
  **/
void OLED_ScreenUDMove(uint8_t PageS, uint8_t PageE, uint8_t ColS, uint8_t ColE, uint8_t UorD, uint8_t Loop)
{
    uint8_t page=0,col=0;
    uint8_t temp[8]={0};
    
    UorD |= (1<<5);//ת��ΪСд��ʽ��

    for(col=ColS;col<=ColE;col++)
    {
        if(UorD=='u')
        {
            for(page=PageS;page<=PageE;page++)
            {
                temp[page] = (OLED_GRAM[col][page]&0x01)<<7;//ȡ�����λ�����ƶ������λ
                OLED_GRAM[col][page] >>= 1;//�������λ�ƶ����������ƶ�
            }
            
            for(page=PageS;page<=PageE;page++)
            {
                if(Loop)//����ѭ��ģʽ
                {
                    OLED_GRAM[col][page] |= temp[(page+1)%8];
                }else{
                    OLED_GRAM[col][page] |= 0;
                }
            }    
            
            OLED_GRAM[col][PageE] |= temp[PageS]; 
        }else if(UorD=='d')
        {
            for(page=PageS;page<=PageE;page++)
            {
                temp[page] = (OLED_GRAM[col][page]&0x80)>>7;//ȡ�����λ�����ƶ������λ
                OLED_GRAM[col][page] <<= 1;//�������λ�ƶ����������ƶ�
            }
            
            for(page=PageS;page<=PageE;page++)
            {
                if(Loop)//����ѭ��ģʽ
                {
                    OLED_GRAM[col][page] |= temp[(page+7)%8];
                }else{
                    OLED_GRAM[col][page] |= 0;
                }
            }   
            
            OLED_GRAM[col][PageS] |= temp[PageE]; 
        }        
    }

    OLED_UpdatePart(PageS, PageE, ColS, ColE, 1);
}







