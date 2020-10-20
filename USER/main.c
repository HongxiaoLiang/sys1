#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h" 
#include "max31865.h"
#include "string.h"
#include "math.h"
#include "gps.h"
#include "stm32f10x.h"  //������Ҫ��ͷ�ļ�
#include "main.h"       //������Ҫ��ͷ�ļ�
#include "delay.h"      //������Ҫ��ͷ�ļ�
#include "usart.h"     //������Ҫ��ͷ�ļ�
#include "uart4.h"     //������Ҫ��ͷ�ļ�
#include "timer1.h"     //������Ҫ��ͷ�ļ�
#include "timer2.h"     //������Ҫ��ͷ�ļ�
#include "timer3.h"     //������Ҫ��ͷ�ļ�
#include "timer4.h"     //������Ҫ��ͷ�ļ�
#include "wifi.h"	    //������Ҫ��ͷ�ļ�
#include "mqtt.h"       //������Ҫ��ͷ�ļ�
#include "dht11.h"
#include "iic.h"        //������Ҫ��ͷ�ļ�
#include "stdio.h"
#include "adc.h"


/************************************************
 ALIENTEK ս��STM32F103������ʵ��18
�ڲ��¶ȴ����� ʵ�� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
double longtitude,latitude,humidity,temperature,oiltemper,speed,ic2;
float ic;
int pressure,gas,altitude;
u8 iaq,altitude1;

u16 oiltemper1,temperature1,humidity1,speed1,adcx1,ic1;
u32 gas1,latitude1,longtitude1,pressure1;


float temperatureTs;


u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//����1,���ͻ�����
nmea_msg gpsx; 											//GPS��Ϣ
__align(4) u8 dtbuf[50];   								//��ӡ������
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode�ַ��� 



static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_X;
  
  /* 4����ռ���ȼ���4����Ӧ���ȼ� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  /*��ռ���ȼ��ɴ���жϼ���͵��ж�*/
	/*��Ӧ���ȼ����ȼ�ִ��*/
	NVIC_X.NVIC_IRQChannel = USART2_IRQn;//�ж�����
  NVIC_X.NVIC_IRQChannelPreemptionPriority = 3;//��ռ���ȼ�
  NVIC_X.NVIC_IRQChannelSubPriority = 3;//��Ӧ���ȼ�
  NVIC_X.NVIC_IRQChannelCmd = ENABLE;//ʹ���ж���Ӧ
  NVIC_Init(&NVIC_X);
}


void send_Instruction(void)
{
	uint8_t send_data[4]={0};
	send_data[0]=0xa5;
	send_data[1]=0x55;
	send_data[2]=0x3F;
	send_data[3]=0x39;
	USART_Send_bytes(send_data,4);//����
	
	delay_ms(100);
	
	send_data[0]=0xa5;
	send_data[1]=0x56;
	send_data[2]=0x02;
	send_data[3]=0xfd;
	USART_Send_bytes(send_data,4);//�����Զ����ָ��
	delay_ms(100);
}


