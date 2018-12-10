/********************************************************************
//��Ȩ˵��	:Shenzhen E-EYE Co. Ltd., Copyright 2009- All Rights Reserved.
//�ļ�����	:Gps_App.c		
//����		:GPSģ��
//�汾��	:
//������	:dxl
//����ʱ��	:2011.8
//�޸���	:
//�޸�ʱ��	:
//�޸ļ�Ҫ˵��	:
//��ע		:
***********************************************************************/
/*************************�޸ļ�¼*************************/
//***********************����(���ڰ��������)*******************
//2011.8.25--1.����GPSģ��,��GPSģ��ֳ�3�����֣�
//-----------GPS_RxIsr(void)���������ݽ���
//-----------GpsParse_EvTask(void):����Э�����
//-----------Gps_TimeTask(void):����ģ�����
//20118.25--2.GPSģ���ȶ��Դ�ʩ��
//-----------(1)�����ϵ縴λ,GPSģ��Ҳ�ϵ縴λ,��Ҫ�ȹ��ٿ�
//-----------(2)У������Ư�ƴ����ﵽһ������ʱ,��λ�����־,��GPSģ�鸴λ
//--------------��λ:ģ���ȹر�5��,���ٴ�,ֻ��ACC ONʱ�Ż�������־
//-----------(3)ACC OFFʱ,��GPSģ��,ÿСʱ��1����
//-----------(4)����40��û���յ�����,��λ�����־
//-----------(5)Ư������10��,��λ�����־
//-----------(6)У���������10��,��λ�����־
//2011.8.25--3.ȥƯ�Ƶķ�����
//-----------(1)γ�ȷ���,�������ʱ��>=250����/Сʱ,��Ϊ��Ư��
//-----------(2)���ȷ���,�������ʱ��>=250����/Сʱ,��Ϊ��Ư��
//-----------(3)�ٶ���������ʱ,�ٶȱ仯��/ʱ��>=20����/Сʱ��Ϊ��Ư��
//-----------(4)��Ȼ����,������С��3��ˮƽ�������Ӵ��ڵ���10,��Ϊ��Ư��
//**********************�޸�*********************
//2012.6.5--4.ȥƯ�Ʒ�����Ϊ��
//-----------(1)γ�ȷ���,�������ʱ��>=250����/Сʱ,��Ϊ��Ư��
//-----------(2)���ȷ���,�������ʱ��>=250����/Сʱ,��Ϊ��Ư��
//-----------(3)�ٶ���������ʱ,�ٶȱ仯��/ʱ��>=9����/Сʱ��Ϊ��Ư��
//-----------(4)����ȥƯ��ʹ�ܿ��أ�����10��2D�����󣬿���ȥƯ�ƣ�����10��Ư�ƺ󣬹ر�ȥƯ�ƣ�
//****************�ļ�����**************************
#include "include.h"
#include "report.h"
//***************��������***********************
static u16	GprmcVerifyErrorCount = 0;//GPRMCУ��������
static u16	GpggaVerifyErrorCount = 0;//GPGGAУ��������
static u16	GprmcParseErrorCount = 0;//GPRMC�����������
static u16	GpggaParseErrorCount = 0;//GPGGA�����������
static u16	GpsParseEnterErrorCount = 0;//û�н��������������
static u8   GnssStatus = 0;
static u8	StopExcursionEnable = 0;//����ֹͣʱȥƯ��ʹ�ܿ���,0Ϊ��ʹ��,1Ϊʹ��
static u8   GpsRunFlag = 0;
static TDF_STU_GPS STU_GPS;
u32	gPositionTime = 0;//gPosition��Чʱ��ʱ�� 
u32	PositionTime = 0;//Position��Чʱ��ʱ�� 

u8	LastLocationFlag = 0;//��һ�ζ�λ״̬
u8	LocationFlag = 0;//��ǰ��λ״̬
//static u16	AccOffCount = 0;
//static u16	AccOnCount = 0; BY WYF
u8	LastLocationSpeed = 0;//��һ�ζ�λ���ٶ� 
GPS_STRUCT	gPosition;//��ǰ��Чλ��
u8	GpsOnOffFlag = 1;//GPS���ر�־,0ΪGPS��,1ΪGPS��
u8      GpsInitFlag=0;//GPS��ʼ���Ƿ���ɣ�liamtsen 2017-05-02
GPS_STRUCT	 Position;//��ʱ������
CELLULAR_STRUCT  Qcellloc;//��վλ����Ϣ��
//*****************�ⲿ����********************
u8	AccOffGpsControlFlag=1;//ACC OFFʱGPSģ����Ʊ�־,0Ϊ��,1Ϊ��
extern u8	GprmcBuffer[];//���GPRMC����
extern u8	GprmcBufferBusyFlag;//GprmcBuffer����æ��־
extern u8	GpggaBuffer[];//���GPGGA����
extern u8	GpggaBufferBusyFlag;//GpggaBuffer����æ��־
extern u8	GpgsvBuffer[];//���GPGGA����
extern u8	GpgsvBufferBusyFlag;//GpggaBuffer����æ��־
//****************��������*********************
/*********************************************************************
//��������	:Gps_ReadStaNum(void)
//����		:��ȡGPS�Ķ�λ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Gps_ReadStaNum(void)
{
	return gPosition.SatNum;
}
/*********************************************************************
//��������	:GPS_AdjustRtc(GPS_STRUCT *Position)
//����		:GPSУʱ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
ErrorStatus Gps_AdjustRtc(GPS_STRUCT *Position)
{
	
	ST_Time time;

	//TIME_T time;
	//u32 timecount;
	
	time.year = Position->Year;
	time.year += 2000;
	time.month = Position->Month;
	time.day = Position->Date;
	time.hour = Position->Hour;
	time.minute = Position->Minute;
	time.second = Position->Second;
	time.timezone = 32;
	/*if(SUCCESS == CheckTimeStruct(&time))
	{	
		timecount = ConverseGmtime(&time);
		timecount += 8*3600;
		Gmtime(&time, timecount);
		if(SUCCESS == CheckTimeStruct(&time))
		{
			SetRtc(&time);
			return SUCCESS;		
		}
		else
		{
			return ERROR;
		}
	}
	else
	{
		return ERROR;
	}*/

	Ql_SetLocalTime(&time);

	return SUCCESS; 
}	
/*********************************************************************
//��������	:GPS_GpsDataIsOk(u8 *pBuffer, u8 BufferLen)
//����		:�ж�GPS�����Ƿ���ȷ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����1�����󷵻�0
//��ע		:
*********************************************************************/
u8 Gps_DataIsOk(u8 *pBuffer, u8 BufferLen)
{
	u8	i;
	u8	j;
	u8	temp;
	u8	sum = 0;
	u8	count = 0;
	u8	High;
	u8	Low;
	u8	verify;

	for(i=1; i<BufferLen; i++)//��1������ʼ��'$'
	{
		temp = *(pBuffer+i);
		if((0x0a == temp)||(0x0d == temp))
		{
			break;//����ѭ��
		}
		else if('*' == temp)
		{
			j = i;	
			break;
		}
		else
		{
			sum ^= temp;//���
			if(',' == temp)
			{
				count++;
			}
		}
	}
	High = *(pBuffer+j+1);
	Low = *(pBuffer+j+2);
	if((High >= '0')&&(High <= '9'))
	{
		High = High - 0x30;
	}
	else if((High >= 'A')&&(High <= 'F'))
	{
		High = High - 0x37;
	}
	else
	{
		return 0;
	}
	if((Low >= '0')&&(Low <= '9'))
	{
		Low = Low - 0x30;
	}
	else if((Low >= 'A')&&(Low <= 'F'))
	{
		Low = Low - 0x37;
	}
	else
	{
		return 0;
	}
	verify = (High << 4)|Low;
	if(verify == sum)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}
