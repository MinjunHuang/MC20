/************************************************************************
//�������ƣ�Blind.c
//���ܣ���ģ��ʵ��ä���������ܡ�
//�汾�ţ�V0.1
//��Ȩ˵������Ȩ�����������������¼����������޹�˾
//�����ˣ�dxl
//����ʱ�䣺2014.10
//�汾��¼���汾��¼������汾���޸��ˡ��޸�ʱ�䡢�޸�ԭ���޸ļ�Ҫ˵��
//V0.1��ÿ�ΰ�5��λ����Ϣ���������ֻ���յ�ƽ̨Ӧ���ŻᲹ����һ����
//����һֱ������ǰ������෢��10�Σ�ÿ��20�롣��10�κ���Ȼû��Ӧ��
//���ȴ�30���Ӻ����ط���ǰ����
*************************************************************************/

/********************�ļ�����*************************/
#include <stdio.h>
#include <string.h>
#include "Blind.h"
#include "Public.h"
#include "RadioProtocol.h"
#include "report.h"
#include "tcp_log.h"

/********************�ⲿ����*************************/
extern u8 ShareBuffer[];

static u8 							reportAck = 0;
static CirQueueHandle_t pBlindCtrl = NULL;
static CirQueue_t 			BlindCtrlBackup;//����
static int 							iSemaphoreId;
static u8  							blindProbe = 0;// ̽��
static u8 							shamLinkCount = 0;

//���½ڵ���Ϣ
void Blind_UpdateCirQueueInfo(void);
QueueStatus Blind_IsCirQueueEmpty(CirQueueHandle_t xCirQueue);
static void Blind_NodeBackup(void);
static void Blind_NodeResume(void);

//ä�����ݴ���ϱ�
QueueStatus Blind_DataPackUploading(CirQueueHandle_t xCirQueue,u32 timeout);
//��ʼ��
void Blind_Init(void);

//ä�����ݴ洢�ṹ,ä����Ϣ�ǹ̶��洢��ַ��Ϊ�������趨
/*

____________________________________________________________________________
|ä����Ϣ|ä		��		��		��		ѭ		��		��		��	|
|_____________|_____________________________________________________________|
|			   |																|
|_____________|_____________________________________________________________|


*/
void proc_blind_task(s32 taskId)
{
	QueueStatus upSta;
	u8 terminalAuthFlag;
	Blind_Init();
	while(1)
	{
		Blind_TakeSemaphore(TRUE);	
		
		//ä���ϱ�
		terminalAuthFlag = GetTerminalAuthorizationFlag();
		APP_DEBUG("\r\n<-- terminalAuthFlag = %d -->\r\n",terminalAuthFlag);
		upSta = Blind_DataPackUploading(pBlindCtrl,BLIND_UP_OVERTIME);
		if(Q_SUCCESS == upSta)
		{
			APP_DEBUG("\r\n<-- ����ä���ϱ� ....-->\r\n");
		}
		else
		{
			CirQueue_t * const pxCirQueue = (CirQueue_t *)pBlindCtrl;
			APP_DEBUG("\r\n...ä���ϱ���� ...ʣ��ä������Ϊ:%d...head:%d...tail:%d\r\n", \
			pxCirQueue->queueLen.ulData,pxCirQueue->head.ulData,pxCirQueue->tail.ulData);
			//Blind_UpdateCirQueueInfo();
			Ql_Sleep(300000);//ÿ��5���Ӽ��һ�η�ֹ���µ�ä������
			if(terminalAuthFlag)
			{
				Blind_GiveSemaphore();
			}
		}
	}
}