//int fputc(int ch, FILE *f)
//{
// while (!(USART1->SR & USART_FLAG_TXE));
// USART_SendData(USART1, (unsigned char) ch);// USART1 ���Ի��� USART2 ��
// return (ch);
//}


	  
//��ʾGPS��λ��Ϣ 
void Gps_Msg_Show(void)
{

 	float tp;		   
	POINT_COLOR=BLUE;  	 
	tp=gpsx.longitude;	   
	sprintf((char *)dtbuf,"Longitude:%.5f %1c   ",tp/=100000,gpsx.ewhemi);	//�õ������ַ���
	longtitude =(tp/=100000) ;
	longtitude1=gpsx.longitude;
 	LCD_ShowString(30,120,200,16,16,dtbuf);	 	   
	tp=gpsx.latitude;	   
	sprintf((char *)dtbuf,"Latitude:%.5f %1c   ",tp/=100000,gpsx.nshemi);	//�õ�γ���ַ���
	latitude=(tp/=100000);
	latitude1=gpsx.latitude;
 	LCD_ShowString(30,140,200,16,16,dtbuf);	 	 
	tp=gpsx.altitude;	   
 	sprintf((char *)dtbuf,"Altitude:%.1fm     ",tp/=10);	    			//�õ��߶��ַ���
 	LCD_ShowString(30,160,200,16,16,dtbuf);	 			   
	tp=gpsx.speed;	
  speed1=	gpsx.speed;
 	sprintf((char *)dtbuf,"Speed:%.3fkm/h     ",tp/=1000);		    		//�õ��ٶ��ַ���	
	speed=tp/=1000;
 	LCD_ShowString(30,180,200,16,16,dtbuf);	 				    
	if(gpsx.fixmode<=3)														//��λ״̬
	{  
		sprintf((char *)dtbuf,"Fix Mode:%s",fixmode_tbl[gpsx.fixmode]);	
	  LCD_ShowString(30,200,200,16,16,dtbuf);			   
	}	 	   
	sprintf((char *)dtbuf,"GPS+BD Valid satellite:%02d",gpsx.posslnum);	 		//���ڶ�λ��GPS������
 	LCD_ShowString(30,220,200,16,16,dtbuf);	    
	sprintf((char *)dtbuf,"GPS Visible satellite:%02d",gpsx.svnum%100);	 		//�ɼ�GPS������
 	LCD_ShowString(30,240,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"BD Visible satellite:%02d",gpsx.beidou_svnum%100);	 		//�ɼ�����������
 	LCD_ShowString(30,260,200,16,16,dtbuf);
	
	sprintf((char *)dtbuf,"UTC Date:%04d/%02d/%02d   ",gpsx.utc.year,gpsx.utc.month,gpsx.utc.date);	//��ʾUTC����
	LCD_ShowString(30,280,200,16,16,dtbuf);		    
	sprintf((char *)dtbuf,"UTC Time:%02d:%02d:%02d   ",gpsx.utc.hour,gpsx.utc.min,gpsx.utc.sec);	//��ʾUTCʱ��
  LCD_ShowString(30,300,200,16,16,dtbuf);		
  	
}


