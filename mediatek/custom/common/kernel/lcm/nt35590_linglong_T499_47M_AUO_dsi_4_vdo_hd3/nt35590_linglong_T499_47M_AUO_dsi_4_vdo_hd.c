/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <linux/xlog.h>
#include <mach/mt_pm_ldo.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#define LCM_ID_NT35590 (0x90)
#define GPIO_LCD_RST_EN      (GPIO131)

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

static unsigned int lcm_esd_test = FALSE;	///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	(lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)										(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)					(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg(cmd)										(lcm_util.dsi_dcs_read_lcm_reg(cmd))
#define read_reg_v2(cmd, buffer, buffer_size)   			(lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

#if defined(MTK_WFD_SUPPORT)
#define   LCM_DSI_CMD_MODE							(1)
#else
#define   LCM_DSI_CMD_MODE							(0)
#endif
void nt35590_DCS_write_1A_1P(unsigned char cmd, unsigned char para)
{
	unsigned int data_array[16];

	data_array[0] = (0x00023902);
	data_array[1] = (0x00000000 | (para << 8) | (cmd));
	dsi_set_cmdq(data_array, 2, 1);
}

#define nt35590_DCS_write_1A_0P(cmd)							data_array[0]=(0x00000500 | (cmd<<16)); \
																dsi_set_cmdq(data_array, 1, 1);