/*********************************************************************************************************
*                                          Blind_DataSave
*
* Description : �洢ä������
*
* Argument(s) : pBuffer--->����ָ��
*			   Addr----->���ݵ�ַ
*			   length---->���ݳ���
*
* Return(s)   : none
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
void Blind_DataSave(u8 *pBuffer, u32 Addr,u32 length)
{
	sFLASH_WriteBuffer(pBuffer,Addr,length);//д��flash
}
/*********************************************************************************************************
*                                          Blind_IsCirQueueEmpty
*
* Description : �����пգ��������=0��Ϊ�գ���ʱû��Ԫ�ؿ��Գ���
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ����״̬
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
QueueStatus Blind_IsCirQueueEmpty(CirQueueHandle_t xCirQueue)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;
	
  if (pxCirQueue->queueLen.ulData == 0)
  {
      return Q_SUCCESS;
  }
  return Q_FAILED;
}

/*********************************************************************************************************
*                                          Blind_IsCirQueueFull
*
* Description : �����������������=���������ʱ������Ԫ�����
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ����״̬
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
QueueStatus Blind_IsCirQueueFull(CirQueueHandle_t xCirQueue)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;
	
  if (pxCirQueue->queueLen.ulData == pxCirQueue->queueCapacity.ulData)
  {
      return Q_SUCCESS;
  }
	return Q_FAILED;
}

/*********************************************************************************************************
*                                          Blind_GetCirQueueLength
*
* Description : ��ȡ���г���
*
* Argument(s) : ѭ������ָ��
*
* Return(s)   : ���г���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
int	Blind_GetCirQueueLength(CirQueueHandle_t xCirQueue)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;
	
	return	pxCirQueue->queueLen.ulData;
}

/*********************************************************************************************************
*                                          Blind_EnCirQueue
*
* Description : ��Ԫ�����
*
* Argument(s) : pBuffer:����ӵ�����ָ�� 
*								BufferLen:������ݳ���
*								pQ:ѭ������ָ��
*
* Return(s)   : ������ݳ���
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
QueueStatus Blind_EnCirQueue(u8 *pBuffer, u32 BufferLen,CirQueueHandle_t xCirQueue)
{
	u8* pBuf = pBuffer;
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;

	if((0 == BufferLen)||(NULL == pBuf))
	{
		return Q_FAILED;
	}
	
    if (Q_FAILED == Blind_IsCirQueueEmpty(xCirQueue))//ä����Ϊ��
	{
		if(pxCirQueue->tail.ulData == pxCirQueue->head.ulData)//���׷β
		{
		//ͷ��ǰ��,ä�����µ����ݸ�����
			pxCirQueue->head.ulData++;
			pxCirQueue->head.ulData = pxCirQueue->head.ulData % pxCirQueue->queueCapacity.ulData;
		}
	}
	//�洢����
	pxCirQueue->startAddr.ulData = FLASH_BLIND_AREA_SAVE_START_ADDR + pxCirQueue->tail.ulData*FLASH_BLIND_STEP_LEN;
	pxCirQueue->dataSaveLen.ulData = BufferLen;
	Ql_memmove(pBuf+4,pBuf,BufferLen);
	Ql_memcpy(pBuf,pxCirQueue->dataSaveLen.ucData,4);//���ݳ���ҲҪ�洢
	Blind_DataSave(pBuf,pxCirQueue->startAddr.ulData,BufferLen+4);
	APP_DEBUG("\r\n...д��ä����ַΪ:%dä������Ϊ:%d\r\n", pxCirQueue->startAddr.ulData,pxCirQueue->dataSaveLen.ulData);
	pxCirQueue->tail.ulData++;//����β������
	//��Ϊ�����ǻ��Σ�����tail��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
	pxCirQueue->tail.ulData = pxCirQueue->tail.ulData%pxCirQueue->queueCapacity.ulData;

	//�������û���ͼ�1������ͱ�������
	if (Q_FAILED == Blind_IsCirQueueFull(xCirQueue))
	{
	  pxCirQueue->queueLen.ulData++;
	}
	APP_DEBUG("\r\n...��ǰä������Ϊ:%d...head:%d...tail:%d\r\n", \
		pxCirQueue->queueLen.ulData,pxCirQueue->head.ulData,pxCirQueue->tail.ulData);
	//���½ڵ���Ϣ�ѽڵ���Ϣд��Flash,������Ϊ�˷�ֹ�쳣�ػ�����ä����ʧ���Ͼ�����ʤ��
	//if(0 == pxCirQueue->queueLen.ulData % 20)
	//Blind_UpdateCirQueueInfo();
	
	return Q_SUCCESS;
}

//����ä������
u8 Blind_Link1Save(u8 *pBuffer, u8 length, u8 attribute)
{
	Blind_EnCirQueue(pBuffer, length,pBlindCtrl);
	if(blindProbe)
	{
		blindProbe = 0;
	}
}

/*********************************************************************************************************
*                                          Blind_DeCirQueue
*
* Description : Ԫ�س���
*
* Argument(s) : pBuffer:���ӵ�����ָ�� 
*			   xCirQueue:ѭ������ָ��
*
* Return(s)   : �����е����ݳ���(�����峤��( 2�ֽ�)+ ������ĳ���)
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
u32 Blind_DeCirQueue(u8 *pBuffer,CirQueueHandle_t xCirQueue)
{
	BLIND_SAVE_STU temp;
	u8* pBuf = pBuffer;
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;
	if (Q_SUCCESS == Blind_IsCirQueueEmpty(xCirQueue))
	{
		//ǿ������
		//pxCirQueue->head.ulData = 0;
		//pxCirQueue->tail.ulData = 0;
	  	return 0;
	}
	
	//Ѱַ
	pxCirQueue->startAddr.ulData = FLASH_BLIND_AREA_SAVE_START_ADDR + pxCirQueue->head.ulData*FLASH_BLIND_STEP_LEN;
	//��ȡä�����ݳ���
	sFLASH_ReadBuffer(temp.dataSaveLen.ucData,pxCirQueue->startAddr.ulData,4);
	//APP_DEBUG("\r\n...��ȡä����ַΪ:%dä������Ϊ:%d\r\n", pxCirQueue->startAddr.ulData,temp.dataSaveLen.ulData);
	//��ȡä������
	*pBuf++ = temp.dataSaveLen.ucData[1];
	*pBuf++ = temp.dataSaveLen.ucData[0];
	sFLASH_ReadBuffer(pBuf,pxCirQueue->startAddr.ulData+4,temp.dataSaveLen.ulData);
	pxCirQueue->head.ulData++;
	//��Ϊ�����ǻ��Σ�����head��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
	pxCirQueue->head.ulData = pxCirQueue->head.ulData % pxCirQueue->queueCapacity.ulData;
	pxCirQueue->queueLen.ulData--; 
	
	return ( temp.dataSaveLen.ulData+2);//���������ֽڵ����ݳ���

}

typedef struct
{
	u16	itemNum;
	u32	blindDataLens;
}BLIND_PACK_INFO;
/*********************************************************************************************************
*                                          Blind_DataPack
*
* Description : ä�����ݴ��
*
* Argument(s) : pBuffer:���ӵ�����ָ�� 
*			   xCirQueue:ѭ������ָ��
*
* Return(s)   : �����������������ȵĽṹ��
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
BLIND_PACK_INFO Blind_DataPack(u8 *pBuffer,CirQueueHandle_t xCirQueue)
{
	u8 i;
	u8* pBuf = pBuffer;
	u32 dataLen=0;
	BLIND_PACK_INFO upInfo={0,0};
	//s32 ret;
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;
	
	for(i = 0;i < FLASH_BLIND_PACK_NUM;i++)
	{
		dataLen = Blind_DeCirQueue(pBuf,xCirQueue);
		pBuf += dataLen;
		if(dataLen)
		{
			upInfo.itemNum++;
		}
		upInfo.blindDataLens += dataLen;
	}
	
	return upInfo;
}


/*********************************************************************************************************
*                                          Blind_DataPackUploading
*
* Description : ä�����ݴ��
*
* Argument(s) : timeout:ä����ʱ���ص����
*			   xCirQueue:ѭ������ָ��
*
* Return(s)   : none
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
QueueStatus Blind_DataPackUploading(CirQueueHandle_t xCirQueue,u32 timeout)
{
	BLIND_PACK_INFO upInfo;
	s32	ret;
	CirQueue_t * const pxCirQueue = (CirQueue_t *)xCirQueue;

	if (Q_SUCCESS == Blind_IsCirQueueEmpty(xCirQueue))
	{
		
	  	return Q_FAILED;
	}

	//backup
	Blind_NodeBackup();
	
	upInfo = Blind_DataPack(ShareBuffer+3,xCirQueue);
	
	ShareBuffer[0] = (u8)(upInfo.itemNum>>8);//�����ֽڱ�ʾ����
	ShareBuffer[1] = (u8)upInfo.itemNum;
	ShareBuffer[2] = 1;//0:������λ�������㱨;1:ä������
	//ShareBuffer[3] = (u8)(upInfo.blindDataLens>>8);//�����ֽ�λ�û㱨��Ϣ����
	//ShareBuffer[4] = (u8)upInfo.blindDataLens;
	APP_DEBUG("\r\n<-- Blind_Report %d   %d-->\r\n",upInfo.itemNum,upInfo.blindDataLens);
	RadioProtocol_PostionInformation_BulkUpTrans(ShareBuffer,upInfo.blindDataLens+3);

	APP_DEBUG("\r\n...ʣ��ä������Ϊ:%d...head:%d...tail:%d\r\n", \
		pxCirQueue->queueLen.ulData,pxCirQueue->head.ulData,pxCirQueue->tail.ulData);
	// ����ä���ϱ��ص�������ʱ��,����
    //ret = Ql_Timer_Start(LOGIC_BLIND_REPORT_TMR_ID, timeout,FALSE);
	Ql_OS_SendMessage(main_task_id, MSG_ID_BLIND_CALLBACK, timeout, 0);
	blindProbe = 1;// ̽������
	return Q_SUCCESS;
}

static void Blind_NodeBackup(void)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)pBlindCtrl;

	Ql_memcpy((void*)&BlindCtrlBackup.head,(void*)&pxCirQueue->head,sizeof(BLIND_SAVE_STU));
}

static void Blind_NodeResume(void)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)pBlindCtrl;

	Ql_memcpy((void*)&pxCirQueue->head,(void*)&BlindCtrlBackup.head,sizeof(BLIND_SAVE_STU));
	//��ä���ϱ��ڼ���д��ä��Ҫ���⴦��
	if(0 == blindProbe)
	{
		if(pxCirQueue->tail.ulData == pxCirQueue->head.ulData)//���׷β
		{
		//ͷ��ǰ��,ä�����µ����ݸ�����
			pxCirQueue->head.ulData++;
			pxCirQueue->head.ulData = pxCirQueue->head.ulData % pxCirQueue->queueCapacity.ulData;
		}
		pxCirQueue->tail.ulData++;//����β������
		//��Ϊ�����ǻ��Σ�����tail��Ҫͨ��ȡģ��ʵ��ת�ص�0λ��
		pxCirQueue->tail.ulData = pxCirQueue->tail.ulData%pxCirQueue->queueCapacity.ulData;

		//�������û���ͼ�1������ͱ�������
		if (Q_FAILED == Blind_IsCirQueueFull(pBlindCtrl))
		{
		  pxCirQueue->queueLen.ulData++;
		}					
	}
}
//���½ڵ���Ϣ
void Blind_UpdateCirQueueInfo(void)
{
	CirQueue_t * const pxCirQueue = (CirQueue_t *)pBlindCtrl;

	pxCirQueue->crc32Verify.ulData = Public_CRC_32( pxCirQueue->head.ucData, FLASH_BLIND_AREA_SAVE_START_ADDR - 4 );
	sFLASH_WriteBuffer(pxCirQueue->head.ucData,0,FLASH_BLIND_AREA_SAVE_START_ADDR);
}

/**************************************************************************
//��������Blind_Link1ReportAck
//���ܣ�����1ä������Ӧ����
//���룺��
//�������
//����ֵ����
//��ע���յ�����1��ä������Ӧ��ʱ����ô˺���
***************************************************************************/
void Blind_Link1ReportAck(void)
{
	reportAck = 1;
}

