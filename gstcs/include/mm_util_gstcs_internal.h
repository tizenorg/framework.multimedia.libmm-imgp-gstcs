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

#ifndef __MM_UTIL_GSTCS_INTERNAL_H__
#define __MM_UTIL_GSTCS_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <glib.h>
#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>
#include "mm_util_gstcs.h"
#include "mm_log.h"
#define GSTCS_FREE(src) { if(src) {g_free(src); src = NULL;} }
typedef struct _image_format_s
{
	char *format_label; /*I420, AYUV, RGB888, BGRA8888 */
	char *colorspace; /* yuv, rgb, RGBA */
	int width;
	int height;
	int stride;
	int elevation;
	int blocksize;
	GstCaps* caps;
} image_format_s;

typedef struct _gstreamer_s
{
	GMainLoop *loop;
	GstElement *pipeline;
	GstElement *appsrc;
	GstElement *colorspace;
	GstElement *videoscale;
	GstElement *videoflip;
	GstElement *appsink;
	GstBuffer *output_buffer;
} gstreamer_s;

/**
 *
 * @remark     image size
 *
 * @param      input_format_label                                                                       [in]           "filename.yuv" or  "filename,rgb" etc
 * @param      input_width, input_height, output_width, output_height   [in]            int value
 * @return     This function returns image size
*/
static int
mm_setup_image_size(const char* image_format_label, int width, int height);


#ifdef __cplusplus
}
#endif

#endif	/*__MM_UTIL_GSTCS_INTERNAL_H__*/