/*********************************************************************
//��������	:GPS_GprmcIsLocation(u8 *pBuffer, u8 BufferLen)
//����		:�ж�GPRMC�����Ƿ�λ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:��ȷ����1�����󷵻�0
//��ע		:
*********************************************************************/
u8 Gps_GprmcIsLocation(u8 *pBuffer, u8 BufferLen)
{
	u8	i;
	u8	temp;
	u8	count = 0;

	for(i=0; i<BufferLen; i++)
	{
		temp = *(pBuffer+i);
		if(',' == temp)
		{
			count++;
		}
		else if('A' == temp)
		{
			if(2 == count)
			{
				return 1;
			}	
		}
	}
	
	return 0;	
}

/*********************************************************************
//��������	:GPS_GprmcParse(u8 *pBuffer, u8 BufferLen)
//����		:����GPRMC����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_GprmcParse(u8 *pBuffer, u8 BufferLen)
{
	u8	i = 0;
	u8	j = 0;
	u8	k = 0;
	u8	l = 0;
	u8	m = 0;
	s8	z = 0;
	u8	temp;
	u8	flag = 0;
	u8	count = 0;
	u16	temp2;
	u16	temp3;

	for(i=0; i<BufferLen; i++)
	{
		temp = *(pBuffer+i);
		if((0x0a == temp)||(0x0d == temp))
		{
			break;
		}
		else if('.' == temp)
		{
			l = i;//.�ŵ�λ��
		}
		else if(',' == temp)
		{
			k = i;//��ǰ���ŵ�λ��
			count++;
			switch(count)
			{
				case 2://��2������,����ʱ����
					{
						if(7 == (l-j))
						{
							//����ʱ��,С������������
							Position.Hour = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
							Position.Minute = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
							Position.Second = (*(pBuffer+j+5)-0x30)*10+(*(pBuffer+j+6)-0x30);	
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;//����
						}
					
						break;
					}
				case 3://��3������,������Ч��־
					{
						if(2 == (k-j))
						{
                                                    	//������Ч��־
							if('A' == *(pBuffer+j+1))
							{
								Position.Status = 1;
							}
							else if('V' == *(pBuffer+j+1))
							{
								Position.Status = 0;
							}
							else
							{
								flag = 1;//����
							}
							
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 4://��4������,����γ��ֵ����ȷ��0.0001�֣���ȡС�������λ
					{
						if((k > l)&&(5 == l-j))
						{
                                                    	//����γ��ֵ
							Position.Latitue_D = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
							Position.Latitue_F = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
							temp2 = 0;
                                                        //������δ����ڷֵ�С��λ����4λʱ����������,dxl,2013.6.27
							for(m=l+1; m<l+5; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l+4-m; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.Latitue_FX = temp2;
                                                        
							
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 5://��5������,����γ�ȷ���
					{
						if(2 == (k-j))
						{
                                                    	//����γ�ȷ���
							if('N' == *(pBuffer+j+1))
							{
								Position.North = 1;
								
							}
							else if('S' == *(pBuffer+j+1))
							{
								Position.North = 0;
								
							}
							else
							{
								flag = 1;
							}
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 6://��6������,��������ֵ����ȷ��0.0001��
					{
						if((k > l)&&(6 == l-j))
						{
                                                    	//��������ֵ
							Position.Longitue_D = (*(pBuffer+j+1)-0x30)*100+(*(pBuffer+j+2)-0x30)*10+*(pBuffer+j+3)-0x30;
							Position.Longitue_F = (*(pBuffer+j+4)-0x30)*10+(*(pBuffer+j+5)-0x30);
							temp2 = 0;
							for(m=l+1; m<l+5; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l+4-m; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.Longitue_FX = temp2;
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 7://��7������,�������ȷ���
					{
						if(2 == (k-j))
						{
                                                    	//�������ȷ���
							if('E' == *(pBuffer+j+1))
							{
								Position.East = 1;
								
							}
							else if('W' == *(pBuffer+j+1))
							{
								Position.East = 0;
								
							}
							else
							{
								flag = 1;
							}
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 8://��8������,�����ٶȣ���ȷ��������С����������
					{
						if((k>l)&&(l>j))
						{
							temp2 = 0;
							for(m=j+1; m<l; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l-1-m; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.Speed = temp2;
                                                        Position.SpeedX = *(pBuffer+l+1)-0x30;
                                                        if(Position.SpeedX >= 10)
                                                        {
                                                              //��������
                                                              Position.SpeedX = 0;
                                                        }
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 9://��9������,�������򣬾�ȷ��������С����������
					{
						if((k>l)&&(l>j))
						{
							temp2 = 0;
							for(m=j+1; m<l; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l-1-m; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.Course = temp2;		
						}
						else if(1 == (k-j))//û������
						{
							Position.Course = 0;
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 10://��10������,����������
					{
						if(7 == (k-j))
						{
							//����
							Position.Date = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
							Position.Month = (*(pBuffer+j+3)-0x30)*10+(*(pBuffer+j+4)-0x30);
							Position.Year = (*(pBuffer+j+5)-0x30)*10+(*(pBuffer+j+6)-0x30);
                                                        //Position.Hour += 8;//��8Сʱ,�б��ⲻ��������Σ���Ϊ�б���ʱ��Уʱ��
                                                        //if(Position.Hour >= 24)//ʵ��ʹ��ʱ��8Сʱ����Уʱ������
                                                        //{
                                                               //Position.Hour -= 24;
                                                               //Position.Date++;
                                                       // }
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				default : break;
				
			}
			j = i;//��һ�����ŵ�λ��
		}
		if(1 == flag)//�����⵽��������ǰ����
		{
			break;
		}
		
	}
	if(1 == flag)
	{
		GprmcParseErrorCount++;
	}
	else
	{
		GprmcParseErrorCount = 0;	
	}
}
/*********************************************************************
//��������	:GPS_GpggaParse(u8 *pBuffer, u8 BufferLen)
//����		:����GPGGA����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_GpggaParse(u8 *pBuffer, u8 BufferLen)
{
	u8	i = 0;
	u8	j = 0;
	u8	k = 0;
	u8	l = 0;
	u8	m = 0;
	s8	z = 0;
	u8	temp;
	s16	temp2;
	s16	temp3;
	u8	flag = 0;
	u8	count = 0;
	for(i=0; i<BufferLen; i++)
	{
		temp = *(pBuffer+i);
		if((0x0a == temp)||(0x0d == temp))
		{
			break;
		}
		else if('.' == temp)
		{
			l = i;
		}
		else if(',' == temp)
		{
			k = i;
			count++;
			switch(count)
			{
			
				case 8://��8�����ţ���������
					{
						if(2 == (k-j))//ֻ��һλ��
						{
							Position.SatNum = (*(pBuffer+j+1)-0x30);
						}
						else if(3 == (k-j))//��λ��
						{
							Position.SatNum = (*(pBuffer+j+1)-0x30)*10+(*(pBuffer+j+2)-0x30);
						}
						else if(1 == (k-j))//û������
						{
							
						}
						else
						{
							flag = 1;
						}
						break;
					}
				case 9://��9�����ţ�����ˮƽ�������ӣ���ȷ��������С������
					{
						temp2 = 0;
						for(m=j+1; m<l; m++)
						{
							temp3 = (*(pBuffer+m)-0x30);
							for(z=0; z<l-m-1; z++)
							{
								temp3 = temp3 * 10;	
							}
							temp2 += temp3;
						}
						Position.HDOP = temp2;
						break;
					}
				case 10://��10�����ţ����������뺣ƽ��ĸ߶ȣ���ȷ��������С������
					{
						temp2 = 0;
						if(*(pBuffer+j+1) == '-')//������
						{
							for(m=j+2; m<l; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l-m-1; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.High = -temp2;
						}
						else//������
						{
							for(m=j+1; m<l; m++)
							{
								temp3 = (*(pBuffer+m)-0x30);
								for(z=0; z<l-m-1; z++)
								{
									temp3 = temp3 * 10;	
								}
								temp2 += temp3;
							}
							Position.High = temp2;
						}
						break;
					}
                                case 12://�����߳����ֵ
                                  {
                                        temp2 = 0;
					if(*(pBuffer+j+1) == '-')//��ƫ��
					{
						for(m=j+2; m<l; m++)
						{
							temp3 = (*(pBuffer+m)-0x30);
							for(z=0; z<l-m-1; z++)
							{
								temp3 = temp3 * 10;	
							}
							temp2 += temp3;
						}
						Position.HighOffset = -temp2;
					}
					else//��ƫ��
					{
						for(m=j+1; m<l; m++)
						{
							temp3 = (*(pBuffer+m)-0x30);
							for(z=0; z<l-m-1; z++)
							{
								temp3 = temp3 * 10;	
							}
							temp2 += temp3;
						}
						Position.HighOffset = temp2;
					}
                                  }
				default : break;
			}
			j = i;
		}
	}
	if(1 == flag)
	{
		GpggaParseErrorCount++;
	}
	else
	{
		GpggaParseErrorCount = 0;	
	}
}
void Gps_CopyToPosition(GPS_STRUCT *dest)
{
    Ql_memcpy(&Position,dest,sizeof(GPS_STRUCT));
}
/*********************************************************************
//��������	:GPS_CopygPosition(GPS_STRUCT *dest, GPS_STRUCT *src)
//����		:����һ�����µ�gpsλ�����ݣ�����һ����Ч�Ķ�λ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_CopygPosition(GPS_STRUCT *dest)
{
    #if 1
    memcpy(dest,&gPosition,sizeof(GPS_STRUCT));
    #else
    dest->Year 		= gPosition.Year;		//��
    dest->Month 		= gPosition.Month;		//��
    dest->Date 		= gPosition.Date;		//��
    dest->Hour 		= gPosition.Hour;		//ʱ
    dest->Minute 		= gPosition.Minute;		//��
    dest->Second 		= gPosition.Second;		//��
    dest->North		= gPosition.North;		//1:��γ,0:��γ
    dest->Latitue_D 	= gPosition.Latitue_D;	//γ��,��
    dest->Latitue_F 	= gPosition.Latitue_F;	//γ��,��
    dest->Latitue_FX 	= gPosition.Latitue_FX;	//γ��,�ֵ�С��,��λΪ0.0001��
    dest->East 		= gPosition.East;		//1:����0:����
    dest->Longitue_D 	= gPosition.Longitue_D;	//����,��,���180��
    dest->Longitue_F 	= gPosition.Longitue_F;	//����,��		
    dest->Longitue_FX 	= gPosition.Longitue_FX;	//����,�ֵ�С��,��λΪ0.0001��
    dest->Speed 		= gPosition.Speed;		//�ٶ�,��λΪ����/Сʱ
    dest->SpeedX 		= gPosition.SpeedX;		//�ٶȵ�С��
    dest->Course 		= gPosition.Course;		//����,��λΪ��
    dest->High 		= gPosition.High;		//����,��λΪ��
    dest->HighOffset 	= gPosition.HighOffset;		//����ƫ��,��λΪ��
    dest->SatNum 		= gPosition.SatNum;		//��������
    dest->HDOP 		= gPosition.HDOP;		//ˮƽ��������
    dest->Status 		= gPosition.Status;	//1:��Ч��λ 0:��Ч��λ
    dest->Error 		= gPosition.Error;	//1:GPSģ���������������  0:ģ������
    #endif
}
/*********************************************************************
//��������	:GPS_CopyPosition(GPS_STRUCT *dest, GPS_STRUCT *src)
//����		:����һ�������յ���gpsλ�����ݣ�����һ������Ч�Ķ�λ����
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_CopyPosition(GPS_STRUCT *dest)
{
    #if 1
    memcpy(dest,&Position,sizeof(GPS_STRUCT));
    #else
	dest->Year 		= Position.Year;		//��
	dest->Month 		= Position.Month;		//��
	dest->Date 		= Position.Date;		//��
	dest->Hour 		= Position.Hour;		//ʱ
	dest->Minute 		= Position.Minute;		//��
	dest->Second 		= Position.Second;		//��
	dest->North		= Position.North;		//1:��γ,0:��γ
	dest->Latitue_D 	= Position.Latitue_D;	//γ��,��
	dest->Latitue_F 	= Position.Latitue_F;	//γ��,��
	dest->Latitue_FX 	= Position.Latitue_FX;	//γ��,�ֵ�С��,��λΪ0.0001��
	dest->East 		= Position.East;		//1:����0:����
	dest->Longitue_D 	= Position.Longitue_D;	//����,��,���180��
	dest->Longitue_F 	= Position.Longitue_F;	//����,��		
	dest->Longitue_FX 	= Position.Longitue_FX;	//����,�ֵ�С��,��λΪ0.0001��
	dest->Speed 		= Position.Speed;		//�ٶ�,��λΪ����/Сʱ
	dest->SpeedX 		= Position.SpeedX;		//�ٶȵ�С��
	dest->Course 		= Position.Course;		//����,��λΪ��
	dest->High 		= Position.High;		//����,��λΪ��
    dest->HighOffset 	= Position.HighOffset;		//����ƫ��,��λΪ��
	dest->SatNum 		= Position.SatNum;		//��������
	dest->HDOP 		= Position.HDOP;		//ˮƽ��������
	dest->Status 		= Position.Status;	//1:��Ч��λ 0:��Ч��λ
	dest->Error 		= Position.Error;	//1:GPSģ���������������  0:ģ������
    #endif
}
/*********************************************************************
//��������	:GPS_UpdatagPosition(void)
//����		:���±���gPosition
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_UpdatagPosition(void)
{
    #if 1
    memcpy(&gPosition,&Position,sizeof(GPS_STRUCT));
    #else
	gPosition.Year 		= Position.Year;		//��
	gPosition.Month 	= Position.Month;		//��
	gPosition.Date 		= Position.Date;		//��
	gPosition.Hour 		= Position.Hour;		//ʱ
	gPosition.Minute 	= Position.Minute;		//��
	gPosition.Second 	= Position.Second;		//��
	gPosition.North		= Position.North;		//1:��γ,0:��γ
	gPosition.Latitue_D 	= Position.Latitue_D;	//γ��,��
	gPosition.Latitue_F 	= Position.Latitue_F;	//γ��,��
	gPosition.Latitue_FX 	= Position.Latitue_FX;	//γ��,�ֵ�С��,��λΪ0.0001��
	gPosition.East 		= Position.East;		//1:����0:����
	gPosition.Longitue_D 	= Position.Longitue_D;	//����,��,���180��
	gPosition.Longitue_F 	= Position.Longitue_F;	//����,��		
	gPosition.Longitue_FX 	= Position.Longitue_FX;	//����,�ֵ�С��,��λΪ0.0001��
	gPosition.Speed 	= Position.Speed;		//�ٶ�,��λΪ����/Сʱ
	gPosition.SpeedX 	= Position.SpeedX;		//�ٶȵ�С��
	gPosition.Course 	= Position.Course;		//����,��λΪ��
	gPosition.High 		= Position.High;		//����,��λΪ��
    gPosition.HighOffset    = Position.HighOffset;		//����ƫ��,��λΪ��
	gPosition.SatNum 	= Position.SatNum;		//��������
	gPosition.HDOP 		= Position.HDOP;		//ˮƽ��������
	gPosition.Status 	= Position.Status;	//1:��Ч��λ 0:��Ч��λ
	gPosition.Error 	= Position.Error;	//1:GPSģ���������������  0:ģ������
    #endif
}
/*********************************************************************
//��������	:GPS_ReadGpsSpeed(void)
//����		:��ȡGPS�ٶ�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Gps_ReadSpeed(void)
{
	u8	Speed;

    Speed = (u8)(gPosition.Speed_D*1.8520);
	
	//if(Speed < 3)//GPS�ٶ�С��3����/Сʱ��ǿ�Ƶ���0
    if(Speed <= 5)//GPS�ٶ�С��5����/Сʱ��ǿ�Ƶ���0,dxl,2014.7.26
	{
		Speed = 0;
	}
        
	return Speed;
}
/*********************************************************************
//��������	:GPS_ReadGpsStatus(void)
//����		:��ȡGPS��״̬
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Gps_ReadStatus(void)
{
	return gPosition.Status;
}
/*********************************************************************
//��������	:GPS_ReadGpsCourse(void)
//����		:��ȡGPS�ķ���
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u16 Gps_ReadCourse(void)
{
	return gPosition.Course;
}
/*********************************************************************
//��������	:GPS_ReadGpsCourseDiv2(void)
//����		:��ȡGPS�ķ��򣨳�����2��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Gps_ReadCourseDiv2(void)
{
	u8	Course;

	Course = gPosition.Course >> 1;

	return Course;
}
/*********************************************************************
//��������	:Gps_PowerOnClearPosition(void)
//����		:����ʹ�þ�γ�ȳ�ʼ��Ϊ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_PowerOnClearPosition(void)
{
	memset(&gPosition,0,sizeof(GPS_STRUCT));
}

/*********************************************************************
//��������	:GPS_PowerOnUpdataPosition(void)
//����		:�ϵ���¾�γ��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_PowerOnUpdataPosition(void)
{
  u8	Buffer[12] = {0};
    u8	PramLen = 0;

    PramLen = FRAM_BufferRead(Buffer, 11, FRAM_LAST_LOCATION_ADDR); 
	if(11!=PramLen)
	{
		Ql_memset(&gPosition,0,sizeof(GPS_STRUCT));
	}
    else
    {
        gPosition.North      	= Buffer[0];	//1:��γ0:��γ,1�ֽ� 
        gPosition.Latitue_D  	= Buffer[1];//��,1�ֽ�
        gPosition.Latitue_F  	= Buffer[2];//��,1�ֽ�
        gPosition.Latitue_FX 	= Buffer[3]*256 + Buffer[4];//��С������,��λΪ0.0001��,2�ֽ�,���ֽ�
        gPosition.East	 	    = Buffer[5];//1:����0:����,1�ֽ�
        gPosition.Longitue_D 	= Buffer[6];//�� ���180��,1�ֽ�,
        gPosition.Longitue_F 	= Buffer[7];//�֣�1�ֽڣ�			
        gPosition.Longitue_FX 	= Buffer[8]*256 + Buffer[9];//���ȷֵ�С������,��λΪ0.0001��,2�ֽ�,���ֽ�
        gPosition.Speed = 0;
        gPosition.SpeedX = 0;
        gPosition.Course = 0;
        gPosition.High = 0;
        gPosition.HighOffset = 0;
        gPosition.SatNum = 0;
        gPosition.HDOP = 0;
        gPosition.Status =  0; 
        gPosition.Error = 0;
    }

	Ql_memset((void*)&STU_GPS,0,sizeof(TDF_STU_GPS));
	STU_GPS.Reflash_Flag = 1;
}
/*********************************************************************
//��������	:GPS_SavePositionToFram(void)
//����		:��λ�ñ��浽eeprom��
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:ִ������ACC ON->ACC OFF || ����->������
*********************************************************************/
void Gps_SavePositionToFram(void)
{
    u8	Buffer[12];

    Buffer[0] = gPosition.North;	//1:��γ0:��γ,1�ֽ� 
    Buffer[1] = gPosition.Latitue_D;//��,1�ֽ�
    Buffer[2] = gPosition.Latitue_F;//��,1�ֽ�
    Buffer[3] = (gPosition.Latitue_FX&0xff00)>>8;//��С������,��λΪ0.0001��,2�ֽ�,���ֽ�
    Buffer[4] = gPosition.Latitue_FX&0x00ff;//��С������,��λΪ0.0001��,2�ֽ�,���ֽ�
    Buffer[5] = gPosition.East;//1:����0:����,1�ֽ�
    Buffer[6] = gPosition.Longitue_D;//�ȣ�1�ֽ�
    Buffer[7] = gPosition.Longitue_F;//1�ֽ�			
    Buffer[8] = (gPosition.Longitue_FX&0xff00)>>8;//���ȷֵ�С������,��λΪ0.0001��,2�ֽ�,���ֽ�
    Buffer[9] = gPosition.Longitue_FX&0x00ff;//���ȷֵ�С������,��λΪ0.0001��,2�ֽ�,���ֽ�
    Buffer[10]= gPosition.Status ;	//1:��Ч��λ 0:��Ч��λ

    if(0 == Buffer[0])
    {
      
    }
    FRAM_BufferWrite(FRAM_LAST_LOCATION_ADDR, Buffer, 11);

}
/*********************************************************************
//��������	:Gps_GetRunFlag
//����		:��ȡȥƯ����ʻ��־
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:1Ϊ��ʻ��0Ϊͣʻ
//��ע		:
*********************************************************************/
u8 Gps_GetRunFlag(void)
{
    return GpsRunFlag;
}

