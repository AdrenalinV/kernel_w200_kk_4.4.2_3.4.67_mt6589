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

#define FRAME_WIDTH  			(720)
#define FRAME_HEIGHT 			(1280)
#define LCM_OTM1283_ID			(0x1283)

#define REGFLAG_DELAY          		(0xFE)
#define REGFLAG_END_OF_TABLE      	(0xFB)	// END OF REGISTERS MARKER

#ifndef FALSE
#define FALSE 0
#endif
#define GPIO_LCD_RST_EN      (GPIO131)

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    		(lcm_util.set_reset_pin((v)))
#define UDELAY(n) 					(lcm_util.udelay(n))
#define MDELAY(n) 					(lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)        (lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update))
#define dsi_set_cmdq(pdata, queue_size, force_update)		(lcm_util.dsi_set_cmdq(pdata, queue_size, force_update))
#define wrtie_cmd(cmd)						(lcm_util.dsi_write_cmd(cmd))
#define write_regs(addr, pdata, byte_nums)			(lcm_util.dsi_write_regs(addr, pdata, byte_nums))
#define read_reg						(lcm_util.dsi_read_reg())
#define read_reg_v2(cmd, buffer, buffer_size)   		(lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size))

static struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00,1,{0x00}},
	{0xff,3,{0x12,0x83,0x01}},	//EXTC=1
	{0x00,1,{0x80}},	        //Orise mode enable
	{0xff,2,{0x12,0x83}},
	{0x00,1,{0x80}},             //TCON Setting
	{0xc0,9,{0x00,0x64,0x00,0x10,0x10,0x00,0x64,0x10,0x10}},
	{0x00,1,{0x90}},             //Panel Timing Setting
	{0xc0,6,{0x00,0x5c,0x00,0x01,0x00,0x04}},
	{0x00,1,{0xa2}},
	{0xC0,3,{0x01,0x00,0x00}},
	{0x00,1,{0xb3}},             //Interval Scan Frame: 0 frame, column inversion
	{0xc0,2,{0x00,0x50}},
	{0x00,1,{0x81}},             //frame rate:60Hz
	{0xc1,1,{0x55}},
	{0x00,1,{0x90}},             //clock delay for data latch 
	{0xc4,1,{0x49}},
	{0x00,1,{0xa0}},             //dcdc setting
	{0xc4,14,{0x05,0x10,0x04,0x02,0x05,0x15,0x11,0x05,0x10,0x07,0x02,0x05,0x15,0x11}},
	{0x00,1,{0xb0}},             //clamp voltage setting
	{0xc4,2,{0x00,0x00}},
	{0x00,1,{0x91}},             //VGH=13V, VGL=-12V, pump ratio:VGH=6x, VGL=-5x
	{0xc5,2,{0x29,0x50}},
	{0x00,1,{0x00}},             //GVDD=4.204V, NGVDD=-4.204V 
	{0xd8,2,{0x8A,0x8A}},             
	{0x00,1,{0x00}},             //VCOM=0.240V
	{0xd9,1,{0x50}},             
	{0x00,1,{0x81}},             //source bias 0.75uA
	{0xc4,1,{0x82}},
	{0x00,1,{0xb0}},             //VDD_18V=1.6V, LVDSVDD=1.55V
	{0xc5,2,{0x04,0xb8}},
	{0x00,1,{0xbb}},             //LVD voltage level setting
	{0xc5,1,{0x80}},
	{0x00,1,{0x82}},		//chopper
	{0xC4,1,{0x02}},
	{0x00,1,{0xc6}},		//debounce
	{0xb0,1,{0x03}},
	{0x00,1,{0x00}},             //ID1
	{0xd0,1,{0x40}},
	{0x00,1,{0x00}},             //ID2, ID3
	{0xd1,2,{0x00,0x00}},
	{0x00,1,{0x80}},             //source blanking frame = black, defacult='30'
	{0xc4,1,{0x00}},
	{0x00,1,{0x98}},             //vcom discharge=gnd:'10', '00'=disable
	{0xc5,1,{0x10}},
	{0x00,1,{0x81}},
	{0xf5,1,{0x15}},  // ibias off
	{0x00,1,{0x83}}, 
	{0xf5,1,{0x15}},  // lvd off
	{0x00,1,{0x85}},
	{0xf5,1,{0x15}},  // gvdd off
	{0x00,1,{0x87}}, 
	{0xf5,1,{0x15}},  // lvdsvdd off
	{0x00,1,{0x89}},
	{0xf5,1,{0x15}},  // nvdd_18 off
	{0x00,1,{0x8b}}, 
	{0xf5,1,{0x15}},  // en_vcom off
	{0x00,1,{0x95}},
	{0xf5,1,{0x15}},  // pump3 off
	{0x00,1,{0x97}}, 
	{0xf5,1,{0x15}},  // pump4 off
	{0x00,1,{0x99}},
	{0xf5,1,{0x15}},  // pump5 off
	{0x00,1,{0xa1}}, 
	{0xf5,1,{0x15}},  // gamma off
	{0x00,1,{0xa3}},
	{0xf5,1,{0x15}},  // sd ibias off
	{0x00,1,{0xa5}}, 
	{0xf5,1,{0x15}},  // sdpch off
	{0x00,1,{0xa7}}, 
	{0xf5,1,{0x15}},  // sdpch bias off
	{0x00,1,{0xab}},
	{0xf5,1,{0x18}},  // ddc osc off
	{0x00,1,{0x94}},  //VCL on  	
	{0xF5,1,{0x02}},
	{0x00,1,{0xBA}},  //VSP on   	
	{0xF5,1,{0x03}},
	{0x00,1,{0xb1}},             //VGLO, VGHO setting
	{0xf5,13,{0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x15,0x08,0x15}},
	{0x00,1,{0xb4}},             //VGLO1/2 Pull low setting
	{0xc5,1,{0xc0}},		//d[7] vglo1 d[6] vglo2 => 0: pull vss, 1: pull vgl
	{0x00,1,{0x80}},             //panel timing state control
	{0xcb,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,1,{0x90}},             //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x00}},
	{0x00,1,{0xa0}},             //panel timing state control
	{0xcb,15,{0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0x00,1,{0xb0}},             //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00}},
	{0x00,1,{0xc0}},             //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x00,0x05,0x05,0x05,0x05,0x05}},
	{0x00,1,{0xd0}},             //panel timing state control
	{0xcb,15,{0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05}},
	{0x00,1,{0xe0}},             //panel timing state control
	{0xcb,14,{0x05,0x00,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x00,0x00}},
	{0x00,1,{0xf0}},             //panel timing state control
	{0xcb,11,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
	{0x00,1,{0x80}},             //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x07,0x00,0x0d,0x09,0x0f,0x0b,0x11}},
	{0x00,1,{0x90}},             //panel pad mapping control
	{0xcc,15,{0x15,0x13,0x17,0x01,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06}},
	{0x00,1,{0xa0}},             //panel pad mapping control
	{0xcc,14,{0x08,0x00,0x0e,0x0a,0x10,0x0c,0x12,0x16,0x14,0x18,0x02,0x04,0x00,0x00}},
	{0x00,1,{0xb0}},             //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x02,0x00,0x0c,0x10,0x0a,0x0e,0x14}},
	{0x00,1,{0xc0}},             //panel pad mapping control
	{0xcc,15,{0x18,0x12,0x16,0x08,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03}},
	{0x00,1,{0xd0}},             //panel pad mapping control
	{0xcc,14,{0x01,0x00,0x0b,0x0f,0x09,0x0d,0x13,0x17,0x11,0x15,0x07,0x05,0x00,0x00}},
	{0x00,1,{0x80}},             //panel VST setting
	{0xce,12,{0x87,0x03,0x28,0x86,0x03,0x28,0x85,0x03,0x28,0x84,0x03,0x28}},
	{0x00,1,{0x90}},             //panel VEND setting
	{0xce,14,{0x34,0xfc,0x28,0x34,0xfd,0x28,0x34,0xfe,0x28,0x34,0xff,0x28,0x00,0x00}},
	{0x00,1,{0xa0}},             //panel CLKA1/2 setting
	{0xce,14,{0x38,0x07,0x05,0x00,0x00,0x28,0x00,0x38,0x06,0x05,0x01,0x00,0x28,0x00}},
	{0x00,1,{0xb0}},             //panel CLKA3/4 setting
	{0xce,14,{0x38,0x05,0x05,0x02,0x00,0x28,0x00,0x38,0x04,0x05,0x03,0x00,0x28,0x00}},
	{0x00,1,{0xc0}},             //panel CLKb1/2 setting
	{0xce,14,{0x38,0x03,0x05,0x04,0x00,0x28,0x00,0x38,0x02,0x05,0x05,0x00,0x28,0x00}},
	{0x00,1,{0xd0}},             //panel CLKb3/4 setting
	{0xce,14,{0x38,0x01,0x05,0x06,0x00,0x28,0x00,0x38,0x00,0x05,0x07,0x00,0x28,0x00}},
	{0x00,1,{0x80}},             //panel CLKc1/2 setting
	{0xcf,14,{0x38,0x07,0x05,0x00,0x00,0x18,0x25,0x38,0x06,0x05,0x01,0x00,0x18,0x25}},
	{0x00,1,{0x90}},             //panel CLKc3/4 setting
	{0xcf,14,{0x38,0x05,0x05,0x02,0x00,0x18,0x25,0x38,0x04,0x05,0x03,0x00,0x18,0x25}},
	{0x00,1,{0xa0}},             //panel CLKd1/2 setting
	{0xcf,14,{0x38,0x03,0x05,0x04,0x00,0x18,0x25,0x38,0x02,0x05,0x05,0x00,0x18,0x25}},
	{0x00,1,{0xb0}},             //panel CLKd3/4 setting
	{0xcf,14,{0x38,0x01,0x05,0x06,0x00,0x18,0x25,0x38,0x00,0x05,0x07,0x00,0x18,0x25}},
	{0x00,1,{0xc0}},             //panel ECLK setting
	{0xcf,11,{0x01,0x01,0x20,0x20,0x00,0x00,0x01,0x81,0x00,0x03,0x08}}, //gate pre. ena.
    {0x00,1,{0x00}}, 
	{0xE1,16,{0x00,0x0d,0x14,0x0E,0x06,0x11,0x0b,0x0b,0x04,0x07,0x0B,0x09,0x10,0x13,0x0F,0x06}}, 
	{0x00,1,{0x00}}, 
	{0xE2,16,{0x00,0x0d,0x13,0x0E,0x06,0x11,0x0C,0x0b,0x04,0x07,0x0A,0x09,0x10,0x13,0x0F,0x06}}, 
    {0x00,1,{0x00}},             //Orise mode disable
    {0xff,3,{0xff,0xff,0xff}},
    {0x35,1,{0x00}},//TE ON                                                                                                 
                                                                                                                        
    {0x11,0,{0x00}},//  
    {REGFLAG_DELAY,120,{}},                                                                                                 
                                   				                                                                                
	{0x29,0,{0x00}}, //Display on (}},
    {REGFLAG_DELAY,20,{}},  
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 60, {}},
	// Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 200, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count,
		       unsigned char force_update)
{
	unsigned int i;
	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;
		switch (cmd) {
		
		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V2(cmd, table[i].count,
					table[i].para_list, force_update);
		}
	}
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS * util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS * params)
{
  memset(params, 0, sizeof(LCM_PARAMS));
//stock ThL.W200.156.140402.JBV2.HD.EN.COM.8P64_MT6589T	
  params->type = 2;
  params->dsi.mode = 2;
  params->dsi.LANE_NUM = 4;
  params->dsi.data_format.color_order = 0;
  params->dsi.data_format.trans_seq = 0;
  params->dsi.data_format.padding = 0;
  params->dsi.data_format.format = 2;
  params->dsi.intermediat_buffer_num = 0;
  params->dsi.PS = 2;
  params->dsi.word_count = 2160;
  params->dsi.vertical_sync_active = 2;
  params->dsi.vertical_backporch = 14;
  params->dsi.horizontal_sync_active = 2;
  params->width = 720;
  params->dsi.horizontal_backporch = 34;
  params->height = 1280;
  params->dsi.horizontal_frontporch = 24;
  params->dsi.packet_size = 256;
  params->dsi.vertical_frontporch = 16;
  params->dsi.vertical_active_line = 1280;
  params->dsi.horizontal_active_pixel = 720;
  params->dsi.pll_div1 = 1;
  params->dsi.pll_div2 = 1;
  params->dsi.fbk_div = 29;
//
}

