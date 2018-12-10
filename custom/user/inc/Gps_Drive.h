

#ifndef __GPS_DRIVE_H__
#define __GPS_DRIVE_H__

#include "ql_type.h"

#define __GNSS_NEW_API__            // receive NMEA by callback funciton

#define MSG_GNSS_ON   0
#define MSG_GNSS_OFF  1

#define GPS_TIMER_PERIOD			1000								//GPS��λ��ѯʱ��
#define TIMEOUT_30S_PERIOD  	 	20000									//GPS��λ��ʱʱ��

#define GPRMC_BUFFER_SIZE  200
#define GPGGA_BUFFER_SIZE  200

typedef enum{
	STATE_WAIT_GPRS_NET=0,					//�ȴ�MC20ģ��ע������ɹ�
	STATE_TIME_SYNC,						//��ѯʱ���Ƿ�ͬ��������ͬ��
	STATE_NTP_WAIT,						//�ȴ�ʱ��ͬ��
	STATE_GPS_NET,						//��ȡ��վλ�ü�������
	STATE_GET_LOC,							//��ȡ��վλ��
	STATE_WAIT_LOC,						//�ȴ���ȡ��վλ��
	STATE_EPO_START,						//ʹ��EPO
	STATE_CHECK_FIX,						//��ȡGPS����
	STATE_GNSS_OFF,   						//�ر�EPO GNSS
	STATE_GPS_IDLE//����״̬
}Enum_GPSSTATE;												//GPS��λ����


//��ʼ�� gps  ��gps��ʱ��
void init_GPStimer(void);
void JO_OpenGPSOutTimer(void);
void JO_OpenGPSTimer(void);
void SentGnssOnMsgToTask(void);
void  GPS_Conversion_Agreement(u8 *gpsbuf);
void CtrGnssOn(void);
//�ⲿ�ر�GNSS�����øú�����GNSS�رգ�ͬʱ����GNSS״̬��ʱ����ر�
void CtrGnssOFF(void);
//����gnss epo״̬
bool Gnss_ReadStatus(void);
void Gps_NMEA_Hdlr(u8 *multi_nmea, u16 len, void *customizePara);

#endif