int main(void)
{	

	u16 i,rxlen;
	u16 lenx;
	u8 key=0XFF;
	u8 upload=0;	
	u16 adcx,ic;
	float temp;
	u16 temp5;
	double temp3;
	short temp4;
	uint8_t data_buf[50]={0},count=0;

  float Temperature,Humidity;
  uint32_t Gas;
  uint32_t Pressure;
  uint16_t IAQ;
	int16_t Altitude=0;
  uint8_t IAQ_accuracy;
	uint16_t temp1=0;
  int16_t temp2=0;
	
	//delay_init(72);//72M 
	extern int pressure;
	
	
	
	
	delay_init();                   //��ʱ���ܳ�ʼ��              
	Uart4_Init(115200);
	TIM4_Init(300,7200);            //TIM4��ʼ������ʱʱ�� 300*7200*1000/72000000 = 30ms
	LED_Init();	                    //LED��ʼ��
	KEY_Init();                     //������ʼ��
	IIC_Init();                     //��ʼ��IIC�ӿ�
	Adc_Init();
	
	delay_init();
	LED_Init();
	uart_init(9600);
	Usart_Int2(9600);
	NVIC_Configuration();//�����ж����ȼ�����
	send_Instruction();//��ģ�鷢��ָ��
	GPIO_SetBits(GPIOB,GPIO_Pin_5);

  MAX31865_Init();
	MAX31865_Cfg();  

			
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 //	usmart_dev.init(72);		//��ʼ��USMART		
 	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD   
	usart3_init(38400);		//��ʼ������3 
	POINT_COLOR=RED;
	LCD_ShowString(30,20,200,16,16,"JIE");	  

	LCD_ShowString(30,60,200,16,16,"ENVIR TEST");
	LCD_ShowString(30,80,200,16,16,"KEY0:Upload NMEA Data SW");   	 										   	   
 
	if(SkyTra_Cfg_Rate(5)!=0)	//���ö�λ��Ϣ�����ٶ�Ϊ5Hz,˳���ж�GPSģ���Ƿ���λ. 
	{
   	LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Setting...");
		do
		{
			usart3_init(9600);			//��ʼ������3������Ϊ9600
	  	SkyTra_Cfg_Prt(3);			//��������ģ��Ĳ�����Ϊ38400
			usart3_init(38400);			//��ʼ������3������Ϊ38400
      key=SkyTra_Cfg_Tp(100000);	//������Ϊ100ms
		}while(SkyTra_Cfg_Rate(5)!=0&&key!=0);//����SkyTraF8-BD�ĸ�������Ϊ5Hz
	  LCD_ShowString(30,120,200,16,16,"SkyTraF8-BD Set Done!!");
		delay_ms(500);
		LCD_Fill(30,120,30+200,120+16,WHITE);//�����ʾ 
	}
	
	WiFi_ResetIO_Init();            //��ʼ��WiFi�ĸ�λIO
  MQTT_Buff_Init();               //��ʼ������,����,�������ݵ� ������ �Լ���״̬����
	AliIoT_Parameter_Init();	    //��ʼ�����Ӱ�����IoTƽ̨MQTT�������Ĳ���	

	while(1) 
	{		
/*--------------------------------------------------------------------*/
		/*   Connect_flag=1ͬ����������������,���ǿ��Է������ݺͽ���������    */
		/*--------------------------------------------------------------------*/
		if(Connect_flag==1){ 

			
			
			/*-------------------------------------------------------------*/
			/*                     �����ͻ���������                      */
			/*-------------------------------------------------------------*/
				if(MQTT_TxDataOutPtr != MQTT_TxDataInPtr){                //if�����Ļ���˵�����ͻ�������������
				//3������ɽ���if
				//��1�֣�0x10 ���ӱ���
				//��2�֣�0x82 ���ı��ģ���ConnectPack_flag��λ����ʾ���ӱ��ĳɹ�
				//��3�֣�SubcribePack_flag��λ��˵�����ӺͶ��ľ��ɹ����������Ŀɷ�
				if((MQTT_TxDataOutPtr[2]==0x10)||((MQTT_TxDataOutPtr[2]==0x82)&&(ConnectPack_flag==1))||(SubcribePack_flag==1)){    
					printf("��������:0x%x\r\n",MQTT_TxDataOutPtr[2]);  //������ʾ��Ϣ
					MQTT_TxData(MQTT_TxDataOutPtr);                       //��������
					MQTT_TxDataOutPtr += BUFF_UNIT;                       //ָ������
					if(MQTT_TxDataOutPtr==MQTT_TxDataEndPtr)              //���ָ�뵽������β����
						MQTT_TxDataOutPtr = MQTT_TxDataBuf[0];            //ָ���λ����������ͷ
				} 				
			}//�����ͻ��������ݵ�else if��֧��β
			
			/*-------------------------------------------------------------*/
			/*                     ������ջ���������                      */
			/*-------------------------------------------------------------*/
			if(MQTT_RxDataOutPtr != MQTT_RxDataInPtr){  //if�����Ļ���˵�����ջ�������������														
				printf("���յ�����:");
				/*-----------------------------------------------------*/
				/*                    ����CONNACK����                  */
				/*-----------------------------------------------------*/				
				//if�жϣ������һ���ֽ���0x20����ʾ�յ�����CONNACK����
				//��������Ҫ�жϵ�4���ֽڣ�����CONNECT�����Ƿ�ɹ�
				if(MQTT_RxDataOutPtr[2]==0x20){             			
				    switch(MQTT_RxDataOutPtr[5]){					
						case 0x00 : printf("CONNECT���ĳɹ�\r\n");                            //���������Ϣ	
								    ConnectPack_flag = 1;                                        //CONNECT���ĳɹ������ı��Ŀɷ�
									break;                                                       //������֧case 0x00                                              
						case 0x01 : printf("�����Ѿܾ�����֧�ֵ�Э��汾��׼������\r\n");     //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x01   
						case 0x02 : printf("�����Ѿܾ������ϸ�Ŀͻ��˱�ʶ����׼������\r\n"); //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x02 
						case 0x03 : printf("�����Ѿܾ�������˲����ã�׼������\r\n");         //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������
									break;                                                       //������֧case 0x03
						case 0x04 : printf("�����Ѿܾ�����Ч���û��������룬׼������\r\n");   //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������						
									break;                                                       //������֧case 0x04
						case 0x05 : printf("�����Ѿܾ���δ��Ȩ��׼������\r\n");               //���������Ϣ
									Connect_flag = 0;                                            //Connect_flag���㣬��������						
									break;                                                       //������֧case 0x05 		
						default   : printf("�����Ѿܾ���δ֪״̬��׼������\r\n");             //���������Ϣ 
									Connect_flag = 0;                                            //Connect_flag���㣬��������					
									break;                                                       //������֧case default 								
					}				
				}			
				//if�жϣ���һ���ֽ���0x90����ʾ�յ�����SUBACK����
				//��������Ҫ�ж϶��Ļظ��������ǲ��ǳɹ�
				else if(MQTT_RxDataOutPtr[2]==0x90){ 
						switch(MQTT_RxDataOutPtr[6]){					
						case 0x00 :
						case 0x01 : printf("���ĳɹ�\r\n");            //���������Ϣ
							        SubcribePack_flag = 1;                //SubcribePack_flag��1����ʾ���ı��ĳɹ����������Ŀɷ���
									Ping_flag = 0;                        //Ping_flag����
   								    TIM3_ENABLE_30S();                    //����30s��PING��ʱ��
									TIM2_ENABLE_30S();                    //����30s���ϴ����ݵĶ�ʱ��
						          TempHumi_State();                     //�ȷ�һ������
									break;                                //������֧                                             
						default   : printf("����ʧ�ܣ�׼������\r\n");  //���������Ϣ 
									Connect_flag = 0;                     //Connect_flag���㣬��������
									break;                                //������֧ 								
					}					
				}
				//if�жϣ���һ���ֽ���0xD0����ʾ�յ�����PINGRESP����
				else if(MQTT_RxDataOutPtr[2]==0xD0){ 
					printf("PING���Ļظ�\r\n"); 		  //���������Ϣ 
					if(Ping_flag==1){                     //���Ping_flag=1����ʾ��һ�η���
						 Ping_flag = 0;    				  //Ҫ���Ping_flag��־
					}else if(Ping_flag>1){ 				  //���Ping_flag>1����ʾ�Ƕ�η����ˣ�������2s����Ŀ��ٷ���
						Ping_flag = 0;     				  //Ҫ���Ping_flag��־
						TIM3_ENABLE_30S(); 				  //PING��ʱ���ػ�30s��ʱ��
					}				
				}	
				//if�жϣ������һ���ֽ���0x30����ʾ�յ����Ƿ�������������������
				//����Ҫ��ȡ��������
				else if((MQTT_RxDataOutPtr[2]==0x30)){ 
					printf("�������ȼ�0����\r\n"); 		   //���������Ϣ 
					MQTT_DealPushdata_Qs0(MQTT_RxDataOutPtr);  //����ȼ�0��������
				}				
								
				MQTT_RxDataOutPtr += BUFF_UNIT;                     //ָ������
				if(MQTT_RxDataOutPtr==MQTT_RxDataEndPtr)            //���ָ�뵽������β����
					MQTT_RxDataOutPtr = MQTT_RxDataBuf[0];          //ָ���λ����������ͷ                        
			}//������ջ��������ݵ�else if��֧��β
			
			/*-------------------------------------------------------------*/
			/*                     ���������������                      */
			/*-------------------------------------------------------------*/
			if(MQTT_CMDOutPtr != MQTT_CMDInPtr){                             //if�����Ļ���˵�����������������			       
				printf("����:%s\r\n",&MQTT_CMDOutPtr[2]);                 //���������Ϣ
				
				MQTT_CMDOutPtr += BUFF_UNIT;                             	 //ָ������
				if(MQTT_CMDOutPtr==MQTT_CMDEndPtr)           	             //���ָ�뵽������β����
					MQTT_CMDOutPtr = MQTT_CMDBuf[0];          	             //ָ���λ����������ͷ				
			}//��������������ݵ�else if��֧��β	
		}//Connect_flag=1��if��֧�Ľ�β
		
		/*--------------------------------------------------------------------*/
		/*      Connect_flag=0ͬ�������Ͽ�������,����Ҫ�������ӷ�����         */
		/*--------------------------------------------------------------------*/
		else{ 
 
			printf("��Ҫ���ӷ�����\r\n");                 //���������Ϣ
			TIM_Cmd(TIM4,DISABLE);                           //�ر�TIM4 
			TIM_Cmd(TIM3,DISABLE);                           //�ر�TIM3  
			WiFi_RxCounter=0;                                //WiFi������������������                        
			memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);          //���WiFi���ջ����� 
			if(WiFi_Connect_IoTServer()==0){   			        //���WiFi�����Ʒ�������������0����ʾ��ȷ������if
				printf("����TCP���ӳɹ�\r\n");               //���������Ϣ
				Connect_flag = 1;                            //Connect_flag��1����ʾ���ӳɹ�	
				WiFi_RxCounter=0;                            //WiFi������������������                        
				memset(WiFi_RX_BUF,0,WiFi_RXBUFF_SIZE);      //���WiFi���ջ����� 
				MQTT_Buff_ReInit();                          //���³�ʼ�����ͻ�����                    
			}				
		}
		
		
		//		
				if(!stata)
		 continue;
		 stata=0;
		delay_ms(1);
		
		if(USART3_RX_STA&0X8000)		//���յ�һ��������
		{
			rxlen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//������һ�ν���
			USART1_TX_BUF[i]=0;			//�Զ���ӽ�����
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//�����ַ���
			Gps_Msg_Show();				//��ʾ��Ϣ	
			if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//���ͽ��յ������ݵ�����1
				if(CHeck(data_buf))
		{
			 count=0;
		   if(data_buf[2]&0x01) //Temperature
			 {
			 temp2=((uint16_t)data_buf[4]<<8|data_buf[5]);   
        Temperature=(float)temp2/100;
		//	  printf("Temperature: %.2f 'C", Temperature);
         count=2;
				 LCD_ShowString(30,350,200,16,16,"Temperate: 00.00C");
				 temp3=Temperature*100;
				 temp4=temp3;
				 temperature = Temperature;
				 temperature1=temp4;
				 if(temp4<0)
         {
				  temp4=-temp4;
					LCD_ShowString(30+10*8,350,16,16,16,"-");	//��ʾ����
				  }else 
				LCD_ShowString(30+10*8,350,16,16,16," ");	//�޷���		
			  LCD_ShowxNum(30+11*8,350,temp4/100,2,16,0);		//��ʾ��������
				LCD_ShowxNum(30+14*8,350,temp4%100,2,16, 0X80);	//��ʾС������
				//delay_ms(1000);
				 }
			  if(data_buf[2]&0x02) //Humidity
			 {  
				 temp3=0;
				 temp4=0;
			    temp1=((uint16_t)data_buf[4+count]<<8)|data_buf[5+count];
				  Humidity=(float)temp1/100; 
				//  printf(" ,Humidity: %.2f %% ", Humidity);
          count+=2;
				 LCD_ShowString(30,370,200,16,16,"Humidity: 00   %");
				 temp3=Humidity ; 
				 temp4=temp3;
				 humidity=temp3;
				 humidity1=temp1;
				// temp4=temp3;
//				 if(temp4<0)
//         {
//				  temp4=-temp4;
//					LCD_ShowString(30+10*8,170,16,16,16,"-");	//��ʾ����
//				  }else 
				LCD_ShowString(30+10*8,370,16,16,16," ");	//�޷���		
			  LCD_ShowxNum(30+11*8,370,temp4,2,16,0);		//��ʾ��������
			//	LCD_ShowxNum(30+14*8,170,temp4%10,2,16, 0X80);	//��ʾС������
				//delay_ms(1000);
			 }
			  if(data_buf[2]&0x04) //Pressure
			 {
			    Pressure=((uint32_t)data_buf[4+count]<<16)|((uint16_t)data_buf[5+count]<<8)|data_buf[6+count];
				//  printf(" ,Pressure: %d Pa", Pressure);
          count+=3;
				 LCD_ShowString(30,390,200,16,16,"Pressure: 000000 Pa");
				// temp4=Pressure ;
				 pressure=Pressure;
				 pressure1=Pressure;
				  LCD_ShowxNum(30+11*8,390,Pressure,6,16,0);		//��ʾ��������
				 
			 }
			 
			  if(data_buf[2]&0x08) //IAQ_accuracy��IAQ
			 {
			   IAQ_accuracy=(data_buf[4+count]&0xf0)>>4;
				 IAQ=(((uint16_t)data_buf[4+count]&0x000f)<<8)|data_buf[5+count];
		
				// printf(" ,IAQ: %d ,IAQ_accuracy: %d",IAQ,IAQ_accuracy);
          count+=2;
				  LCD_ShowString(30,410,200,16,16,"IAQ: ");
				 iaq=IAQ;
				 LCD_ShowxNum(30+14*8,410,IAQ,4,16,0);		//��ʾ��������
				 LCD_ShowString(30,430,200,18,16,"IAQ_accuracy: ");
				 LCD_ShowxNum(30+14*8,430,IAQ_accuracy,2,16,0);		//��ʾ��������
			 }

			   if(data_buf[2]&0x10) //Gas
			 {
			    Gas =((uint32_t)data_buf[4+count]<<24)|((uint32_t)data_buf[5+count]<<16)|((uint16_t)data_buf[6+count]<<8)|data_buf[7+count]; 
			//	  printf(" ,Gas: %d ohm ", Gas);
          count+=4;
				 gas=Gas;
				 gas1=Gas;
				 LCD_ShowString(30,450,200,16,16,"Gas:              ohm");
				 //LCD_ShowString(30,260,200,18,16,"IAQ_accuracy: ");
				 LCD_ShowxNum(30+11*8,450,Gas,6,16,0);		//��ʾ��������
			 }
			   if(data_buf[2]&0x10)//����
				 {
				    Altitude=((int16_t)data_buf[4+count]<<8)|data_buf[5+count];
					 altitude=Altitude;
					 altitude1=Altitude;
				  //  printf(" ,Altitude: %d m \r\n", Altitude);
					 LCD_ShowString(30,470,200,16,16,"Altitude:           m");
					 LCD_ShowxNum(30+14*8,470,Altitude,5,16,0);		//��ʾ��������
				 }
		temp3=MAX31865_GetTemp();
		
		     //		temp2=MAX31865_GetTemp();
		temp4=temp3*100;
				 oiltemper1=temp4;
		//temp=temp1;
	//	printf(" temp=%d\r\n",temp1);
		LCD_ShowString(30,510,200,16,16,"OilTemper: 00.00C");
//		if(temp4<0)
//		{
//			temp4=-temp4;
//			LCD_ShowString(30+10*8,510,16,16,16,"-");	//��ʾ����
//		}else 
		LCD_ShowString(30+10*8,510,16,16,16," ");	//�޷���		
		LCD_ShowxNum(30+11*8,510,temp4/100,2,16,0);		//��ʾ��������
		LCD_ShowxNum(30+14*8,510,temp4%100,2,16, 0X80);	//��ʾС������;
			 oiltemper=temp3;
				 
				 
		}

 		}
		 	adcx=Get_Adc_Average(ADC_Channel_1,10);
					//LCD_ShowString(30,530,200,16,16,"ADCCH0AL:");	      
					LCD_ShowString(30,550,200,16,16,"ADCCH0VOL: 0.000A");	
					//LCD_ShowxNum(30+11*8,530,adcx,4,16,0);//��ʾADC��ֵ
					temp=(float)adcx*(3.3/4096);
		      adcx=temp;
					ic1=1000*temp;
		      ic2=temp;
					//adcx=temp;
					LCD_ShowxNum(30+11*8,550,adcx,1,16,0);//��ʾ��ѹֵ
					temp-=adcx;
					temp*=1000;
					LCD_ShowxNum(46+11*8,550,temp,3,16,0X80);
				 
		}	 
					 
	}			 
						
	
		
		
		
		