/*********************************************************************
//��������	:Gps_IsInRunning
//����		:�жϵ�ǰ�Ǵ�����ʻ״̬����ͣʻ״̬
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:1Ϊ��ʻ��0Ϊͣʻ
//��ע		:1�����1��
*********************************************************************/
void Gps_IsInRunning(void)
{
    static u32 UpCount = 0;
    static u32 DownCount = 0;    
    u8 Speed;
    
	//Speed = Position.Speed;    
    Speed = (Position.Speed*18520+Position.SpeedX*1852+5000)/10000;
    if(Speed > 5)//dxl,2014.8.26,ԭ�����ٶȴ���0,���ھ���Ϊ5km/h
    {
        DownCount = 0;
        UpCount++;
        if(UpCount >= 10)
        {
            GpsRunFlag = 1;//����0����10��Ϊ��ʻ
        }
    }
    else
    {
        UpCount = 0;
        DownCount++;
        if(DownCount >= 10)
        {
            GpsRunFlag = 0;//����0����10��Ϊͣʻ
        }
    }
}
/*************************************************************
** ��������: Public_GetCurGpsCoordinate
** ��������: ��õ�ǰ����(γ��ֵ������ֵ)
** ��ڲ���: flag����:1:����,0:˳��
** ���ڲ���: 
** ���ز���: 
** ȫ�ֱ���: ��
** ����ģ��: ��
*************************************************************/
void Public_GetCurGpsCoordinate(unsigned char *data,unsigned char flag)
{  
    unsigned long temp;
    GPS_STRUCT Position;
    //��GPS��Ϣ
    Gps_CopygPosition(&Position);
    //��γ��ֵ
    temp = (Position.Latitue_FX*100UL+Position.Latitue_F*1000000UL)/60+(Position.Latitue_D*1000000UL);
    Public_Mymemcpy(&data[0],(unsigned char *)&temp,4,flag);
    //��ȡ����ֵ
    temp = (Position.Longitue_FX*100UL+Position.Longitue_F*1000000UL)/60+(Position.Longitue_D*1000000UL);
    Public_Mymemcpy(&data[4],(unsigned char *)&temp,4,flag);
}
extern s32 m_pwrOnReason;