u8 Blind_GetLink1ReportAck(void)
{
	return reportAck;
}

void Blind_ClrLink1ReportAck(void)
{
	reportAck = 0;
	shamLinkCount = 0;
}

//����һ���ź���
int Blind_CreateSemaphore(void)
{
	return Ql_OS_CreateSemaphore("MySemaphore", 1);
}
//�����ź���
void Blind_GiveSemaphore(void)
{
	Ql_OS_GiveSemaphore(iSemaphoreId);
}
//��ȡ�ź���
void Blind_TakeSemaphore(bool wait)
{
	Ql_OS_TakeSemaphore(iSemaphoreId,wait);
}

//ä���ϱ��ص�����
void Blind_Report_Callback_OnTimer(u32 timerId, void* param)
{	
    if(LOGIC_BLIND_REPORT_TMR_ID == timerId)
	{
		if(0 == Blind_GetLink1ReportAck())//ä���ϱ�û��Ӧ
		{
			//resume
			Blind_NodeResume();
			//�������,�����ǲ���Ӧ��˵��������������ô��
			if(GetTerminalAuthorizationFlag())
			{
				APP_DEBUG("Blind_Report_Callback_OnTimer......������...%d\r\n",shamLinkCount);
				if(shamLinkCount++ > 5)
				{				
					shamLinkCount = 0;
					ClearTerminalAuthorizationFlag(CHANNEL_DATA_1);
					//Socket_Restart();//������tcp��ʼ������
					Net_First_Close();
					//Blind_UpdateCirQueueInfo();
					//Report_RtcWakeUpOffsetSave();
					//Ql_Reset(0);
				}
			}
		}
		else
		{			
			Blind_ClrLink1ReportAck();//���ä���ϱ�Ӧ���־
		}
		
		//�������,�����ź���,����ä���ϱ�
		if(GetTerminalAuthorizationFlag())
		{
			if (Q_SUCCESS == Blind_IsCirQueueEmpty(pBlindCtrl))
			{
				//���½ڵ���Ϣ�ѽڵ���Ϣд��Flash
				Blind_UpdateCirQueueInfo();
			}
			Blind_GiveSemaphore();//�ͷ��ź���
		}
		else
		{
			Blind_UpdateCirQueueInfo();
		}
		blindProbe = 0; //�������Ҫ��λ
	}
}
/*
�������������ʲк�����С��
���ʲк�����С�����ӷ�ʱ����ˮ�˼��ơ�
֦�����വ���٣����ĺδ��޷��ݡ�
ǽ����ǧǽ�����ǽ�����ˣ�ǽ�����Ц��
Ц�����������ģ�����ȴ�������ա�
*/
//��ȡä���ϱ�״̬
u8 Blind_GetReportState(void)
{
	return (u8)Blind_IsCirQueueEmpty(pBlindCtrl);	
}