/*-------------------------------------------------*/
/*���������ɼ���ʪ�ȣ���������������               */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void TempHumi_State(void)
{

//	u8 EnvTemperature,RoomHumidity,oilTemp;	
//	char temp[256];  
//	//oilTemp =126;
//	//DHT11_Read_Data(&EnvTemperature,&RoomHumidity);	//��ȡ��ʪ��ֵ	
////	AHT10_Data(&tempdata,&humidata);
//	

////	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"humidity\":%0.2f,\"longtitude\":%0.4f,\"latitude\":%0.4f,\"speed\":%0.2f,\"pressure\":%d,\"altitude\":%d}\"version\":\"1.0.0\"}",humidity,longtitude,latitude,speed,pressure,altitude);  //�����ظ�ʪ���¶�����
////	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������	
//	
//	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"203302322\",\"params\":{\"iaq\":%2d}\"version\":\"1.0.0\"}",iaq);
//	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));
		//u8 EnvTemperature,RoomHumidity;	
	char temp[512];  
	
	//DHT11_Read_Data(&EnvTemperature,&RoomHumidity);	//��ȡ��ʪ��ֵ	
//	AHT10_Data(&tempdata,&humidata);
	//printf("�¶ȣ�%d  ʪ�ȣ�%d\r\n",EnvTemperature,RoomHumidity);
  printf("ʪ��:%0.2f ����:%0.4f ��γ:%0.4f �ٶ�:%0.2f ��ѹ:%d ����:%d ��������:%d ����:%d �¶�:%0.2f ����:%0.2f\r\n",humidity,longtitude,latitude,speed,pressure,altitude,iaq,gas,temperature,oiltemper);
//	sprintf(temp,"{\"oiltemper100\":%2d,\"temperature100\":%2d,\"iaq\":%2d,\"gas\":%2d,\"pressure\":%2d,\"altitude\":%2d,\"humidity100\":%2d,\"longtitude100000\":%2d,\"latitude100000\":%2d,\"speed1000\":%2d,\"IC1000\":%2d}",oiltemper1,temperature1,iaq,gas1,pressure1,altitude,humidity1,longtitude1,latitude1,speed1,ic1);  //�����ظ�ʪ���¶�����
	sprintf(temp,"{\"method\":\"thing.event.property.post\",\"id\":\"1349184133\",\"params\":{\"CurrentTemperature\":%.2f,\"CurrentHumidity\":%.2f},\"version\":\"1.0.0\"}",temperature,humidity);
	MQTT_PublishQs0(P_TOPIC_NAME,temp,strlen(temp));   //������ݣ�������������	
}