/***********************************************
����:��γ�����ݴ������˲�
����:0--Ư�Ƶ����ݣ�>0--����������
��ע:�ο������ʽ������γ�ȵĶ���ֲ���ת��Ϊ�������ľ�γ��
************************************************/
u8 GPS_DataFilter(void)
{
	double	   Speed;
	u32 Lat_Temp = 0ul;
	u32 Log_Temp = 0ul;
	static u8 RtcAdjFlg = 0;
	static u8 LocValidCnt = 0;
	static u32 Last_Lat = 0ul;
	static u32 Last_Log = 0ul;
	static u32 Delat_Lat = 0ul;
	static u32 Delat_Log = 0ul;
	u8	val = 0;
	//����ģ������־
    Io_WriteAlarmBit(ALARM_BIT_GNSS_FAULT, RESET);
	Lat_Temp = (Position.Latitue_FX*100UL+Position.Latitue_F*1000000)/60+(Position.Latitue_D*1000000UL);//γ��
	Log_Temp = (Position.Longitue_FX*100UL+Position.Longitue_F*1000000)/60+(Position.Longitue_D*1000000UL);//����
	//Lat_Temp = Position.Latitue*10000;
	//Log_Temp = Position.Longitue*10000;
	
	/*�쳯��γ�ȷ�Χ����γ3~53������73~135*/
	//if((Lat_Temp < 3000000)||(Lat_Temp > 53000000))return 0;

	//if((Log_Temp < 73000000)||(Log_Temp > 135000000))return 0;
	
	//��û�ж�λ
	if(!Position.Status)
	{
		if(RTCPWRON != m_pwrOnReason)//����������ӻ���
		{
			val = 120;
		}
		else
		{
			val = 30;
		}
		if(STU_GPS.Not_Locate_Cnt++ > val)//2����û��λ�������ˣ����ϱ���־
		{
			STU_GPS.Not_Locate_Cnt= 0;
			//������,������־��0
        	Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
			if(ReportStatus_Off == Report_GetReportStatus()){
			Report_SetReportStatus(ReportStatus_Init);
			APP_DEBUG("<\r\n...gpsδ��λ....\r\n");
			}
		}
		else
		if(STU_GPS.Not_Locate_Cnt++ > 10)
		{			
			//�ٶ���0
			Position.Speed_D = 0.0;
			gPosition.Speed_D = 0.0;
            Position.Speed = 0;
            gPosition.Speed = 0;
            Position.SpeedX = 0;
            gPosition.SpeedX = 0;
            //��������0
            Position.SatNum = 0;
            gPosition.SatNum = 0;
			LocationFlag = 0;
		}
		STU_GPS.Valid = 0;
		STU_GPS.Speed_Up_Cnt = 0;
		STU_GPS.Speed_Down_Cnt = 0;
	}
	else
	{
		if(0 == RtcAdjFlg){
		if(LocValidCnt++ >10 )
		{
			Gps_AdjustRtc(&Position);
		}RtcAdjFlg = 1;
		}
		STU_GPS.Not_Locate_Cnt= 0;
		Speed = Position.Speed_D*1.8520;
		//APP_DEBUG("...Speed=%0.4f...\r\n",Speed);
	    if(Speed > 5.0)//dxl,2014.8.26,ԭ�����ٶȴ���0,���ھ���Ϊ5km/h
	    {	        
	        STU_GPS.Speed_Up_Cnt++;
	        if(STU_GPS.Speed_Up_Cnt >= 10)
	        {
	        	STU_GPS.Speed_Down_Cnt = 0;
	        	STU_GPS.Speed_Up_Cnt = 0; 
	            STU_GPS.Reflash_Flag = 1;
	        }
	    }
	    else
	    {	        
			STU_GPS.Speed_Down_Cnt++;
			APP_DEBUG("...Speed_Down_Cnt=%d...\r\n",STU_GPS.Speed_Down_Cnt);
			if(STU_GPS.Speed_Down_Cnt >= 3600)//ͣʻ1Сʱ����ˢ��GPS����һ��
			{
				STU_GPS.Speed_Down_Cnt = 0;
				STU_GPS.Reflash_Flag = 1;
			}
			else
	        if(STU_GPS.Speed_Down_Cnt >= 10)
	        {  
	        	STU_GPS.Speed_Up_Cnt = 0;
	            STU_GPS.Reflash_Flag = 0;
	        }
	    }
		//GPS����ˢ��״̬
		if(0 == STU_GPS.Reflash_Flag)
		{
			/******��ֹ״̬����γ�Ȳ�����******/
		    Lat_Temp = Last_Lat;
		    Log_Temp = Last_Log;
			STU_GPS.Valid = 0;			
		}
		else
		{
			/*****��γ���Ƿ�Ϸ����Ƿ���Ҫ����****/
			if(Last_Lat == 0ul)
			{
				Last_Lat = Lat_Temp;
				Last_Log = Log_Temp;
			}
			else
			{
				Delat_Lat = abs(Last_Lat - Lat_Temp);
				Delat_Log = abs(Last_Log - Log_Temp);
				APP_DEBUG("...Delat_Lat=%d,Delat_Log=%d...\r\n",Delat_Lat,Delat_Log);
				if((Delat_Lat > 1000)||(Delat_Log > 1000))
				{
				    /**ƮѽƮ**/
				    Last_Lat = 0ul;
				    Last_Log = 0ul;
				    STU_GPS.Valid = 0;
				}
				else
				{
				    Last_Lat = Lat_Temp;
				    Last_Log = Log_Temp;
					STU_GPS.Valid = 1;
					LocationFlag = 1;
					APP_DEBUG("<\r\n...GPS�Ѷ�λ....\r\n");
					
					//��λ������־
	                Io_WriteStatusBit(STATUS_BIT_NAVIGATION, SET);
					if(ReportStatus_Off == Report_GetReportStatus()){
						Report_SetReportStatus(ReportStatus_Init);	
					}
				}
			}
		}
	}

	return (STU_GPS.Valid);

}

