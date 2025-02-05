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

#define GPIO_LCD_RST_EN      (GPIO131)
#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#define REGFLAG_DELAY                                       0xFE
#define REGFLAG_END_OF_TABLE                                0x100 
static unsigned int lcm_esd_test = FALSE;	///only for ESD test

#define LCM_ID_NT35590 (0x90)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE							1

static struct LCM_setting_table
{
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] =
{
  {0xFF,1,{0xEE}}, 
  {0xFB,1,{0x01}}, 
  {0x05,1,{0x59}}, 
  {0xFF,1,{0x00}},
  {0xFF,1,{0xEE}}, 
  {0x12,1,{0x50}}, 
  {0x13,1,{0x02}}, 
  {0x6A,1,{0x60}}, 
  {0xFB,1,{0x01}}, 
  {0xFF,1,{0x00}},
  {0xFF,1,{0x05}}, 
  {0xFB,1,{0x01}}, 
  {0x28,1,{0x01}}, 
  {0x2F,1,{0x02}}, 
  {0xFF,1,{0x00}},
  {0xFF,1,{0x01}}, 
  {0xFB,1,{0x01}}, 
  {0x00,1,{0x4A}}, 
  {0x01,1,{0x33}}, 
  {0x02,1,{0x53}}, 
  {0x03,1,{0x55}}, 
  {0x04,1,{0x55}}, 
  {0x05,1,{0x33}}, 
  {0x06,1,{0x22}}, 
  {0x08,1,{0x56}}, 
  {0x09,1,{0x8F}}, 
  {0x0B,1,{0xCF}}, 
  {0x0C,1,{0xCF}}, 
  {0x0D,1,{0x2F}}, 
  {0x0E,1,{0x29}}, 
  {0x11,1,{0x83}}, 
  {0x12,1,{0x03}}, 
  {0x36,1,{0x73}}, 
  {0x0F,1,{0x0A}}, 
  {0xFF,1,{0xEE}}, 
  {0xFB,1,{0x01}}, 
  {0x12,1,{0x50}}, 
  {0x13,1,{0x02}}, 
  {0x6A,1,{0x60}}, 
  {0xFF,1,{0x05}}, 
  {0xFB,1,{0x01}}, 
  {0x01,1,{0x00}},
  {0x02,1,{0x82}}, 
  {0x03,1,{0x82}}, 
  {0x04,1,{0x82}}, 
  {0x06,1,{0x33}}, 
  {0x07,1,{0x01}}, 
  {0x08,1,{0x00}},
  {0x09,1,{0x46}}, 
  {0x0A,1,{0x46}}, 
  {0x0D,1,{0x0B}}, 
  {0x0E,1,{0x1D}}, 
  {0x0F,1,{0x08}}, 
  {0x10,1,{0x53}}, 
  {0x11,1,{0x00}},
  {0x12,1,{0x00}},
  {0x14,1,{0x01}}, 
  {0x15,1,{0x00}},
  {0x16,1,{0x05}}, 
  {0x17,1,{0x00}},
  {0x19,1,{0x7F}}, 
  {0x1A,1,{0xFF}}, 
  {0x1B,1,{0x0F}}, 
  {0x1C,1,{0x00}},
  {0x1D,1,{0x00}},
  {0x1E,1,{0x00}},
  {0x1F,1,{0x07}}, 
  {0x20,1,{0x00}},
  {0x21,1,{0x00}},
  {0x22,1,{0x55}}, 
  {0x23,1,{0x4D}}, 
  {0x2D,1,{0x02}}, 
  {0x83,1,{0x01}}, 
  {0x9E,1,{0x58}}, 
  {0x9F,1,{0x6A}}, 
  {0xA0,1,{0x01}}, 
  {0xA2,1,{0x10}}, 
  {0xBB,1,{0x0A}}, 
  {0xBC,1,{0x0A}}, 
  {0x32,1,{0x08}}, 
  {0x33,1,{0xB8}}, 
  {0x36,1,{0x01}}, 
  {0x37,1,{0x00}},
  {0x43,1,{0x00}},
  {0x4B,1,{0x21}}, 
  {0x4C,1,{0x03}}, 
  {0x50,1,{0x21}}, 
  {0x51,1,{0x03}}, 
  {0x58,1,{0x21}}, 
  {0x59,1,{0x03}}, 
  {0x5D,1,{0x21}}, 
  {0x5E,1,{0x03}}, 
  {0x6C,1,{0x00}},
  {0x6D,1,{0x00}},
  {0xFF,1,{0x00}},
  {0xFB,1,{0x01}}, 
  {0xBA,1,{0x02}}, 
  {0xC2,1,{0x08}}, 
  {0xFF,1,{0x01}}, 
  {0xFB,1,{0x01}}, 
  {0x75,1,{0x00}},
  {0x76,1,{0x98}}, 
  {0x77,1,{0x00}},
  {0x78,1,{0xAF}}, 
  {0x79,1,{0x00}},
  {0x7A,1,{0xD1}}, 
  {0x7B,1,{0x00}},
  {0x7C,1,{0xE9}}, 
  {0x7D,1,{0x00}},
  {0x7E,1,{0xFE}}, 
  {0x7F,1,{0x01}}, 
  {0x80,1,{0x10}}, 
  {0x81,1,{0x01}}, 
  {0x82,1,{0x20}}, 
  {0x83,1,{0x01}}, 
  {0x84,1,{0x2E}}, 
  {0x85,1,{0x01}}, 
  {0x86,1,{0x3B}}, 
  {0x87,1,{0x01}}, 
  {0x88,1,{0x65}}, 
  {0x89,1,{0x01}}, 
  {0x8A,1,{0x88}}, 
  {0x8B,1,{0x01}}, 
  {0x8C,1,{0xBD}}, 
  {0x8D,1,{0x01}}, 
  {0x8E,1,{0xE7}}, 
  {0x8F,1,{0x02}}, 
  {0x90,1,{0x27}}, 
  {0x91,1,{0x02}}, 
  {0x92,1,{0x59}}, 
  {0x93,1,{0x02}}, 
  {0x94,1,{0x5B}}, 
  {0x95,1,{0x02}}, 
  {0x96,1,{0x87}}, 
  {0x97,1,{0x02}}, 
  {0x98,1,{0xB6}}, 
  {0x99,1,{0x02}}, 
  {0x9A,1,{0xD5}}, 
  {0x9B,1,{0x02}}, 
  {0x9C,1,{0xFD}}, 
  {0x9D,1,{0x03}}, 
  {0x9E,1,{0x19}}, 
  {0x9F,1,{0x03}}, 
  {0xA0,1,{0x40}}, 
  {0xA2,1,{0x03}}, 
  {0xA3,1,{0x4C}}, 
  {0xA4,1,{0x03}}, 
  {0xA5,1,{0x59}}, 
  {0xA6,1,{0x03}}, 
  {0xA7,1,{0x67}}, 
  {0xA9,1,{0x03}}, 
  {0xAA,1,{0x78}}, 
  {0xAB,1,{0x03}}, 
  {0xAC,1,{0x8A}}, 
  {0xAD,1,{0x03}}, 
  {0xAE,1,{0xA8}}, 
  {0xAF,1,{0x03}}, 
  {0xB0,1,{0xB8}}, 
  {0xB1,1,{0x03}}, 
  {0xB2,1,{0xBE}}, 
  {0xB3,1,{0x00}},
  {0xB4,1,{0x98}}, 
  {0xB5,1,{0x00}},
  {0xB6,1,{0xAF}}, 
  {0xB7,1,{0x00}},
  {0xB8,1,{0xD1}}, 
  {0xB9,1,{0x00}},
  {0xBA,1,{0xE9}}, 
  {0xBB,1,{0x00}},
  {0xBC,1,{0xFE}}, 
  {0xBD,1,{0x01}}, 
  {0xBE,1,{0x10}}, 
  {0xBF,1,{0x01}}, 
  {0xC0,1,{0x20}}, 
  {0xC1,1,{0x01}}, 
  {0xC2,1,{0x2E}}, 
  {0xC3,1,{0x01}}, 
  {0xC4,1,{0x3B}}, 
  {0xC5,1,{0x01}}, 
  {0xC6,1,{0x65}}, 
  {0xC7,1,{0x01}}, 
  {0xC8,1,{0x88}}, 
  {0xC9,1,{0x01}}, 
  {0xCA,1,{0xBD}}, 
  {0xCB,1,{0x01}}, 
  {0xCC,1,{0xE7}}, 
  {0xCD,1,{0x02}}, 
  {0xCE,1,{0x27}}, 
  {0xCF,1,{0x02}}, 
  {0xD0,1,{0x59}}, 
  {0xD1,1,{0x02}}, 
  {0xD2,1,{0x5B}}, 
  {0xD3,1,{0x02}}, 
  {0xD4,1,{0x87}}, 
  {0xD5,1,{0x02}}, 
  {0xD6,1,{0xB6}}, 
  {0xD7,1,{0x02}}, 
  {0xD8,1,{0xD5}}, 
  {0xD9,1,{0x02}}, 
  {0xDA,1,{0xFD}}, 
  {0xDB,1,{0x03}}, 
  {0xDC,1,{0x19}}, 
  {0xDD,1,{0x03}}, 
  {0xDE,1,{0x40}}, 
  {0xDF,1,{0x03}}, 
  {0xE0,1,{0x4C}}, 
  {0xE1,1,{0x03}}, 
  {0xE2,1,{0x59}}, 
  {0xE3,1,{0x03}}, 
  {0xE4,1,{0x67}}, 
  {0xE5,1,{0x03}}, 
  {0xE6,1,{0x78}}, 
  {0xE7,1,{0x03}}, 
  {0xE8,1,{0x8A}}, 
  {0xE9,1,{0x03}}, 
  {0xEA,1,{0xA8}}, 
  {0xEB,1,{0x03}}, 
  {0xEC,1,{0xB8}}, 
  {0xED,1,{0x03}}, 
  {0xEE,1,{0xBE}}, 
  {0xEF,1,{0x00}},
  {0xF0,1,{0x98}}, 
  {0xF1,1,{0x00}},
  {0xF2,1,{0xAF}}, 
  {0xF3,1,{0x00}},
  {0xF4,1,{0xD1}}, 
  {0xF5,1,{0x00}},
  {0xF6,1,{0xE9}}, 
  {0xF7,1,{0x00}},
  {0xF8,1,{0xFE}}, 
  {0xF9,1,{0x01}}, 
  {0xFA,1,{0x10}}, 
  {0xFF,1,{0x02}}, 
  {0xFB,1,{0x01}}, 
  {0x00,1,{0x01}}, 
  {0x01,1,{0x20}}, 
  {0x02,1,{0x01}}, 
  {0x03,1,{0x2E}}, 
  {0x04,1,{0x01}}, 
  {0x05,1,{0x3B}}, 
  {0x06,1,{0x01}}, 
  {0x07,1,{0x65}}, 
  {0x08,1,{0x01}}, 
  {0x09,1,{0x88}}, 
  {0x0A,1,{0x01}}, 
  {0x0B,1,{0xBD}}, 
  {0x0C,1,{0x01}}, 
  {0x0D,1,{0xE7}}, 
  {0x0E,1,{0x02}}, 
  {0x0F,1,{0x27}}, 
  {0x10,1,{0x02}}, 
  {0x11,1,{0x59}}, 
  {0x12,1,{0x02}}, 
  {0x13,1,{0x5B}}, 
  {0x14,1,{0x02}}, 
  {0x15,1,{0x87}}, 
  {0x16,1,{0x02}}, 
  {0x17,1,{0xB6}}, 
  {0x18,1,{0x02}}, 
  {0x19,1,{0xD5}}, 
  {0x1A,1,{0x02}}, 
  {0x1B,1,{0xFD}}, 
  {0x1C,1,{0x03}}, 
  {0x1D,1,{0x19}}, 
  {0x1E,1,{0x03}}, 
  {0x1F,1,{0x40}}, 
  {0x20,1,{0x03}}, 
  {0x21,1,{0x4C}}, 
  {0x22,1,{0x03}}, 
  {0x23,1,{0x59}}, 
  {0x24,1,{0x03}}, 
  {0x25,1,{0x67}}, 
  {0x26,1,{0x03}}, 
  {0x27,1,{0x78}}, 
  {0x28,1,{0x03}}, 
  {0x29,1,{0x8A}}, 
  {0x2A,1,{0x03}}, 
  {0x2B,1,{0xA8}}, 
  {0x2D,1,{0x03}}, 
  {0x2F,1,{0xB8}}, 
  {0x30,1,{0x03}}, 
  {0x31,1,{0xBE}}, 
  {0x32,1,{0x00}},
  {0x33,1,{0x98}}, 
  {0x34,1,{0x00}},
  {0x35,1,{0xAF}}, 
  {0x36,1,{0x00}},
  {0x37,1,{0xD1}}, 
  {0x38,1,{0x00}},
  {0x39,1,{0xE9}}, 
  {0x3A,1,{0x00}},
  {0x3B,1,{0xFE}}, 
  {0x3D,1,{0x01}}, 
  {0x3F,1,{0x10}}, 
  {0x40,1,{0x01}}, 
  {0x41,1,{0x20}}, 
  {0x42,1,{0x01}}, 
  {0x43,1,{0x2E}}, 
  {0x44,1,{0x01}}, 
  {0x45,1,{0x3B}}, 
  {0x46,1,{0x01}}, 
  {0x47,1,{0x65}}, 
  {0x48,1,{0x01}}, 
  {0x49,1,{0x88}}, 
  {0x4A,1,{0x01}}, 
  {0x4B,1,{0xBD}}, 
  {0x4C,1,{0x01}}, 
  {0x4D,1,{0xE7}}, 
  {0x4E,1,{0x02}}, 
  {0x4F,1,{0x27}}, 
  {0x50,1,{0x02}}, 
  {0x51,1,{0x59}}, 
  {0x52,1,{0x02}}, 
  {0x53,1,{0x5B}}, 
  {0x54,1,{0x02}}, 
  {0x55,1,{0x87}}, 
  {0x56,1,{0x02}}, 
  {0x58,1,{0xB6}}, 
  {0x59,1,{0x02}}, 
  {0x5A,1,{0xD5}}, 
  {0x5B,1,{0x02}}, 
  {0x5C,1,{0xFD}}, 
  {0x5D,1,{0x03}}, 
  {0x5E,1,{0x19}}, 
  {0x5F,1,{0x03}}, 
  {0x60,1,{0x40}}, 
  {0x61,1,{0x03}}, 
  {0x62,1,{0x4C}}, 
  {0x63,1,{0x03}}, 
  {0x64,1,{0x59}}, 
  {0x65,1,{0x03}}, 
  {0x66,1,{0x67}}, 
  {0x67,1,{0x03}}, 
  {0x68,1,{0x78}}, 
  {0x69,1,{0x03}}, 
  {0x6A,1,{0x8A}}, 
  {0x6B,1,{0x03}}, 
  {0x6C,1,{0xA8}}, 
  {0x6D,1,{0x03}}, 
  {0x6E,1,{0xB8}}, 
  {0x6F,1,{0x03}}, 
  {0x70,1,{0xBE}}, 
  {0x71,1,{0x00}},
  {0x72,1,{0x98}}, 
  {0x73,1,{0x00}},
  {0x74,1,{0xAF}}, 
  {0x75,1,{0x00}},
  {0x76,1,{0xD1}}, 
  {0x77,1,{0x00}},
  {0x78,1,{0xE9}}, 
  {0x79,1,{0x00}},
  {0x7A,1,{0xFE}}, 
  {0x7B,1,{0x01}}, 
  {0x7C,1,{0x10}}, 
  {0x7D,1,{0x01}}, 
  {0x7E,1,{0x20}}, 
  {0x7F,1,{0x01}}, 
  {0x80,1,{0x2E}}, 
  {0x81,1,{0x01}}, 
  {0x82,1,{0x3B}}, 
  {0x83,1,{0x01}}, 
  {0x84,1,{0x65}}, 
  {0x85,1,{0x01}}, 
  {0x86,1,{0x88}}, 
  {0x87,1,{0x01}}, 
  {0x88,1,{0xBD}}, 
  {0x89,1,{0x01}}, 
  {0x8A,1,{0xE7}}, 
  {0x8B,1,{0x02}}, 
  {0x8C,1,{0x27}}, 
  {0x8D,1,{0x02}}, 
  {0x8E,1,{0x59}}, 
  {0x8F,1,{0x02}}, 
  {0x90,1,{0x5B}}, 
  {0x91,1,{0x02}}, 
  {0x92,1,{0x87}}, 
  {0x93,1,{0x02}}, 
  {0x94,1,{0xB6}}, 
  {0x95,1,{0x02}}, 
  {0x96,1,{0xD5}}, 
  {0x97,1,{0x02}}, 
  {0x98,1,{0xFD}}, 
  {0x99,1,{0x03}}, 
  {0x9A,1,{0x19}}, 
  {0x9B,1,{0x03}}, 
  {0x9C,1,{0x40}}, 
  {0x9D,1,{0x03}}, 
  {0x9E,1,{0x4C}}, 
  {0x9F,1,{0x03}}, 
  {0xA0,1,{0x59}}, 
  {0xA2,1,{0x03}}, 
  {0xA3,1,{0x67}}, 
  {0xA4,1,{0x03}}, 
  {0xA5,1,{0x78}}, 
  {0xA6,1,{0x03}}, 
  {0xA7,1,{0x8A}}, 
  {0xA9,1,{0x03}}, 
  {0xAA,1,{0xA8}}, 
  {0xAB,1,{0x03}}, 
  {0xAC,1,{0xB8}}, 
  {0xAD,1,{0x03}}, 
  {0xAE,1,{0xBE}}, 
  {0xAF,1,{0x00}},
  {0xB0,1,{0x98}}, 
  {0xB1,1,{0x00}},
  {0xB2,1,{0xAF}}, 
  {0xB3,1,{0x00}},
  {0xB4,1,{0xD1}}, 
  {0xB5,1,{0x00}},
  {0xB6,1,{0xE9}}, 
  {0xB7,1,{0x00}},
  {0xB8,1,{0xFE}}, 
  {0xB9,1,{0x01}}, 
  {0xBA,1,{0x10}}, 
  {0xBB,1,{0x01}}, 
  {0xBC,1,{0x20}}, 
  {0xBD,1,{0x01}}, 
  {0xBE,1,{0x2E}}, 
  {0xBF,1,{0x01}}, 
  {0xC0,1,{0x3B}}, 
  {0xC1,1,{0x01}}, 
  {0xC2,1,{0x65}}, 
  {0xC3,1,{0x01}}, 
  {0xC4,1,{0x88}}, 
  {0xC5,1,{0x01}}, 
  {0xC6,1,{0xBD}}, 
  {0xC7,1,{0x01}}, 
  {0xC8,1,{0xE7}}, 
  {0xC9,1,{0x02}}, 
  {0xCA,1,{0x27}}, 
  {0xCB,1,{0x02}}, 
  {0xCC,1,{0x59}}, 
  {0xCD,1,{0x02}}, 
  {0xCE,1,{0x5B}}, 
  {0xCF,1,{0x02}}, 
  {0xD0,1,{0x87}}, 
  {0xD1,1,{0x02}}, 
  {0xD2,1,{0xB6}}, 
  {0xD3,1,{0x02}}, 
  {0xD4,1,{0xD5}}, 
  {0xD5,1,{0x02}}, 
  {0xD6,1,{0xFD}}, 
  {0xD7,1,{0x03}}, 
  {0xD8,1,{0x19}}, 
  {0xD9,1,{0x03}}, 
  {0xDA,1,{0x40}}, 
  {0xDB,1,{0x03}}, 
  {0xDC,1,{0x4C}}, 
  {0xDD,1,{0x03}}, 
  {0xDE,1,{0x59}}, 
  {0xDF,1,{0x03}}, 
  {0xE0,1,{0x67}}, 
  {0xE1,1,{0x03}}, 
  {0xE2,1,{0x78}}, 
  {0xE3,1,{0x03}}, 
  {0xE4,1,{0x8A}}, 
  {0xE5,1,{0x03}}, 
  {0xE6,1,{0xA8}}, 
  {0xE7,1,{0x03}}, 
  {0xE8,1,{0xB8}}, 
  {0xE9,1,{0x03}}, 
  {0xEA,1,{0xBE}},
//
	{0xFF,1,{0x00}},
	{0x35,1,{0x00}},
//
	{0x11,1,{0x00}},
	
	{REGFLAG_DELAY, 120, {}},                            

	{0x29,1,{0x00}},            		
	{REGFLAG_DELAY, 40, {}},
	
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
		
    {0x28, 0, {0x00}},
    {REGFLAG_DELAY, 10, {}},

	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},
	
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {

        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd)
        {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
            case REGFLAG_END_OF_TABLE :
                break;
            default:				
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{

  memset(params, 0, sizeof(LCM_PARAMS));
//stock ThL.W200.156.140402.JBV2.HD.EN.COM.8P64_MT6589T	
  params->width = 720;
  params->height = 1280;
  params->dsi.packet_size = 256;
  params->dsi.vertical_active_line = 1280;
  params->dsi.horizontal_active_pixel = 720;
  params->type = 2;
  params->dbi.te_mode = 0;
  params->dsi.mode = 0;
  params->dsi.LANE_NUM = 3;
  params->dsi.data_format.color_order = 0;
  params->dsi.data_format.trans_seq = 0;
  params->dsi.data_format.padding = 0;
  params->dsi.data_format.format = 2;
  params->dsi.intermediat_buffer_num = 0;
  params->dsi.PS = 2;
  params->dsi.word_count = 2160;
  params->dsi.vertical_sync_active = 3;
  params->dsi.vertical_backporch = 8;
  params->dsi.vertical_frontporch = 8;
  params->dsi.horizontal_sync_active = 11;
  params->dsi.horizontal_backporch = 64;
  params->dsi.horizontal_frontporch = 64;
  params->dsi.pll_div1 = 1;
  params->dsi.pll_div2 = 1;
  params->dsi.fbk_div = 47;
  params->dsi.lcm_int_te_monitor = 0;
  params->dsi.lcm_int_te_period = 1;
  params->dsi.lcm_ext_te_monitor = 0;
  params->dsi.noncont_clock = 0;
  params->dsi.noncont_clock_period = 2;
//
}

static void lcm_init(void)
{
  unsigned int data_array[16];
  
#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#endif

  MDELAY(60);
  SET_RESET_PIN(0);
  MDELAY(20);
  SET_RESET_PIN(1);
  MDELAY(1);
  
  data_array[0] = 0xEEFF1500;
  dsi_set_cmdq(data_array, 1, 1);
  data_array[0] = 0x08261500;
  dsi_set_cmdq(data_array, 1, 1);
  MDELAY(1);
  data_array[0] = 0x00261500;
  dsi_set_cmdq(data_array, 1, 1);
  data_array[0] = 0x00FF1500;
  dsi_set_cmdq(data_array, 1, 1);
  
  MDELAY(10);
  SET_RESET_PIN(0);
  MDELAY(1);
  SET_RESET_PIN(1);
  MDELAY(20);

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
  push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
  
  SET_RESET_PIN(0);
  MDELAY(10);
  SET_RESET_PIN(1);
  MDELAY(10);

#ifdef BUILD_LK
	upmu_set_rg_vgp6_en(0);
#endif
}


static void lcm_resume(void)
{
  unsigned int data_array[16];

  SET_RESET_PIN(0);
  MDELAY(20);
  SET_RESET_PIN(1);
  MDELAY(1);
  
  data_array[0] = 0xEEFF1500;
  dsi_set_cmdq(data_array, 1, 1);
  data_array[0] = 0x08261500;
  dsi_set_cmdq(data_array, 1, 1);
  
  MDELAY(1);
  
  data_array[0] = 0x00261500;
  dsi_set_cmdq(data_array, 1, 1);
  data_array[0] = 0x00FF1500;
  dsi_set_cmdq(data_array, 1, 1);
  
  MDELAY(10);
  SET_RESET_PIN(0);
  MDELAY(1);
  SET_RESET_PIN(1);
  MDELAY(20);

  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];
#if 1
	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
#endif
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);
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
#endif

  mt_set_gpio_mode(131, 0);
  mt_set_gpio_dir(131, 1);
  mt_set_gpio_out(131, 1);
  
  MDELAY(60);
  SET_RESET_PIN(1);
  MDELAY(50);
  SET_RESET_PIN(0);
  MDELAY(50);
  SET_RESET_PIN(1);
  MDELAY(120);
  
  array[0] = 0x23700;
  dsi_set_cmdq(array, 1, 1);  
  read_reg_v2(0xF4, buffer, 2);
  
#ifdef BUILD_LK
	printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
#else		   
  printk("wqcat ykl id0_num = 0x%x,id1_num = 0x%x", buffer, __func__, id);
#endif
  
  return 1;
}


static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x36, buffer, 1);
	if(buffer[0]==0x90)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
 #endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}
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
