#ifndef __BLIND_H
#define __BLIND_H

#include "include.h"


#define VERIFY_MODE_HASH	(0)
#define VERIFY_MODE_CRC32	(1)

#define configVerifyMode	(VERIFY_MODE_CRC32)

typedef struct
{
	u32	dataLen;				//�ڵ����ݳ���ä���ڵ㳤��+ä�����ݳ���
	//u32	dataAddr;				//�ڵ����ݵ�ַ
}BLIND_NODE;
typedef union
{
	u32 ulData;
	u8	ucData[4];
}UN_BLIND_DATA;
typedef struct
{
	UN_BLIND_DATA	head;					//����ͷ
	UN_BLIND_DATA	tail;					//����β
	UN_BLIND_DATA	queueLen;				//���г���
	UN_BLIND_DATA	queueCapacity;			//��������
	UN_BLIND_DATA	startAddr;			//�洢��ʼ��ַ
	UN_BLIND_DATA	dataSaveLen;			//ä�����ݳ���
	#if( VERIFY_MODE_CRC32 == configVerifyMode )
	UN_BLIND_DATA	crc32Verify;					//CRC32У��
	#elif( VERIFY_MODE_HASH == configVerifyMode )
	UN_BLIND_DATA	hashVerify;			//��ϣУ��
	#endif
}BLIND_SAVE_STU;

typedef BLIND_SAVE_STU	CirQueue_t;
typedef void * CirQueueHandle_t;

typedef enum {Q_SUCCESS = 0u,Q_FAILED = !Q_SUCCESS}QueueStatus;

#define BLIND_UP_OVERTIME					(15000)	// ä���ϱ���ʱʱ��ms
#define	FLASH_BLIND_PACK_NUM				(5)//ä�������Ŀ
#define	FLASH_BLIND_AREA_SAVE_START_ADDR	(sizeof(BLIND_SAVE_STU))	//ѭ��������ʼ��ַ��ǰ��Ԥ�����Ǵ洢�ڵ���Ϣ��
#define	FLASH_BLIND_AREA_SAVE_END_ADDR		(FLASH_ONE_SECTOR_BYTES*FLASH_BLIND_END_SECTOR)
#define	FLASH_BLIND_AREA_SAVE_NUM			((FLASH_BLIND_AREA_SAVE_END_ADDR-FLASH_BLIND_AREA_SAVE_START_ADDR)/FLASH_BLIND_STEP_LEN-1)

void Blind_Init(void);

//����һ���ź���
int Blind_CreateSemaphore(void);
//�����ź���
void Blind_GiveSemaphore(void);
//��ȡ�ź���
void Blind_TakeSemaphore(bool wait);
//ע��һ����ʱ����������ä���ϱ�
void Blind_Timer_Register(void);
//���½ڵ���Ϣ
void Blind_UpdateCirQueueInfo(void);

//��ȡä���ϱ�״̬
u8 Blind_GetReportState(void);

/**************************************************************************
//��������Blind_Link1Save
//���ܣ��洢һ������1��ä������
//���룺һ��λ����Ϣ������
//�������
//����ֵ��0Ϊ�ɹ�����0Ϊʧ�ܣ����ȳ���ʱ�᷵��ʧ��
//��ע��һ��ä�����ݰ�����ʱ��4�ֽ�+����1�ֽڣ�Ԥ����+У���1�ֽ�+λ����Ϣ����1�ֽ�+λ����Ϣ�����ֽ�
***************************************************************************/
u8 Blind_Link1Save(u8 *pBuffer, u8 length, u8 attribute);

/**************************************************************************
//��������Blind_Link1ReportAck
//���ܣ�����1ä������Ӧ����
//���룺��
//�������
//����ֵ����
//��ע���յ�����1��ä������Ӧ��ʱ����ô˺���
***************************************************************************/
void Blind_Link1ReportAck(void);

#endif
