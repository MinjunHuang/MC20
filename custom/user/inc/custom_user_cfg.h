/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   custom_user_cfg.h
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   The file intends for USER initialization definition. 
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifndef __CUSTOM_USER_CFG_H__
#define __CUSTOM_USER_CFG_H__

//��Ʒ�ͺ�
#define	PRODUCT_MODEL_VTKG_22A ( 0 )
#define	PRODUCT_MODEL_VTKG_22G ( 1 )

#define	PRODUCT_MODEL PRODUCT_MODEL_VTKG_22G

#define	configUSE_MTK_NMEA		0

#define	configUSE_GNSS			0	//GPS���ܿ���  0-->�ر� 1-->��

extern u8 ProjectInitFinshFlag;	


#endif //__CUSTOM_USER_CFG_H__
