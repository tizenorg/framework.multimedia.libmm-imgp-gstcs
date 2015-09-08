/*
 * libmm-imgp-gstcs
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: YoungHun Kim <yh8004.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef __MM_UTIL_GSTCS_H__
#define __MM_UTIL_GSTCS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGE_FORMAT_LABEL_BUFFER_SIZE 9

typedef enum
{
	MM_UTIL_ROTATE_0,               /**< Rotation 0 degree - no effect */
	MM_UTIL_ROTATE_90,              /**< Rotation 90 degree */
	MM_UTIL_ROTATE_180,             /**< Rotation 180 degree */
	MM_UTIL_ROTATE_270,             /**< Rotation 270 degree */
	MM_UTIL_ROTATE_FLIP_HORZ,       /**< Flip horizontal */
	MM_UTIL_ROTATE_FLIP_VERT,       /**< Flip vertial */
	MM_UTIL_ROTATE_NUM              /**< Number of rotation types */
} mm_util_img_rotate_type_e;

/* Enumerations */
typedef enum
{
	IMGP_CSC = 0,
	IMGP_RSZ,
	IMGP_ROT,
} imgp_type_e;

typedef enum
{
	/* YUV planar format */
	MM_UTIL_IMG_FMT_YUV420 = 0x00,  /**< YUV420 format - planer */
	MM_UTIL_IMG_FMT_YUV422,         /**< YUV422 format - planer */
	MM_UTIL_IMG_FMT_I420,           /**< YUV420 format - planar */
	MM_UTIL_IMG_FMT_NV12,           /**< NV12 format - planer */

	/* YUV packed format */
	MM_UTIL_IMG_FMT_UYVY,           /**< UYVY format - YUV packed format */
	MM_UTIL_IMG_FMT_YUYV,           /**< YUYV format - YUV packed format */

	/* RGB color */
	MM_UTIL_IMG_FMT_RGB565,         /**< RGB565 pixel format */
	MM_UTIL_IMG_FMT_RGB888,         /**< RGB888 pixel format */
	MM_UTIL_IMG_FMT_ARGB8888,       /**< ARGB8888 pixel format */

	//added by yh8004.kim
	MM_UTIL_IMG_FMT_BGRA8888,      /**< BGRA8888 pixel format */
	MM_UTIL_IMG_FMT_RGBA8888,      /**< RGBA8888 pixel format */
	MM_UTIL_IMG_FMT_BGRX8888,      /**<BGRX8888 pixel format */
	/* non-standard format */
	MM_UTIL_IMG_FMT_NV12_TILED,     /**< Customized color format in s5pc110 */
	MM_UTIL_IMG_FMT_NUM,            /**< Number of image formats */
} mm_util_img_format_e;

/**
 * Image Process Info for dlopen
 */
typedef struct _imgp_info_s
{
	char *input_format_label;
	mm_util_img_format_e src_format;
	unsigned int src_width;
	unsigned int src_height;
	char *output_format_label;
	mm_util_img_format_e dst_format;
	unsigned int dst_width;
	unsigned int dst_height;
	unsigned int output_stride;
	unsigned int output_elevation;
	mm_util_img_rotate_type_e angle;
} imgp_info_s;

/**
 *
 * @remark 	colorspace converter  		I420, nv12 etc <-> RGB888 or ARGB8888 etc
 *
 * @remark	resize 					if input_width != output_width or input_height != output_height
 *
 * @remark 	rotate 					flip the image
 * @param	 _imgp_type_e file 									 [in]		convert / resize / rotate
 * @param	input_ file 										 [in]		"filename.yuv" or  "filename,rgb" etc
 * @param	input_format_lable, output_format_lable 				 [in]		 I420 or rgb888 etc
 * @param	input_width, input_height, output_width, output_height	 [in]		 int value
 * @param	output_stride, output_elevation  						 [in]		 output_width value or output_height  + padding (using  round_up function )

 * @return  	This function returns gstremer image processor result value
 *		if the resule is -1, then do not execute when the colopsapce converter is not supported
 *		else if  the resule is 0, then you can use output_image pointer(char** value)
*/

int
mm_imgp(imgp_info_s* pImgp_info, unsigned char *src, unsigned char *dst, imgp_type_e _imgp_type_e);

#ifdef __cplusplus__
};
#endif

#endif	/*__MM_UTIL_GSTCS_H__*/