static void lcm_init(void)
{
	unsigned int data_array[16];

#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#endif

  mt_set_gpio_mode(131, 0);
  mt_set_gpio_dir(131, 1);
  mt_set_gpio_out(131, 1);
  
  SET_RESET_PIN(1);
  SET_RESET_PIN(0);
  MDELAY(50);
  SET_RESET_PIN(1);
  MDELAY(100);
  
  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);  
}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

  SET_RESET_PIN(0);
  MDELAY(10);
  SET_RESET_PIN(1);
  
  push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
  
#ifdef BUILD_LK
	upmu_set_rg_vgp6_en(0);
#endif
}

static void lcm_resume(void)
{
#ifndef BUILD_LK
	lcm_init();
#endif
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id0, id1, id2, id3, id4;
	unsigned char buffer[5];
	unsigned int array[5];
	
#ifdef BUILD_LK
	upmu_set_rg_vgp6_vosel(6);
	upmu_set_rg_vgp6_en(1);
#endif

  mt_set_gpio_mode(131, 0);
  mt_set_gpio_dir(131, 1);
  mt_set_gpio_out(131, 1);
  
  SET_RESET_PIN(1);
  SET_RESET_PIN(0);
  MDELAY(50);
  SET_RESET_PIN(1);
  MDELAY(100);
  
  array[0] = 0x00083700;
  dsi_set_cmdq(array, 1, 1);
  read_reg_v2(0xA1, buffer, 5);
  
	id0 = buffer[0];
	id1 = buffer[1];
	id2 = buffer[2];
	id3 = buffer[3];
	id4 = buffer[4];

#if defined(BUILD_LK)
	printf("%s, Module ID = {%x, %x, %x, %x, %x} \n", __func__, id0,
	       id1, id2, id3, id4);
#else
	printk("%s, Module ID = {%x, %x, %x, %x,%x} \n", __func__, id0,
	       id1, id2, id3, id4);
#endif

	return (LCM_OTM1283_ID == ((id2 << 8) | id3)) ? 1 : 0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm1283a_ykl_LP050APKP159A_auo_dsi_vdo_hd_lcm_drv = {
	.name = "otm1283a_ykl_LP050APKP159A_auo_dsi_vdo_hd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.compare_id = lcm_compare_id,
};