static void init_lcm_registers(void)
{
	unsigned int data_array[5];

  nt35590_DCS_write_1A_1P(0xFF,0xEE); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x05,0x59); 
  nt35590_DCS_write_1A_1P(0xFF,0x00);
  nt35590_DCS_write_1A_1P(0xFF,0xEE); 
  nt35590_DCS_write_1A_1P(0x12,0x50); 
  nt35590_DCS_write_1A_1P(0x13,0x02); 
  nt35590_DCS_write_1A_1P(0x6A,0x60); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0xFF,0x00);
  nt35590_DCS_write_1A_1P(0xFF,0x05); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x28,0x01); 
  nt35590_DCS_write_1A_1P(0x2F,0x02);
  nt35590_DCS_write_1A_1P(0xFF,0x00);
  nt35590_DCS_write_1A_1P(0xFF,0x01); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x00,0x4A); 
  nt35590_DCS_write_1A_1P(0x01,0x33); 
  nt35590_DCS_write_1A_1P(0x02,0x53); 
  nt35590_DCS_write_1A_1P(0x03,0x55); 
  nt35590_DCS_write_1A_1P(0x04,0x55); 
  nt35590_DCS_write_1A_1P(0x05,0x33); 
  nt35590_DCS_write_1A_1P(0x06,0x22); 
  nt35590_DCS_write_1A_1P(0x08,0x56); 
  nt35590_DCS_write_1A_1P(0x09,0x8F); 
  nt35590_DCS_write_1A_1P(0x0B,0xCF); 
  nt35590_DCS_write_1A_1P(0x0C,0xCF); 
  nt35590_DCS_write_1A_1P(0x0D,0x2F); 
  nt35590_DCS_write_1A_1P(0x0E,0x29); 
  nt35590_DCS_write_1A_1P(0x11,0x83);
  nt35590_DCS_write_1A_1P(0x12,0x03); 
  nt35590_DCS_write_1A_1P(0x36,0x73); 
  nt35590_DCS_write_1A_1P(0x0F,0x0A); 
  nt35590_DCS_write_1A_1P(0xFF,0xEE); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x12,0x50); 
  nt35590_DCS_write_1A_1P(0x13,0x02); 
  nt35590_DCS_write_1A_1P(0x6A,0x60); 
  nt35590_DCS_write_1A_1P(0xFF,0x05); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x01,0x00);
  nt35590_DCS_write_1A_1P(0x02,0x82); 
  nt35590_DCS_write_1A_1P(0x03,0x82); 
  nt35590_DCS_write_1A_1P(0x04,0x82); 
  nt35590_DCS_write_1A_1P(0x06,0x33); 
  nt35590_DCS_write_1A_1P(0x07,0x01); 
  nt35590_DCS_write_1A_1P(0x08,0x00);
  nt35590_DCS_write_1A_1P(0x09,0x46); 
  nt35590_DCS_write_1A_1P(0x0A,0x46); 
  nt35590_DCS_write_1A_1P(0x0D,0x0B); 
  nt35590_DCS_write_1A_1P(0x0E,0x1D); 
  nt35590_DCS_write_1A_1P(0x0F,0x08); 
  nt35590_DCS_write_1A_1P(0x10,0x53); 
  nt35590_DCS_write_1A_1P(0x11,0x00);
  nt35590_DCS_write_1A_1P(0x12,0x00);
  nt35590_DCS_write_1A_1P(0x14,0x01); 
  nt35590_DCS_write_1A_1P(0x15,0x00);
  nt35590_DCS_write_1A_1P(0x16,0x05); 
  nt35590_DCS_write_1A_1P(0x17,0x00);
  nt35590_DCS_write_1A_1P(0x19,0x7F); 
  nt35590_DCS_write_1A_1P(0x1A,0xFF); 
  nt35590_DCS_write_1A_1P(0x1B,0x0F); 
  nt35590_DCS_write_1A_1P(0x1C,0x00);
  nt35590_DCS_write_1A_1P(0x1D,0x00);
  nt35590_DCS_write_1A_1P(0x1E,0x00);
  nt35590_DCS_write_1A_1P(0x1F,0x07); 
  nt35590_DCS_write_1A_1P(0x20,0x00);
  nt35590_DCS_write_1A_1P(0x21,0x00);
  nt35590_DCS_write_1A_1P(0x22,0x55); 
  nt35590_DCS_write_1A_1P(0x23,0x4D); 
  nt35590_DCS_write_1A_1P(0x2D,0x02); 
  nt35590_DCS_write_1A_1P(0x83,0x01); 
  nt35590_DCS_write_1A_1P(0x9E,0x58); 
  nt35590_DCS_write_1A_1P(0x9F,0x6A); 
  nt35590_DCS_write_1A_1P(0xA0,0x01); 
  nt35590_DCS_write_1A_1P(0xA2,0x10); 
  nt35590_DCS_write_1A_1P(0xBB,0x0A); 
  nt35590_DCS_write_1A_1P(0xBC,0x0A);
  nt35590_DCS_write_1A_1P(0x32,0x08); 
  nt35590_DCS_write_1A_1P(0x33,0xB8); 
  nt35590_DCS_write_1A_1P(0x36,0x01); 
  nt35590_DCS_write_1A_1P(0x37,0x00);
  nt35590_DCS_write_1A_1P(0x43,0x00);
  nt35590_DCS_write_1A_1P(0x4B,0x21); 
  nt35590_DCS_write_1A_1P(0x4C,0x03); 
  nt35590_DCS_write_1A_1P(0x50,0x21); 
  nt35590_DCS_write_1A_1P(0x51,0x03); 
  nt35590_DCS_write_1A_1P(0x58,0x21); 
  nt35590_DCS_write_1A_1P(0x59,0x03); 
  nt35590_DCS_write_1A_1P(0x5D,0x21); 
  nt35590_DCS_write_1A_1P(0x5E,0x03); 
  nt35590_DCS_write_1A_1P(0x6C,0x00);
  nt35590_DCS_write_1A_1P(0x6D,0x00);
  nt35590_DCS_write_1A_1P(0xFF,0x00);
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0xBA,0x02);
  nt35590_DCS_write_1A_1P(0xC2,0x08);

  nt35590_DCS_write_1A_1P(0xFF,0x01); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x75,0x00);
  nt35590_DCS_write_1A_1P(0x76,0x98); 
  nt35590_DCS_write_1A_1P(0x77,0x00);
  nt35590_DCS_write_1A_1P(0x78,0xAF); 
  nt35590_DCS_write_1A_1P(0x79,0x00);
  nt35590_DCS_write_1A_1P(0x7A,0xD1); 
  nt35590_DCS_write_1A_1P(0x7B,0x00);
  nt35590_DCS_write_1A_1P(0x7C,0xE9); 
  nt35590_DCS_write_1A_1P(0x7D,0x00);
  nt35590_DCS_write_1A_1P(0x7E,0xFE); 
  nt35590_DCS_write_1A_1P(0x7F,0x01); 
  nt35590_DCS_write_1A_1P(0x80,0x10); 
  nt35590_DCS_write_1A_1P(0x81,0x01); 
  nt35590_DCS_write_1A_1P(0x82,0x20); 
  nt35590_DCS_write_1A_1P(0x83,0x01); 
  nt35590_DCS_write_1A_1P(0x84,0x2E); 
  nt35590_DCS_write_1A_1P(0x85,0x01); 
  nt35590_DCS_write_1A_1P(0x86,0x3B); 
  nt35590_DCS_write_1A_1P(0x87,0x01); 
  nt35590_DCS_write_1A_1P(0x88,0x65); 
  nt35590_DCS_write_1A_1P(0x89,0x01); 
  nt35590_DCS_write_1A_1P(0x8A,0x88); 
  nt35590_DCS_write_1A_1P(0x8B,0x01); 
  nt35590_DCS_write_1A_1P(0x8C,0xBD); 
  nt35590_DCS_write_1A_1P(0x8D,0x01); 
  nt35590_DCS_write_1A_1P(0x8E,0xE7); 
  nt35590_DCS_write_1A_1P(0x8F,0x02); 
  nt35590_DCS_write_1A_1P(0x90,0x27); 
  nt35590_DCS_write_1A_1P(0x91,0x02); 
  nt35590_DCS_write_1A_1P(0x92,0x59); 
  nt35590_DCS_write_1A_1P(0x93,0x02); 
  nt35590_DCS_write_1A_1P(0x94,0x5B); 
  nt35590_DCS_write_1A_1P(0x95,0x02); 
  nt35590_DCS_write_1A_1P(0x96,0x87); 
  nt35590_DCS_write_1A_1P(0x97,0x02); 
  nt35590_DCS_write_1A_1P(0x98,0xB6); 
  nt35590_DCS_write_1A_1P(0x99,0x02); 
  nt35590_DCS_write_1A_1P(0x9A,0xD5); 
  nt35590_DCS_write_1A_1P(0x9B,0x02); 
  nt35590_DCS_write_1A_1P(0x9C,0xFD); 
  nt35590_DCS_write_1A_1P(0x9D,0x03); 
  nt35590_DCS_write_1A_1P(0x9E,0x19); 
  nt35590_DCS_write_1A_1P(0x9F,0x03); 
  nt35590_DCS_write_1A_1P(0xA0,0x40); 
  nt35590_DCS_write_1A_1P(0xA2,0x03); 
  nt35590_DCS_write_1A_1P(0xA3,0x4C); 
  nt35590_DCS_write_1A_1P(0xA4,0x03); 
  nt35590_DCS_write_1A_1P(0xA5,0x59); 
  nt35590_DCS_write_1A_1P(0xA6,0x03); 
  nt35590_DCS_write_1A_1P(0xA7,0x67); 
  nt35590_DCS_write_1A_1P(0xA9,0x03); 
  nt35590_DCS_write_1A_1P(0xAA,0x78); 
  nt35590_DCS_write_1A_1P(0xAB,0x03); 
  nt35590_DCS_write_1A_1P(0xAC,0x8A); 
  nt35590_DCS_write_1A_1P(0xAD,0x03); 
  nt35590_DCS_write_1A_1P(0xAE,0xA8); 
  nt35590_DCS_write_1A_1P(0xAF,0x03); 
  nt35590_DCS_write_1A_1P(0xB0,0xB8); 
  nt35590_DCS_write_1A_1P(0xB1,0x03); 
  nt35590_DCS_write_1A_1P(0xB2,0xBE); 
  nt35590_DCS_write_1A_1P(0xB3,0x00);
  nt35590_DCS_write_1A_1P(0xB4,0x98); 
  nt35590_DCS_write_1A_1P(0xB5,0x00);
  nt35590_DCS_write_1A_1P(0xB6,0xAF); 
  nt35590_DCS_write_1A_1P(0xB7,0x00);
  nt35590_DCS_write_1A_1P(0xB8,0xD1); 
  nt35590_DCS_write_1A_1P(0xB9,0x00);
  nt35590_DCS_write_1A_1P(0xBA,0xE9); 
  nt35590_DCS_write_1A_1P(0xBB,0x00);
  nt35590_DCS_write_1A_1P(0xBC,0xFE); 
  nt35590_DCS_write_1A_1P(0xBD,0x01); 
  nt35590_DCS_write_1A_1P(0xBE,0x10); 
  nt35590_DCS_write_1A_1P(0xBF,0x01); 
  nt35590_DCS_write_1A_1P(0xC0,0x20); 
  nt35590_DCS_write_1A_1P(0xC1,0x01); 
  nt35590_DCS_write_1A_1P(0xC2,0x2E); 
  nt35590_DCS_write_1A_1P(0xC3,0x01); 
  nt35590_DCS_write_1A_1P(0xC4,0x3B); 
  nt35590_DCS_write_1A_1P(0xC5,0x01); 
  nt35590_DCS_write_1A_1P(0xC6,0x65); 
  nt35590_DCS_write_1A_1P(0xC7,0x01); 
  nt35590_DCS_write_1A_1P(0xC8,0x88); 
  nt35590_DCS_write_1A_1P(0xC9,0x01); 
  nt35590_DCS_write_1A_1P(0xCA,0xBD); 
  nt35590_DCS_write_1A_1P(0xCB,0x01); 
  nt35590_DCS_write_1A_1P(0xCC,0xE7); 
  nt35590_DCS_write_1A_1P(0xCD,0x02); 
  nt35590_DCS_write_1A_1P(0xCE,0x27); 
  nt35590_DCS_write_1A_1P(0xCF,0x02); 
  nt35590_DCS_write_1A_1P(0xD0,0x59); 
  nt35590_DCS_write_1A_1P(0xD1,0x02); 
  nt35590_DCS_write_1A_1P(0xD2,0x5B); 
  nt35590_DCS_write_1A_1P(0xD3,0x02); 
  nt35590_DCS_write_1A_1P(0xD4,0x87); 
  nt35590_DCS_write_1A_1P(0xD5,0x02); 
  nt35590_DCS_write_1A_1P(0xD6,0xB6); 
  nt35590_DCS_write_1A_1P(0xD7,0x02); 
  nt35590_DCS_write_1A_1P(0xD8,0xD5); 
  nt35590_DCS_write_1A_1P(0xD9,0x02); 
  nt35590_DCS_write_1A_1P(0xDA,0xFD); 
  nt35590_DCS_write_1A_1P(0xDB,0x03); 
  nt35590_DCS_write_1A_1P(0xDC,0x19); 
  nt35590_DCS_write_1A_1P(0xDD,0x03); 
  nt35590_DCS_write_1A_1P(0xDE,0x40); 
  nt35590_DCS_write_1A_1P(0xDF,0x03); 
  nt35590_DCS_write_1A_1P(0xE0,0x4C); 
  nt35590_DCS_write_1A_1P(0xE1,0x03); 
  nt35590_DCS_write_1A_1P(0xE2,0x59); 
  nt35590_DCS_write_1A_1P(0xE3,0x03); 
  nt35590_DCS_write_1A_1P(0xE4,0x67); 
  nt35590_DCS_write_1A_1P(0xE5,0x03); 
  nt35590_DCS_write_1A_1P(0xE6,0x78); 
  nt35590_DCS_write_1A_1P(0xE7,0x03); 
  nt35590_DCS_write_1A_1P(0xE8,0x8A); 
  nt35590_DCS_write_1A_1P(0xE9,0x03); 
  nt35590_DCS_write_1A_1P(0xEA,0xA8); 
  nt35590_DCS_write_1A_1P(0xEB,0x03); 
  nt35590_DCS_write_1A_1P(0xEC,0xB8); 
  nt35590_DCS_write_1A_1P(0xED,0x03); 
  nt35590_DCS_write_1A_1P(0xEE,0xBE); 
  nt35590_DCS_write_1A_1P(0xEF,0x00);
  nt35590_DCS_write_1A_1P(0xF0,0x98); 
  nt35590_DCS_write_1A_1P(0xF1,0x00);
  nt35590_DCS_write_1A_1P(0xF2,0xAF); 
  nt35590_DCS_write_1A_1P(0xF3,0x00);
  nt35590_DCS_write_1A_1P(0xF4,0xD1); 
  nt35590_DCS_write_1A_1P(0xF5,0x00);
  nt35590_DCS_write_1A_1P(0xF6,0xE9); 
  nt35590_DCS_write_1A_1P(0xF7,0x00);
  nt35590_DCS_write_1A_1P(0xF8,0xFE); 
  nt35590_DCS_write_1A_1P(0xF9,0x01); 
  nt35590_DCS_write_1A_1P(0xFA,0x10); 
  nt35590_DCS_write_1A_1P(0xFF,0x02); 
  nt35590_DCS_write_1A_1P(0xFB,0x01); 
  nt35590_DCS_write_1A_1P(0x00,0x01); 
  nt35590_DCS_write_1A_1P(0x01,0x20); 
  nt35590_DCS_write_1A_1P(0x02,0x01); 
  nt35590_DCS_write_1A_1P(0x03,0x2E); 
  nt35590_DCS_write_1A_1P(0x04,0x01); 
  nt35590_DCS_write_1A_1P(0x05,0x3B); 
  nt35590_DCS_write_1A_1P(0x06,0x01); 
  nt35590_DCS_write_1A_1P(0x07,0x65); 
  nt35590_DCS_write_1A_1P(0x08,0x01); 
  nt35590_DCS_write_1A_1P(0x09,0x88); 
  nt35590_DCS_write_1A_1P(0x0A,0x01); 
  nt35590_DCS_write_1A_1P(0x0B,0xBD); 
  nt35590_DCS_write_1A_1P(0x0C,0x01); 
  nt35590_DCS_write_1A_1P(0x0D,0xE7); 
  nt35590_DCS_write_1A_1P(0x0E,0x02); 
  nt35590_DCS_write_1A_1P(0x0F,0x27); 
  nt35590_DCS_write_1A_1P(0x10,0x02); 
  nt35590_DCS_write_1A_1P(0x11,0x59); 
  nt35590_DCS_write_1A_1P(0x12,0x02); 
  nt35590_DCS_write_1A_1P(0x13,0x5B); 
  nt35590_DCS_write_1A_1P(0x14,0x02); 
  nt35590_DCS_write_1A_1P(0x15,0x87); 
  nt35590_DCS_write_1A_1P(0x16,0x02); 
  nt35590_DCS_write_1A_1P(0x17,0xB6); 
  nt35590_DCS_write_1A_1P(0x18,0x02); 
  nt35590_DCS_write_1A_1P(0x19,0xD5); 
  nt35590_DCS_write_1A_1P(0x1A,0x02); 
  nt35590_DCS_write_1A_1P(0x1B,0xFD); 
  nt35590_DCS_write_1A_1P(0x1C,0x03); 
  nt35590_DCS_write_1A_1P(0x1D,0x19); 
  nt35590_DCS_write_1A_1P(0x1E,0x03); 
  nt35590_DCS_write_1A_1P(0x1F,0x40); 
  nt35590_DCS_write_1A_1P(0x20,0x03); 
  nt35590_DCS_write_1A_1P(0x21,0x4C); 
  nt35590_DCS_write_1A_1P(0x22,0x03); 
  nt35590_DCS_write_1A_1P(0x23,0x59); 
  nt35590_DCS_write_1A_1P(0x24,0x03); 
  nt35590_DCS_write_1A_1P(0x25,0x67); 
  nt35590_DCS_write_1A_1P(0x26,0x03); 
  nt35590_DCS_write_1A_1P(0x27,0x78); 
  nt35590_DCS_write_1A_1P(0x28,0x03); 
  nt35590_DCS_write_1A_1P(0x29,0x8A); 
  nt35590_DCS_write_1A_1P(0x2A,0x03); 
  nt35590_DCS_write_1A_1P(0x2B,0xA8); 
  nt35590_DCS_write_1A_1P(0x2D,0x03); 
  nt35590_DCS_write_1A_1P(0x2F,0xB8); 
  nt35590_DCS_write_1A_1P(0x30,0x03); 
  nt35590_DCS_write_1A_1P(0x31,0xBE); 
  nt35590_DCS_write_1A_1P(0x32,0x00);
  nt35590_DCS_write_1A_1P(0x33,0x98); 
  nt35590_DCS_write_1A_1P(0x34,0x00);
  nt35590_DCS_write_1A_1P(0x35,0xAF); 
  nt35590_DCS_write_1A_1P(0x36,0x00);
  nt35590_DCS_write_1A_1P(0x37,0xD1); 
  nt35590_DCS_write_1A_1P(0x38,0x00);
  nt35590_DCS_write_1A_1P(0x39,0xE9); 
  nt35590_DCS_write_1A_1P(0x3A,0x00);
  nt35590_DCS_write_1A_1P(0x3B,0xFE); 
  nt35590_DCS_write_1A_1P(0x3D,0x01); 
  nt35590_DCS_write_1A_1P(0x3F,0x10); 
  nt35590_DCS_write_1A_1P(0x40,0x01); 
  nt35590_DCS_write_1A_1P(0x41,0x20); 
  nt35590_DCS_write_1A_1P(0x42,0x01); 
  nt35590_DCS_write_1A_1P(0x43,0x2E); 
  nt35590_DCS_write_1A_1P(0x44,0x01); 
  nt35590_DCS_write_1A_1P(0x45,0x3B); 
  nt35590_DCS_write_1A_1P(0x46,0x01); 
  nt35590_DCS_write_1A_1P(0x47,0x65); 
  nt35590_DCS_write_1A_1P(0x48,0x01); 
  nt35590_DCS_write_1A_1P(0x49,0x88); 
  nt35590_DCS_write_1A_1P(0x4A,0x01); 
  nt35590_DCS_write_1A_1P(0x4B,0xBD); 
  nt35590_DCS_write_1A_1P(0x4C,0x01); 
  nt35590_DCS_write_1A_1P(0x4D,0xE7); 
  nt35590_DCS_write_1A_1P(0x4E,0x02); 
  nt35590_DCS_write_1A_1P(0x4F,0x27); 
  nt35590_DCS_write_1A_1P(0x50,0x02); 
  nt35590_DCS_write_1A_1P(0x51,0x59); 
  nt35590_DCS_write_1A_1P(0x52,0x02); 
  nt35590_DCS_write_1A_1P(0x53,0x5B); 
  nt35590_DCS_write_1A_1P(0x54,0x02); 
  nt35590_DCS_write_1A_1P(0x55,0x87); 
  nt35590_DCS_write_1A_1P(0x56,0x02); 
  nt35590_DCS_write_1A_1P(0x58,0xB6); 
  nt35590_DCS_write_1A_1P(0x59,0x02); 
  nt35590_DCS_write_1A_1P(0x5A,0xD5); 
  nt35590_DCS_write_1A_1P(0x5B,0x02); 
  nt35590_DCS_write_1A_1P(0x5C,0xFD); 
  nt35590_DCS_write_1A_1P(0x5D,0x03); 
  nt35590_DCS_write_1A_1P(0x5E,0x19); 
  nt35590_DCS_write_1A_1P(0x5F,0x03); 
  nt35590_DCS_write_1A_1P(0x60,0x40); 
  nt35590_DCS_write_1A_1P(0x61,0x03); 
  nt35590_DCS_write_1A_1P(0x62,0x4C); 
  nt35590_DCS_write_1A_1P(0x63,0x03); 
  nt35590_DCS_write_1A_1P(0x64,0x59); 
  nt35590_DCS_write_1A_1P(0x65,0x03); 
  nt35590_DCS_write_1A_1P(0x66,0x67); 
  nt35590_DCS_write_1A_1P(0x67,0x03); 
  nt35590_DCS_write_1A_1P(0x68,0x78); 
  nt35590_DCS_write_1A_1P(0x69,0x03); 
  nt35590_DCS_write_1A_1P(0x6A,0x8A); 
  nt35590_DCS_write_1A_1P(0x6B,0x03); 
  nt35590_DCS_write_1A_1P(0x6C,0xA8); 
  nt35590_DCS_write_1A_1P(0x6D,0x03); 
  nt35590_DCS_write_1A_1P(0x6E,0xB8); 
  nt35590_DCS_write_1A_1P(0x6F,0x03); 
  nt35590_DCS_write_1A_1P(0x70,0xBE); 
  nt35590_DCS_write_1A_1P(0x71,0x00);
  nt35590_DCS_write_1A_1P(0x72,0x98); 
  nt35590_DCS_write_1A_1P(0x73,0x00);
  nt35590_DCS_write_1A_1P(0x74,0xAF); 
  nt35590_DCS_write_1A_1P(0x75,0x00);
  nt35590_DCS_write_1A_1P(0x76,0xD1); 
  nt35590_DCS_write_1A_1P(0x77,0x00);
  nt35590_DCS_write_1A_1P(0x78,0xE9); 
  nt35590_DCS_write_1A_1P(0x79,0x00);
  nt35590_DCS_write_1A_1P(0x7A,0xFE); 
  nt35590_DCS_write_1A_1P(0x7B,0x01); 
  nt35590_DCS_write_1A_1P(0x7C,0x10); 
  nt35590_DCS_write_1A_1P(0x7D,0x01); 
  nt35590_DCS_write_1A_1P(0x7E,0x20); 
  nt35590_DCS_write_1A_1P(0x7F,0x01); 
  nt35590_DCS_write_1A_1P(0x80,0x2E); 
  nt35590_DCS_write_1A_1P(0x81,0x01); 
  nt35590_DCS_write_1A_1P(0x82,0x3B); 
  nt35590_DCS_write_1A_1P(0x83,0x01); 
  nt35590_DCS_write_1A_1P(0x84,0x65); 
  nt35590_DCS_write_1A_1P(0x85,0x01); 
  nt35590_DCS_write_1A_1P(0x86,0x88); 
  nt35590_DCS_write_1A_1P(0x87,0x01); 
  nt35590_DCS_write_1A_1P(0x88,0xBD); 
  nt35590_DCS_write_1A_1P(0x89,0x01); 
  nt35590_DCS_write_1A_1P(0x8A,0xE7); 
  nt35590_DCS_write_1A_1P(0x8B,0x02); 
  nt35590_DCS_write_1A_1P(0x8C,0x27); 
  nt35590_DCS_write_1A_1P(0x8D,0x02); 
  nt35590_DCS_write_1A_1P(0x8E,0x59); 
  nt35590_DCS_write_1A_1P(0x8F,0x02); 
  nt35590_DCS_write_1A_1P(0x90,0x5B); 
  nt35590_DCS_write_1A_1P(0x91,0x02); 
  nt35590_DCS_write_1A_1P(0x92,0x87); 
  nt35590_DCS_write_1A_1P(0x93,0x02); 
  nt35590_DCS_write_1A_1P(0x94,0xB6); 
  nt35590_DCS_write_1A_1P(0x95,0x02); 
  nt35590_DCS_write_1A_1P(0x96,0xD5); 
  nt35590_DCS_write_1A_1P(0x97,0x02); 
  nt35590_DCS_write_1A_1P(0x98,0xFD); 
  nt35590_DCS_write_1A_1P(0x99,0x03); 
  nt35590_DCS_write_1A_1P(0x9A,0x19); 
  nt35590_DCS_write_1A_1P(0x9B,0x03); 
  nt35590_DCS_write_1A_1P(0x9C,0x40); 
  nt35590_DCS_write_1A_1P(0x9D,0x03); 
  nt35590_DCS_write_1A_1P(0x9E,0x4C); 
  nt35590_DCS_write_1A_1P(0x9F,0x03); 
  nt35590_DCS_write_1A_1P(0xA0,0x59); 
  nt35590_DCS_write_1A_1P(0xA2,0x03); 
  nt35590_DCS_write_1A_1P(0xA3,0x67); 
  nt35590_DCS_write_1A_1P(0xA4,0x03); 
  nt35590_DCS_write_1A_1P(0xA5,0x78); 
  nt35590_DCS_write_1A_1P(0xA6,0x03); 
  nt35590_DCS_write_1A_1P(0xA7,0x8A); 
  nt35590_DCS_write_1A_1P(0xA9,0x03); 
  nt35590_DCS_write_1A_1P(0xAA,0xA8); 
  nt35590_DCS_write_1A_1P(0xAB,0x03); 
  nt35590_DCS_write_1A_1P(0xAC,0xB8); 
  nt35590_DCS_write_1A_1P(0xAD,0x03); 
  nt35590_DCS_write_1A_1P(0xAE,0xBE); 
  nt35590_DCS_write_1A_1P(0xAF,0x00);
  nt35590_DCS_write_1A_1P(0xB0,0x98); 
  nt35590_DCS_write_1A_1P(0xB1,0x00);
  nt35590_DCS_write_1A_1P(0xB2,0xAF); 
  nt35590_DCS_write_1A_1P(0xB3,0x00);
  nt35590_DCS_write_1A_1P(0xB4,0xD1); 
  nt35590_DCS_write_1A_1P(0xB5,0x00);
  nt35590_DCS_write_1A_1P(0xB6,0xE9); 
  nt35590_DCS_write_1A_1P(0xB7,0x00);
  nt35590_DCS_write_1A_1P(0xB8,0xFE); 
  nt35590_DCS_write_1A_1P(0xB9,0x01); 
  nt35590_DCS_write_1A_1P(0xBA,0x10); 
  nt35590_DCS_write_1A_1P(0xBB,0x01); 
  nt35590_DCS_write_1A_1P(0xBC,0x20); 
  nt35590_DCS_write_1A_1P(0xBD,0x01); 
  nt35590_DCS_write_1A_1P(0xBE,0x2E); 
  nt35590_DCS_write_1A_1P(0xBF,0x01); 
  nt35590_DCS_write_1A_1P(0xC0,0x3B); 
  nt35590_DCS_write_1A_1P(0xC1,0x01); 
  nt35590_DCS_write_1A_1P(0xC2,0x65); 
  nt35590_DCS_write_1A_1P(0xC3,0x01); 
  nt35590_DCS_write_1A_1P(0xC4,0x88); 
  nt35590_DCS_write_1A_1P(0xC5,0x01); 
  nt35590_DCS_write_1A_1P(0xC6,0xBD); 
  nt35590_DCS_write_1A_1P(0xC7,0x01); 
  nt35590_DCS_write_1A_1P(0xC8,0xE7); 
  nt35590_DCS_write_1A_1P(0xC9,0x02); 
  nt35590_DCS_write_1A_1P(0xCA,0x27); 
  nt35590_DCS_write_1A_1P(0xCB,0x02); 
  nt35590_DCS_write_1A_1P(0xCC,0x59); 
  nt35590_DCS_write_1A_1P(0xCD,0x02); 
  nt35590_DCS_write_1A_1P(0xCE,0x5B); 
  nt35590_DCS_write_1A_1P(0xCF,0x02); 
  nt35590_DCS_write_1A_1P(0xD0,0x87); 
  nt35590_DCS_write_1A_1P(0xD1,0x02); 
  nt35590_DCS_write_1A_1P(0xD2,0xB6); 
  nt35590_DCS_write_1A_1P(0xD3,0x02); 
  nt35590_DCS_write_1A_1P(0xD4,0xD5); 
  nt35590_DCS_write_1A_1P(0xD5,0x02); 
  nt35590_DCS_write_1A_1P(0xD6,0xFD); 
  nt35590_DCS_write_1A_1P(0xD7,0x03); 
  nt35590_DCS_write_1A_1P(0xD8,0x19); 
  nt35590_DCS_write_1A_1P(0xD9,0x03); 
  nt35590_DCS_write_1A_1P(0xDA,0x40); 
  nt35590_DCS_write_1A_1P(0xDB,0x03); 
  nt35590_DCS_write_1A_1P(0xDC,0x4C); 
  nt35590_DCS_write_1A_1P(0xDD,0x03); 
  nt35590_DCS_write_1A_1P(0xDE,0x59); 
  nt35590_DCS_write_1A_1P(0xDF,0x03); 
  nt35590_DCS_write_1A_1P(0xE0,0x67); 
  nt35590_DCS_write_1A_1P(0xE1,0x03); 
  nt35590_DCS_write_1A_1P(0xE2,0x78); 
  nt35590_DCS_write_1A_1P(0xE3,0x03); 
  nt35590_DCS_write_1A_1P(0xE4,0x8A); 
  nt35590_DCS_write_1A_1P(0xE5,0x03); 
  nt35590_DCS_write_1A_1P(0xE6,0xA8); 
  nt35590_DCS_write_1A_1P(0xE7,0x03); 
  nt35590_DCS_write_1A_1P(0xE8,0xB8); 
  nt35590_DCS_write_1A_1P(0xE9,0x03); 
  nt35590_DCS_write_1A_1P(0xEA,0xBE);