/*********************************************************************************************************
*                                          Blind_CirQueueGenericCreate
*
* Description : ����һ��ѭ������
*
* Argument(s) : uxQueueLength:���е�����
*
* Return(s)   : ����Ķ���ָ��
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
CirQueueHandle_t Blind_CirQueueGenericCreate(const u32 uxQueueLength)
{
	CirQueue_t * pxCirQueue;

	pxCirQueue = (CirQueue_t *)Ql_MEM_Alloc(sizeof(CirQueue_t));
	if( pxCirQueue == NULL )
	{
		return NULL;
	}
	Ql_memset(pxCirQueue->head.ucData,0,FLASH_BLIND_AREA_SAVE_START_ADDR);
	pxCirQueue->startAddr.ulData = FLASH_BLIND_AREA_SAVE_START_ADDR;
	pxCirQueue->queueCapacity.ulData = uxQueueLength;

	return pxCirQueue;
}
/*
�������ɽ����
�����ѻ�֮ľ�����粻ϵ֮�ۡ�
����ƽ����ҵ�����ݻ������ݡ�
*/
//ע��һ����ʱ����������ä���ϱ�
void Blind_Timer_Register(void)
{	
	s32 ret = Ql_Timer_Register(LOGIC_BLIND_REPORT_TMR_ID, Blind_Report_Callback_OnTimer, NULL);
	APP_DEBUG("\r\nBlind_Timer_Register	%d......\r\n",ret);
}
/*********************************************************************************************************
*                                          Blind_Init
*
* Description : ä����ʼ��
*
* Argument(s) : 
*
* Return(s)   : 
*
* Caller(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************/
void Blind_Init(void)
{
	BLIND_SAVE_STU temp;
	u32 crc32Verify;
	s32 ret;
	CirQueue_t * pxCirQueue;
	//����һ���ź���
	iSemaphoreId = Blind_CreateSemaphore();
	//����һ��������
	//iMutexId = Ql_OS_CreateMutex("MyMutex");
	//����ڵ�ռ�
	pBlindCtrl = Blind_CirQueueGenericCreate(FLASH_BLIND_AREA_SAVE_NUM);
	if(NULL == pBlindCtrl)
	{
		APP_DEBUG("blind init failed,System is about to restart......\r\n");	
		Ql_Reset(0);
	}
	APP_DEBUG("blind init success......\r\n");
	pxCirQueue = (CirQueue_t *)pBlindCtrl;
	
	sFLASH_ReadBuffer(temp.head.ucData,0,FLASH_BLIND_AREA_SAVE_START_ADDR);
	crc32Verify = Public_CRC_32( temp.head.ucData, FLASH_BLIND_AREA_SAVE_START_ADDR - 4 );
	//CRCУ��
	if(crc32Verify == temp.crc32Verify.ulData)
	{
		Ql_memcpy(pxCirQueue->head.ucData,temp.head.ucData,FLASH_BLIND_AREA_SAVE_START_ADDR);
		APP_DEBUG("Node check success......%04x    %04x...\r\n",crc32Verify,temp.crc32Verify.ulData);
	}
	else
	{
	//���½ڵ���Ϣ�ѽڵ���Ϣд��Flash
		Blind_UpdateCirQueueInfo();
	}
	//����һ��
	pxCirQueue->queueCapacity.ulData = FLASH_BLIND_AREA_SAVE_NUM;
	//���ͷ��ź���
	Blind_TakeSemaphore(FALSE);
}