/*********************************************************************
//��������	:GpsParse_EvTask(void)
//����		:GPS��������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
#if 1
void Gps_EvTask(void)
{
	if(GPS_DataFilter())
	{
		Gps_UpdatagPosition();
	}
}
#else
void Gps_EvTask(void)
{
    u8	VerifyFlag = 0;
    u8	flag;
    u8	Num;
    u32	s1;
    u32	s2;
    u8  ACC;

    static  u16	DriftCount = 0;
    static  u16	noNavigationCount = 0;//������������������У����������0
    static  u16     VerifyErrorCount = 0;//У����������У����ȷ��0
    static  u32     NavigationCount = 0;//dxl,2014.7.28,�������������������У����������0���ñ������ڴӲ������������˳�ǰ��3�������� 
    static  u8      AdjustRtcFlag = 0;//Уʱ��־���ϵ��Ӳ�����������Уʱ1�Σ����ÿ��50СʱУʱ1��

#if PRODUCT_MODEL_VTKG_22G == PRODUCT_MODEL
	ACC = 1;
#else
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
#endif

    //У���Ƿ�ͨ��
    //if(1 == VerifyFlag)//ͨ��
    {
        VerifyErrorCount = 0;
        //�б���ʱ���ν���
        //GpsParseEnterErrorCount = 0;
        //����ģ������־
        Io_WriteAlarmBit(ALARM_BIT_GNSS_FAULT, RESET);
        /*
        if((0 == GpsOnOffFlag)||
        ((1 == GpsOnOffFlag)&&(0 == AccOffGpsControlFlag)&&(0 == ACC)))//GPSģ��ر�״̬,dxl,2014.9.24ȥ��ACC OFFʱGPS����5�����ڼ�Ľ���    
        {
            Position.Speed = 0;
            gPosition.Speed = 0;  
            Position.SpeedX = 0;
            gPosition.SpeedX = 0;  
            return ;
        }
        */
        //�ж��Ƿ�λ
        //flag = Gps_GprmcIsLocation(GprmcBuffer, GPRMC_BUFFER_SIZE);
		flag = Position.Status;
        if(1 != flag)
        {
        	//APP_DEBUG("<--!!!������������GPS����!!!!-->\r\n");
            NavigationCount = 0;
            //��λæ��־
            //GprmcBufferBusyFlag = 1;
            //����
            //Gps_GprmcParse(GprmcBuffer, GPRMC_BUFFER_SIZE);//Ϊ�б��ͼ����
            //���æ��־
            //GprmcBufferBusyFlag = 0;

            LocationFlag = 0;//��ǰ�㵼��״̬
            //������,������־��0
            Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);//������������ʾ
            noNavigationCount++;
            if(noNavigationCount >= 10)//����10���ֲ�����,�ٶ���0
            {
                noNavigationCount = 0;
                //�ٶ���0
                Position.Speed = 0;
                gPosition.Speed = 0;
                Position.SpeedX = 0;
                gPosition.SpeedX = 0;
                //��������0,liamtsen add 2017-06-13
                gPosition.SatNum = 0;
                
            }
            //�ж�·���·�����Ļ�,�ٶȺ͵�����־��0
            if((1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHUT))
            ||(1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHORT))
            ||(1 == StopExcursionEnable))
            {
                StopExcursionEnable = 0;
                gPosition.Status = 0;
                //������,������־��0
                Io_WriteStatusBit(STATUS_BIT_NAVIGATION, RESET);
            }
        }
        else
        {
            if(0 == AdjustRtcFlag)
            {
                if(SUCCESS == Gps_AdjustRtc(&Position))
                {
                    Gps_SavePositionToFram();
                    AdjustRtcFlag = 1;
                }
            }
            noNavigationCount = 0;
		#if 0
            //*************����GPRMC************
            //��λæ��־
            GprmcBufferBusyFlag = 1;
            //����
            Gps_GprmcParse(GprmcBuffer, GPRMC_BUFFER_SIZE);
            //���æ��־
            GprmcBufferBusyFlag = 0;
            //��¼ʱ��
            PositionTime = 0;//////RTC_GetCounter();
            //*************����GPGGA*************
            //��λæ��־
            GpggaBufferBusyFlag = 1;
            //����
            Gps_GpggaParse(GpggaBuffer, GPGGA_BUFFER_SIZE);
            //���æ��־
            GpggaBufferBusyFlag = 0;
		#endif
            //**************ȥƯ��**********************
            //�б���ʱ���ο�ʼ
            flag = 0;
            //v=s/t >= 200km/h(55m/s)����Ϊ��Ư��,1�ֵȼ���1�����1852��
            //γ�Ⱦ���
            s1 = ((Position.Latitue_D*60+Position.Latitue_F)*1000+Position.Latitue_FX/10)*2;//��2����1.852
            s2 = ((gPosition.Latitue_D*60+gPosition.Latitue_F)*1000+gPosition.Latitue_FX/10)*2; 
            if((s1 > s2)&&(0 != s1)&&(0 != s2))
            {
                if(((s1-s2)/(PositionTime - gPositionTime) > 55)&&(0 != Position.Latitue_D)&&(0 != gPosition.Latitue_D)&&(0 != gPositionTime))//���ϵ�û��������ά��Ϊ0
                {
                    flag = 1;//��Ư�Ƶ�
                }
            }
            else if((0 != s1)&&(0 != s2))
            {
                if(((s2-s1)/(PositionTime - gPositionTime) > 55)&&(0 != Position.Latitue_D)&&(0 != gPosition.Latitue_D)&&(0 != gPositionTime))
                {
                    flag = 1;//��Ư�Ƶ�
                }
            }
            //���Ⱦ���
            s1 = ((Position.Longitue_D*60+Position.Longitue_F)*1000+Position.Longitue_FX/10)*2;//��2����1.852
            s2 = ((gPosition.Longitue_D*60+gPosition.Longitue_F)*1000+gPosition.Longitue_FX/10)*2; 
            if((s1 > s2)&&(0 != s1)&&(0 != s2))
            {
                if(((s1-s2)/(PositionTime - gPositionTime) > 55)&&(0 != Position.Longitue_D)&&(0 != gPosition.Longitue_D)&&(0 != gPositionTime))
                {
                    flag = 1;//��Ư�Ƶ�
                }
            }
            else if((0 != s1)&&(0 != s2))
            {
                if(((s2-s1)/(PositionTime - gPositionTime) > 55)&&(0 != Position.Longitue_D)&&(0 != gPosition.Longitue_D)&&(0 != gPositionTime))
                {
                    flag = 1;//��Ư�Ƶ�
                }
            }
            //����ٶȱ仯,��ǰ�ٶ�С����һ�ε��ٶȱ仯�ʲ�������
            //if(Position.Speed >= LastLocationSpeed)
            if((Position.Speed > LastLocationSpeed)&&(0 != gPositionTime))//dxl,2013.6.4
            {
                if((Position.Speed-LastLocationSpeed) >= (PositionTime - gPositionTime)*6)
                {
                    flag = 1;
                }
            }
            //�����ٶȣ����ٶ�ȥƯ�ƣ�������300���㣨5���ӣ���Ư��ʱ������ʱ�������ȥƯ�ƣ��Ⱦ���ʱ��
            //�����߶�·�����Ļ���Ҫ���¶�λ��StopExcursionEnable = 0,dxl,2013.10.16
            if(1 == Io_ReadAlarmBit(ALARM_BIT_ANT_SHUT))
            {
                StopExcursionEnable = 0;
            }
            if(1 == flag)
            {
                DriftCount++;
                if(DriftCount == 10)
                {
                    Position.Speed = 0;
                    gPosition.Speed = 0;  
                    Position.SpeedX = 0;
                    gPosition.SpeedX = 0;  
                }
                else if(DriftCount >= 300)
                {
                    flag = 0;
                }
            }
            else
            {
                DriftCount = 0;

                if(0 == Gps_GetRunFlag())//ͣʻ
                {
                    if(1 == StopExcursionEnable)
                    {
                        flag = 1;//ͣʻ���������� 
                    }
                }
                else
                {
                    StopExcursionEnable = 0;
                }
            }
            if(0 == LastLocationFlag)
            {
                Num = 3;
            }
            else
            {
                Num = 2;
            }
            if(0 == AccOffGpsControlFlag)//ACC OFFʱGPSģ��ر�
            {
                //��ACC״̬����
            }
            else
            {
                //����ACC״̬����
                ACC = 1;
            }
			
            if((Position.SatNum>Num)&&(Position.HDOP<=8)&&(Position.Speed<=110)&&(0 == flag)&&(1 == ACC))
            //if((Position.SatNum>Num)&&(Position.HDOP<=8)&&(Position.Speed<=110)&&(0 == flag))//liamtsen  �޸� 2017-06-13
            {
                //��λ������־
                NavigationCount++;
                //�ҵ���һ����ֹ��,����ͣ��ȥƯ�ƣ��б���ʱ���ο�ʼ

                if(NavigationCount > 3)//ǰ3���������˳���
                {
                    if(DriftCount >= 300)
                    {
                        DriftCount = 0;
                    }
                    Gps_UpdatagPosition();
					
                    //��¼ʱ��
                    gPositionTime = 0;////////RTC_GetCounter();
                    //��һ���ٶ�
                    LastLocationSpeed = gPosition.Speed;
                    //��λ������־
                    Io_WriteStatusBit(STATUS_BIT_NAVIGATION, SET);
                    LocationFlag = 1;
                    //�ҵ���һ����ֹ��,����ͣ��ȥƯ�ƣ��б���ʱ���ο�ʼ
                    if(0 == Gps_GetRunFlag())
                    {
                        StopExcursionEnable = 1;
                    }
                }
            }
            else
            {
                LocationFlag = 0;
            }            

            //�б���ʱ���ο�ʼ
        }
        //�б���ʱ���ο�ʼ
    }
	
    //�б���ʱ���ο�ʼ
    //�Ӳ�����������,Уʱ

    //Уʱ���б���ʱ����������
    //��¼��1�ζ�λ״̬
    LastLocationFlag = gPosition.Status;


}
#endif
/*********************************************************************
//��������	:Gps_TimeTask(void)
//����		:GPSʱ������
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:1�����1��
*********************************************************************/
FunctionalState Gps_TimeTask(void)
{
    static u8 	count = 0;
    static u8   AccOffSaveFlag = 0;
    static u32  EnterCount = 0;
		static u16	gnss_pwn_cnt = 0;
    u8	ACC;

    //static u32  EnterCount2 = 0;
    static u32  AccOnCount = 0;
    static u32  AccOffCount = 0;

#if PRODUCT_MODEL_VTKG_22G == PRODUCT_MODEL
	ACC = 1;
#else
    ACC = Io_ReadStatusBit(STATUS_BIT_ACC);
#endif
    //�������һ����Ч�㵽����
    count++;
    if(count > 5)
    {
        count = 0;
        if(0 == AccOffGpsControlFlag)//��ACC����
        {
            if(0 == ACC)//ACC ��
            {
                if(0 == AccOffSaveFlag)
                {
                    Gps_SavePositionToFram();
                    AccOffSaveFlag = 1;
                }
            }
            else// ACC��
            {
                AccOffSaveFlag = 0;
                Gps_SavePositionToFram();
            }
        }
        else
        {
            AccOffSaveFlag = 0;
            Gps_SavePositionToFram();
        }
    }

    //��GPS�ٶ��жϵ�ǰ����ʻ����ֹͣ��ֹͣʱ������GPS����
    Gps_IsInRunning();
    if(GpsRunFlag)
  	{
  		gnss_pwn_cnt = 0;
			//APP_DEBUG("<\r\n...������ʻ��....\r\n");
  	}
		else
		{
			//APP_DEBUG("<\r\n...������ֹ....%d\r\n",gnss_pwn_cnt);
			#ifdef __GNSS_NEW_API__
			if(gnss_pwn_cnt++ == 1800)//30���Ӿ�ֹ�ر�gnssģ��
			{
				Ql_GNSS_PowerDown();			
			}
			else if(gnss_pwn_cnt >= 3600)//���60�������¿���gnssģ��
			{
				gnss_pwn_cnt = 0;
				Ql_GNSS_PowerOn(RMC_EN | GGA_EN, Gps_NMEA_Hdlr, &Position); 
			}
			#endif
		}
    //ÿСʱ����һ��ʱ��
    EnterCount++;
    if((EnterCount >= 3600*50)&&(1 == LocationFlag))
    {
        EnterCount = 0;
        Gps_AdjustRtc(&Position);
    }
    
	
    return ENABLE ;
}

/*********************************************************************
//��������	:Gps_UpdateParam
//����		
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
void Gps_UpdateParam(void)
{
    //dxl,2017.2.23,�޴˹���
}
/*********************************************************************
//��������	:Gps_Off_flag
//����		:֪ͨgpsģ���Ѿ��ر�
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 Gps_Off_flag(void)
{
 return GpsOnOffFlag;//=0 ;
}
/*********************************************************************
//��������	:StrcpyNum
//����		:��Դ��ַ����ָ������֮ǰ���ֽڵ�Ŀ���ַ
//����		:
//���		:
//ʹ����Դ	:
//ȫ�ֱ���	:   
//���ú���	:
//�ж���Դ	:  
//����		:
//��ע		:
*********************************************************************/
u8 StrcpyNum(u8 *src,u8 *dest,u8 Endchar)
{
  u8 cpyLen=0;
  while(*src!=Endchar)
  {
    cpyLen++;
    *dest=*src;
    dest++;
    src++;
  }
  return cpyLen;
}
//���GPS���ݽ������������
void Gps_ClrParseErr(void)
{
	GpsParseEnterErrorCount = 0;
	GnssStatus = 0;
}

/******************************************************************************
**                            End Of File
******************************************************************************/