////////////
  
	nt35590_DCS_write_1A_1P(0xFF, 0x00);

  nt35590_DCS_write_1A_1P(0x35,0x00); 


	nt35590_DCS_write_1A_0P(0x11);
	MDELAY(200);
	nt35590_DCS_write_1A_0P(0x29);
	MDELAY(10);
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));

		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;
                params->dbi.te_mode                             = LCM_DBI_TE_MODE_DISABLED;
                params->dbi.te_edge_polarity            = LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;
        #endif

	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM = LCM_THREE_LANE; //LCM_THREE_LANE; //LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

		// Video mode setting
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 2;// 3    2
		params->dsi.vertical_backporch					= 20;// 20   1
		params->dsi.vertical_frontporch					= 6; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 6;// 50  2
		params->dsi.horizontal_backporch				= 12;
		params->dsi.horizontal_frontporch				= 80;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8;

		// Bit rate calculation
		//1 Every lane speed
		params->dsi.pll_div1=1;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		params->dsi.pll_div2=1;		// div2=0,1,2,3;div1_real=1,2,4,4	
		params->dsi.fbk_div =45;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	

}

static void lcm_init(void)
{
	unsigned int data_array[16];
#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_3000, "LCM");
#endif

	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(150);
	init_lcm_registers();
}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	nt35590_DCS_write_1A_0P(0x28);
	MDELAY(100);
	nt35590_DCS_write_1A_0P(0x10);
	MDELAY(100);
#ifdef BUILD_LK
	upmu_set_rg_vgp6_en(0);
#else
	hwPowerDown(MT65XX_POWER_LDO_VGP6, "LCM");
#endif

}


static void lcm_resume(void)
{
#ifndef BUILD_LK
	lcm_init();
#endif
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
		       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

	data_array[0] = 0x00053902;
	data_array[1] =
	    (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] =
	    (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[2];
	unsigned int array[16];
#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#else
	hwPowerOn(MT65XX_POWER_LDO_VGP6, VOL_2800, "LCM");
#endif
	mt_set_gpio_mode(GPIO_LCD_RST_EN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_RST_EN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
	MDELAY(10);
	mt_set_gpio_out(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
	MDELAY(100);


	array[0] = 0x00023700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0];		//we only need ID
#ifdef BUILD_LK
	printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__,
	       id);
#else
	printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n",
	       __func__, id);
#endif

	if (id == LCM_ID_NT35590)
		return 1;
	else
		return 0;
}

//

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER nt35590_linglong_T499_47M_AUO_dsi_4_vdo_hd_lcm_drv = {
	.name = "nt35590_linglong_T499_47M_AUO_dsi_4_vdo_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
