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
#include "mm_util_gstcs_internal.h"
#include <mm_debug.h>
#include <gst/check/gstcheck.h>
#include <mm_error.h>
#define MM_UTIL_ROUND_UP_2(num) (((num)+1)&~1)
#define MM_UTIL_ROUND_UP_4(num) (((num)+3)&~3)
#define MM_UTIL_ROUND_UP_8(num) (((num)+7)&~7)
#define MM_UTIL_ROUND_UP_16(num) (((num)+15)&~15)
/*########################################################################################*/
#define setup_image_size_I420(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width) * MM_UTIL_ROUND_UP_2 (height) + MM_UTIL_ROUND_UP_8 (width) * MM_UTIL_ROUND_UP_2 (height) /2); \
	return size; \
}

#define setup_image_size_Y42B(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width) * height + MM_UTIL_ROUND_UP_8 (width) * height); \
	return size; \
}

#define setup_image_size_Y444(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width) * height * 3); \
	return size; \
}

#define setup_image_size_UYVY(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_2 (width) * 2 * height); \
	return size; \
}

#define setup_image_size_YUYV(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_2 (width) * 2 * height); \
	return size; \
}

#define setup_image_size_YV12(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width) * MM_UTIL_ROUND_UP_2 (height)+ MM_UTIL_ROUND_UP_8 (width) * MM_UTIL_ROUND_UP_2 (height) / 2); \
	return size; \
}

#define setup_image_size_NV12(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width) * MM_UTIL_ROUND_UP_2 (height) *1.5); \
	return size; \
}

#define setup_image_size_RGB565(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width * 2) *height); \
	return size; \
}

#define setup_image_size_RGB888(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width*3) * height); \
	return size; \
}

#define setup_image_size_BGR888(width, height) { \
	int size=0; \
	size = (MM_UTIL_ROUND_UP_4 (width*3) * height); \
	return size; \
}
/*########################################################################################*/

static void
_mm_sink_buffer (GstElement * appsink, gpointer user_data)
{
	GstBuffer *_buf=NULL;
	gstreamer_s * pGstreamer_s = (gstreamer_s*) user_data;
	_buf = gst_app_sink_pull_buffer((GstAppSink*)appsink);

	pGstreamer_s->output_buffer = _buf;

	if(pGstreamer_s->output_buffer != NULL) {
		debug_log("Create Output Buffer: GST_BUFFER_DATA: %p\t GST_BUFFER_SIZE: %d\n", GST_BUFFER_DATA(pGstreamer_s->output_buffer), GST_BUFFER_SIZE (pGstreamer_s->output_buffer));
	}else {
		debug_error("ERROR -Input Prepare Buffer! Check createoutput buffer function");
	}
	gst_buffer_unref (_buf); /* we don't need the appsink buffer anymore */
	gst_buffer_ref (pGstreamer_s->output_buffer); /* when you want to avoid flushing */
}

static void
_mm_sink_preroll (GstElement * appsink, gpointer user_data)
{
	GstBuffer *_buf=NULL;
	gstreamer_s * pGstreamer_s = (gstreamer_s*) user_data;
	_buf = gst_app_sink_pull_preroll((GstAppSink*)appsink);

	pGstreamer_s->output_buffer = _buf;
	/* pGstreamer_s->output_image_format_s->caps = GST_BUFFER_CAPS(_buf); */
	if(pGstreamer_s->output_buffer != NULL) {
		debug_log("Create Output Buffer: GST_BUFFER_DATA: %p\t GST_BUFFER_SIZE: %d\n",
			GST_BUFFER_DATA(pGstreamer_s->output_buffer), GST_BUFFER_SIZE (pGstreamer_s->output_buffer));
	}else {
		debug_error("ERROR -Input Prepare Buffer! Check createoutput buffer function");
	}

	gst_buffer_unref (_buf); /* we don't need the appsink buffer anymore */
	gst_buffer_ref (pGstreamer_s->output_buffer); /* when you want to avoid flushings */
}

static gboolean
_mm_on_sink_message (GstBus * bus, GstMessage * message, gstreamer_s * pGstreamer_s)
{
	switch (GST_MESSAGE_TYPE (message)) {
		case GST_MESSAGE_EOS:
			debug_log("Finished playback\n"); /* g_main_loop_quit (pGstreamer_s->loop); */
			break;
		case GST_MESSAGE_ERROR:
			debug_error("Received error\n"); /* g_main_loop_quit (pGstreamer_s->loop); */
			break;
		case GST_MESSAGE_STATE_CHANGED:
			debug_log("[%s] %s(%d) \n", GST_MESSAGE_SRC_NAME(message), GST_MESSAGE_TYPE_NAME(message), GST_MESSAGE_TYPE(message));
			break;
		case GST_MESSAGE_STREAM_STATUS:
			debug_log("[%s] %s(%d) \n", GST_MESSAGE_SRC_NAME(message), GST_MESSAGE_TYPE_NAME(message), GST_MESSAGE_TYPE(message));
			break;
		default:
			debug_log("[%s] %s(%d) \n", GST_MESSAGE_SRC_NAME(message), GST_MESSAGE_TYPE_NAME(message), GST_MESSAGE_TYPE(message));
			break;
	}
	return TRUE;
}


static int
_mm_create_pipeline( gstreamer_s* pGstreamer_s)
{
	int ret = MM_ERROR_NONE;
	pGstreamer_s->pipeline= gst_pipeline_new ("ffmpegcolorsapce");
	pGstreamer_s->appsrc= gst_element_factory_make("appsrc","appsrc");
	pGstreamer_s->colorspace=gst_element_factory_make("ffmpegcolorspace","colorconverter");

	pGstreamer_s->videoscale=gst_element_factory_make("videoscale", "scale");
	pGstreamer_s->videoflip=gst_element_factory_make( "videoflip", "flip" );

	pGstreamer_s->appsink=gst_element_factory_make("appsink","appsink");

	if (!pGstreamer_s->pipeline || !pGstreamer_s->appsrc||!pGstreamer_s->colorspace || !pGstreamer_s->appsink) {
		debug_error("One element could not be created. Exiting.\n");
		ret = MM_ERROR_IMAGE_INVALID_VALUE;
	}
	return ret;
}

static gboolean
_mm_check_resize_format(int src_width, int src_height, int dst_width, int dst_height)
{
	gboolean _bool=FALSE;

	if( (src_width != dst_width) || (src_height != dst_height) ) {
		_bool = TRUE;
	}

	return _bool;
}

static gboolean
_mm_check_rotate_format(int GstVideoFlipMethod)
{
	gboolean _bool=FALSE;

	if((GstVideoFlipMethod >= 1) && ( GstVideoFlipMethod<= 7)) {
		_bool = TRUE;
	}

	return _bool;
}

static gboolean
_mm_check_resize_format_label(char* __format_label)
{
	gboolean _bool = FALSE;

	if(strcmp(__format_label, "AYUV") == 0
		|| strcmp(__format_label, "UYVY") == 0 ||strcmp(__format_label, "Y800") == 0 || strcmp(__format_label, "I420") == 0 || strcmp(__format_label, "YV12") == 0
		|| strcmp(__format_label, "RGB888") == 0 || strcmp(__format_label, "RGB565") == 0 || strcmp(__format_label, "BGR888") == 0 || strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 ||strcmp(__format_label, "ABGR8888") == 0 ||strcmp(__format_label, "RGBX") == 0
		||strcmp(__format_label, "XRGB") == 0 ||strcmp(__format_label, "BGRX") == 0 ||strcmp(__format_label, "XBGR") == 0 ||strcmp(__format_label, "Y444") == 0
		||strcmp(__format_label, "Y42B") == 0 ||strcmp(__format_label, "YUY2") == 0 ||strcmp(__format_label, "YUYV") == 0 ||strcmp(__format_label, "UYVY") == 0
		||strcmp(__format_label, "Y41B") == 0 ||strcmp(__format_label, "Y16") == 0 ||strcmp(__format_label, "Y800") == 0 ||strcmp(__format_label, "Y8") == 0
		||strcmp(__format_label, "GREY") == 0 ||strcmp(__format_label, "AY64") == 0 || strcmp(__format_label, "YUV422") == 0) {

		_bool=TRUE;
	}

	return _bool;
}

static gboolean
_mm_check_rotate_format_label(const char* __format_label)
{
	gboolean _bool = FALSE;

	if(strcmp(__format_label, "I420") == 0 ||strcmp(__format_label, "YV12") == 0 || strcmp(__format_label, "IYUV")
		|| strcmp(__format_label, "RGB888") ||strcmp(__format_label, "BGR888") == 0 ||strcmp(__format_label, "RGBA8888") == 0
		|| strcmp(__format_label, "ARGB8888") == 0 ||strcmp(__format_label, "BGRA8888") == 0 ||strcmp(__format_label, "ABGR8888") == 0 ) {
		_bool=TRUE;
	}

	return _bool;
}

static void
_mm_link_pipeline_order_csc_rsz(gstreamer_s* pGstreamer_s, image_format_s* input_format, image_format_s* output_format)
{
	if(_mm_check_resize_format(input_format->width,input_format->height, output_format->width, output_format->height)) 	{
		debug_log("check_for_resize");
		gst_bin_add_many(GST_BIN(pGstreamer_s->pipeline), pGstreamer_s->appsrc, pGstreamer_s->videoscale, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL);
		debug_log("gst_bin_add_many");
		if(_mm_check_resize_format_label( input_format->format_label)) {
			debug_log(" input_format->format_label: %s", input_format->format_label);
			if(!gst_element_link_many(pGstreamer_s->appsrc, pGstreamer_s->videoscale, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL)) {
				debug_error("Fail to link b/w ffmpeg and appsink except rot\n");
			}
		}else if(_mm_check_resize_format_label(output_format->format_label)) {
			debug_log("output_format->format_label: %s", output_format->format_label);
			if(!gst_element_link_many(pGstreamer_s->appsrc, pGstreamer_s->colorspace, pGstreamer_s->videoscale, pGstreamer_s->appsink, NULL)) {
				debug_error("Fail to link b/w ffmpeg and appsink except rot\n");
			}
		}
	}else {
		debug_log("check_for_convert");
		gst_bin_add_many(GST_BIN(pGstreamer_s->pipeline), pGstreamer_s->appsrc, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL);
		debug_log("gst_bin_add_many");
		if(!gst_element_link_many(pGstreamer_s->appsrc, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL)) {
			debug_error("Fail to link b/w ffmpeg and appsink except rsz & rot\n");
		}
	}
}

static void
_mm_link_pipeline_order_csc_rsz_rot(gstreamer_s* pGstreamer_s, image_format_s* input_format, image_format_s* output_format)
{
	if(_mm_check_rotate_format_label(input_format->format_label)) {
		gst_bin_add_many(GST_BIN(pGstreamer_s->pipeline), pGstreamer_s->appsrc, pGstreamer_s->videoscale, pGstreamer_s->videoflip, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL);
		if(!gst_element_link_many(pGstreamer_s->appsrc, pGstreamer_s->videoscale, pGstreamer_s->videoflip, pGstreamer_s->colorspace, pGstreamer_s->appsink, NULL)) {
			debug_error("Fail to link b/w appsrc and ffmpeg in rotate\n");
		}
	}else if(_mm_check_rotate_format_label(output_format->format_label)) {
		gst_bin_add_many(GST_BIN(pGstreamer_s->pipeline), pGstreamer_s->appsrc, pGstreamer_s->colorspace, pGstreamer_s->videoscale, pGstreamer_s->videoflip,pGstreamer_s->appsink, NULL);
		if(!gst_element_link_many(pGstreamer_s->appsrc, pGstreamer_s->colorspace, pGstreamer_s->videoscale, pGstreamer_s->videoflip, pGstreamer_s->appsink, NULL)) {
			debug_error("Fail to link b/w ffmpeg and appsink in rotate\n");
		}
	}
}


static void
_mm_link_pipeline( gstreamer_s* pGstreamer_s, image_format_s* input_format, image_format_s* output_format, int _valuepGstreamer_sVideoFlipMethod)
{
	/* set property */
	gst_app_src_set_caps(GST_APP_SRC(pGstreamer_s->appsrc), input_format->caps); /* g_object_set(pGstreamer_s->appsrc, "caps", input_format->caps, NULL); you can use appsrc'cap property */
	g_object_set(pGstreamer_s->appsrc, "num-buffers", 1, NULL);
	g_object_set(pGstreamer_s->appsrc, "is-live", TRUE, NULL); /* add because of gstreamer_s time issue */
	g_object_set (pGstreamer_s->appsrc, "format", GST_FORMAT_TIME, NULL);
	g_object_set(pGstreamer_s->appsrc, "stream-type", 0 /*stream*/, NULL);

	g_object_set(pGstreamer_s->videoflip, "method", _valuepGstreamer_sVideoFlipMethod, NULL ); /* GST_VIDEO_FLIP_METHOD_IDENTITY (0): none- Identity (no rotation) (1): clockwise - Rotate clockwise 90 degrees (2): rotate-180 - Rotate 180 degrees (3): counterclockwise - Rotate counter-clockwise 90 degrees (4): horizontal-flip - Flip horizontally (5): vertical-flip - Flip vertically (6): upper-left-diagonal - Flip across upper left/lower right diagonal (7): upper-right-diagonal - Flip across upper right/lower left diagonal */

	gst_app_sink_set_caps(GST_APP_SINK(pGstreamer_s->appsink), output_format->caps); /*g_object_set(pGstreamer_s->appsink, "caps", output_format->caps, NULL); */
	g_object_set(pGstreamer_s->appsink, "drop", TRUE, NULL);
	g_object_set(pGstreamer_s->appsink, "emit-signals", TRUE, "sync", FALSE, NULL);

	if(_mm_check_rotate_format(_valuepGstreamer_sVideoFlipMethod)) { /* when you want to rotate image */
		/* because IYUV, I420, YV12 format can use vidoeflip*/
		debug_log("set_link_pipeline_order_csc_rsz_rot");
		_mm_link_pipeline_order_csc_rsz_rot(pGstreamer_s, input_format, output_format);
	}else {
		debug_log("set_link_pipeline_order_csc_rsz");
		_mm_link_pipeline_order_csc_rsz(pGstreamer_s, input_format, output_format);
	}
	/* Conecting to the new-buffer signal emited by the appsink*/
	debug_log("Start G_CALLBACK (mm_sink_buffer)");
	g_signal_connect (pGstreamer_s->appsink, "new-buffer", G_CALLBACK (_mm_sink_buffer), pGstreamer_s);
	debug_log("End G_CALLBACK (mm_sink_buffer)");
}



static void
_mm_set_image_format_s_capabilities(image_format_s* __format) /*_format_label: I420 _colorsapace: YUV */
{
	char _a='A', _b='A', _c='A', _d='A';
	int _bpp=0; int _depth=0; int _red_mask=0; int _green_mask=0; int _blue_mask=0; int _alpha_mask=0; int _endianness=0;

	if(__format == NULL) {
		debug_error("Image format is NULL\n");
		return;
	}

	debug_log("colorspace: %s(%d) width: %d stride: %d", __format->colorspace, strlen(__format->colorspace), __format->width, __format->stride);

	if(strcmp(__format->colorspace,"YUV") == 0) {
		if(strcmp(__format->format_label,"I420") == 0) {
			_a='I'; _b='4', _c='2', _d='0';
		}else if(strcmp(__format->format_label,"Y42B") == 0) {
			_a='Y'; _b='4', _c='2', _d='B';
		}else if(strcmp(__format->format_label,"Y444") == 0) {
			_a='Y'; _b='4', _c='4', _d='4';
		}else if(strcmp(__format->format_label,"YV12") == 0) {
			_a='Y'; _b='V', _c='1', _d='2'; /* GStreamer-CRITICAL **: gst_mini_object_ref: assertion `mini_object != NULL' failed */
		}else if(strcmp(__format->format_label,"NV12") == 0) {
			_a='N'; _b='V', _c='1', _d='2';
		}else if(strcmp(__format->format_label,"UYVY") == 0) {
			_a='U'; _b='Y', _c='V', _d='Y';
		}else if(strcmp(__format->format_label,"YUYV") == 0) {
			_a='Y'; _b='U', _c='Y', _d='2';
		}

		__format->caps = gst_caps_new_simple ("video/x-raw-yuv",
			"format", GST_TYPE_FOURCC, GST_MAKE_FOURCC (_a, _b, _c, _d), /*'I', '4', '2', '0'),*/
			"framerate", GST_TYPE_FRACTION, 25, 1,
			"pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
			"width", G_TYPE_INT, __format->width,
			"height", G_TYPE_INT, __format->height,
			"framerate", GST_TYPE_FRACTION, 1, 1,
			NULL);
	}

	else if(strcmp(__format->colorspace,"RGB") ==0 || strcmp(__format->colorspace,"BGRX") ==0) {
		if(strcmp(__format->format_label,"RGB888") == 0) {
			_bpp=24; _depth=24; _red_mask=16711680; _green_mask=65280; _blue_mask=255; _endianness=4321;
		}else if(strcmp(__format->format_label,"BGR888") == 0) {
			_bpp=24; _depth=24; _red_mask=255; _green_mask=65280; _blue_mask=16711680; _endianness=4321;
		}else if(strcmp(__format->format_label,"RGB565") == 0) {
			_bpp=16; _depth=16; _red_mask=63488; _green_mask=2016; _blue_mask=31; _endianness=1234;
		}else if( (strcmp(__format->format_label, "BGRX") == 0)) {
			_bpp=32; _depth=24; _red_mask=65280; _green_mask=16711680; _blue_mask=-16777216; _endianness=4321;
		}
		__format->caps = gst_caps_new_simple ("video/x-raw-rgb",
			"bpp", G_TYPE_INT, _bpp,
			"depth", G_TYPE_INT, _depth,
			"red_mask", G_TYPE_INT, _red_mask,
			"green_mask", G_TYPE_INT, _green_mask,
			"blue_mask", G_TYPE_INT, _blue_mask,
			"width", G_TYPE_INT, __format->width,
			"height", G_TYPE_INT, __format->height,
			"endianness", G_TYPE_INT, _endianness,
			"framerate", GST_TYPE_FRACTION, 1, 1, NULL);
	}

	else if(strcmp(__format->colorspace,"RGBA") ==0) {
		if(strcmp(__format->format_label,"ARGB8888") == 0) { /*[Low Arrary Address] ARGBARGB... [High Array Address]*/
			_bpp=32; _depth=32; _red_mask=16711680; _green_mask=65280; _blue_mask=255; _alpha_mask=-16777216; _endianness=4321;
		}else if(strcmp(__format->format_label,"BGRA8888") == 0) { /*[Low Arrary Address] BGRABGRA...[High Array Address]*/
		_bpp=32; _depth=32; _red_mask=65280; _green_mask=16711680; _blue_mask=-16777216; _alpha_mask=255; _endianness=4321;
		}else if(strcmp(__format->format_label,"RGBA8888") == 0) { /*[Low Arrary Address] RGBARGBA...[High Array Address]*/
			_bpp=32; _depth=32; _red_mask=-16777216; _green_mask=16711680; _blue_mask=65280; _alpha_mask=255; _endianness=4321;
		}else if(strcmp(__format->format_label,"ABGR8888") == 0) { /*[Low Arrary Address] ABGRABGR...[High Array Address]*/
		_bpp=32; _depth=32; _red_mask=255; _green_mask=65280; _blue_mask=16711680; _alpha_mask=-16777216; _endianness=4321;
		}else {
			debug_error("***Wrong format cs type***\n");
		}

		__format->caps = gst_caps_new_simple ("video/x-raw-rgb",
			"bpp", G_TYPE_INT, _bpp,
			"depth", G_TYPE_INT, _depth,
			"red_mask", G_TYPE_INT, _red_mask,
			"green_mask", G_TYPE_INT, _green_mask,
			"blue_mask", G_TYPE_INT, _blue_mask,
			"width", G_TYPE_INT, __format->width,
			"height", G_TYPE_INT, __format->height,
			"alpha_mask", G_TYPE_INT,_alpha_mask,
			"endianness", G_TYPE_INT, _endianness,
			"framerate", GST_TYPE_FRACTION, 1, 1, NULL);
	}
	if(__format->caps) {
		debug_log("###__format->caps is not NULL###, %p", __format->caps);
	}else {
		debug_error("__format->caps is NULL");
	}
}

static void
_mm_set_image_colorspace( image_format_s* __format)
{
	debug_log("format_label: %s\n", __format->format_label);

	__format->colorspace = (char*)malloc(sizeof(char) * IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	memset(__format->colorspace, 0, IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	if( (strcmp(__format->format_label, "I420") == 0) ||(strcmp(__format->format_label, "Y42B") == 0) || (strcmp(__format->format_label, "Y444") == 0)
		|| (strcmp(__format->format_label, "YV12") == 0) ||(strcmp(__format->format_label, "NV12") == 0) ||(strcmp(__format->format_label, "UYVY") == 0) ||(strcmp(__format->format_label, "YUYV") == 0)) {
		strncpy(__format->colorspace, "YUV", IMAGE_FORMAT_LABEL_BUFFER_SIZE-1);
	}else if( (strcmp(__format->format_label, "RGB888") == 0) ||(strcmp(__format->format_label, "BGR888") == 0) ||(strcmp(__format->format_label, "RGB565") == 0)) {
		strncpy(__format->colorspace, "RGB", IMAGE_FORMAT_LABEL_BUFFER_SIZE-1);
	}else if( (strcmp(__format->format_label, "ARGB8888") == 0) || (strcmp(__format->format_label, "BGRA8888") == 0) ||(strcmp(__format->format_label, "RGBA8888") == 0)	|| (strcmp(__format->format_label, "ABGR8888") == 0)) {
		strncpy(__format->colorspace, "RGBA",IMAGE_FORMAT_LABEL_BUFFER_SIZE-1);
	}else if( (strcmp(__format->format_label, "BGRX") == 0)) {
		strncpy(__format->colorspace, "BGRX", IMAGE_FORMAT_LABEL_BUFFER_SIZE-1);
	}else {
		debug_error("Check your colorspace format label");
		GSTCS_FREE(__format->colorspace);
	}
}

static image_format_s*
_mm_set_input_image_format_s_struct(imgp_info_s* pImgp_info) /* char* __format_label, int __width, int __height) */
{
	image_format_s* __format = NULL;

	__format=(image_format_s*)malloc(sizeof(image_format_s));
	memset(__format, 0, sizeof(image_format_s));

	__format->format_label = (char *)malloc(sizeof(char) * IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	memset(__format->format_label, 0, IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	strncpy(__format->format_label, pImgp_info->input_format_label, strlen(pImgp_info->input_format_label));
	debug_log("input_format_label: %s\n", pImgp_info->input_format_label);
	_mm_set_image_colorspace(__format);

	__format->width=pImgp_info->src_width;
	__format->height=pImgp_info->src_height;

	__format->blocksize = mm_setup_image_size(pImgp_info->input_format_label, pImgp_info->src_width, pImgp_info->src_height);
	debug_log("blocksize: %d\n", __format->blocksize);
	_mm_set_image_format_s_capabilities(__format);

	return __format;
}

static void
_mm_round_up_output_image_widh_height(image_format_s* pFormat)
{
	if(strcmp(pFormat->colorspace,"YUV") ==0) {
		pFormat->stride=MM_UTIL_ROUND_UP_8(pFormat->width);
		pFormat->elevation=MM_UTIL_ROUND_UP_2(pFormat->height);
	} else if(strcmp(pFormat->colorspace, "RGB") ==0) {
		pFormat->stride=MM_UTIL_ROUND_UP_4(pFormat->width);
		pFormat->elevation=MM_UTIL_ROUND_UP_2(pFormat->height);
	}
}

static image_format_s*
_mm_set_output_image_format_s_struct(imgp_info_s* pImgp_info)
{
	image_format_s* __format = NULL;

	__format=(image_format_s*)malloc(sizeof(image_format_s));
	memset(__format, 0, sizeof(image_format_s));

	__format->format_label = (char *)malloc(sizeof(char) * IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	memset(__format->format_label, 0, IMAGE_FORMAT_LABEL_BUFFER_SIZE);
	strncpy(__format->format_label, pImgp_info->output_format_label, strlen(pImgp_info->output_format_label));
	_mm_set_image_colorspace(__format);

	__format->width=pImgp_info->dst_width;
	__format->height=pImgp_info->dst_height;
	_mm_round_up_output_image_widh_height(__format);

	__format->blocksize = mm_setup_image_size(pImgp_info->output_format_label, pImgp_info->dst_width, pImgp_info->dst_height);
	debug_log("output_format_label: %s", pImgp_info->output_format_label);
	_mm_set_image_format_s_capabilities(__format);
	return __format;
}

static gboolean
__mm_check_resize_format( char* _in_format_label, int _in_w, int _in_h, char* _out_format_label, int _out_w, int _out_h)
{
	gboolean _bool=TRUE;
	debug_log("input image format: %s input image width: %d input image height: %d output image format: %s output image width: %d output image height: %d\n",
	_in_format_label, _in_w,_in_h, _out_format_label, _out_w, _out_h);
	if(_mm_check_resize_format(_in_w, _in_h, _out_w, _out_h)) {
		if( !( _mm_check_resize_format_label(_in_format_label) ||_mm_check_resize_format_label(_out_format_label) ) ) {
			debug_error("ERROR-%s | %s Resize Only [AYUV] [YUY2] [YVYU] [UYVY] [Y800] [I420][YV12] format can use vidoeflip ERROR- Resize!!!#####\n", _in_format_label, _out_format_label);
			_bool = FALSE;
		}
	}
	return _bool;
}

static gboolean
__mm_check_rotate_format(int angle, const char* input_format_label, const char* output_format_label)
{
	gboolean _bool=TRUE;
	debug_log("rotate value: %d input_format_label: %s output_format_label: %s\n",
	angle, input_format_label, output_format_label);
	if(_mm_check_rotate_format(angle)) {
		if(!(_mm_check_rotate_format_label(input_format_label) || _mm_check_rotate_format_label(output_format_label))) {
			debug_error("ERROR- %s | %s Rotate Only IYUV, I420, YV12 format can use vidoeflip ERROR -Rotate", input_format_label, output_format_label);
			_bool = FALSE;
		}
	}
	return _bool;
}

static int
_mm_push_buffer_into_pipeline(imgp_info_s* pImgp_info, unsigned char *src, unsigned char *dst, gstreamer_s * pGstreamer_s, GstCaps*_caps)
{
	int ret = MM_ERROR_NONE;
	if(_caps==NULL) {
		debug_error("caps is NULL\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(pGstreamer_s->pipeline == NULL) {
		debug_error("pipeline is NULL\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	GstBuffer* gst_buf = (GstBuffer *) gst_mini_object_new (GST_TYPE_BUFFER);

	if(gst_buf==NULL) {
		debug_error("buffer is NULL\n");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	int gst_imgsize = mm_setup_image_size(pImgp_info->input_format_label, pImgp_info->src_width, pImgp_info->src_height);
	debug_log("GST_BUFFER_SIZE (src): %d", gst_imgsize);
	if(pImgp_info->src_format == MM_UTIL_IMG_FMT_RGB888 && pImgp_info->src_width %4 != 0) {
		int real_width = pImgp_info->src_width * 3;
		int stride = GST_ROUND_UP_4(real_width);
		int i, y;
		guint8 * data =(guint8 *) g_malloc (gst_imgsize);
		if(data==NULL) {
			debug_error("app_buffer is NULL\n");
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
		for (y = 0; y < pImgp_info->src_height; y++) {
			guint8 *pLine = (guint8 *) &(src[real_width * y]);
			for(i=0; i < real_width; i ++) {
				data[y * stride + i] = pLine[i];
			}
			for(i=real_width; i < stride; i ++) {
				data[y * stride + i] = 0x00;
			}
		}
		debug_log("[padding] GST_BUFFER_SIZE (data): %d", GST_BUFFER_SIZE (data));
		GST_BUFFER_DATA (gst_buf) = (guint8 *) data;
	}else {
		GST_BUFFER_DATA (gst_buf) = (guint8 *) src;
	}
	GST_BUFFER_SIZE (gst_buf) = gst_imgsize;
	GST_BUFFER_FLAG_SET (gst_buf, GST_BUFFER_FLAG_READONLY);
	gst_buffer_set_caps (gst_buf, _caps);

	gst_app_src_push_buffer (GST_APP_SRC (pGstreamer_s->appsrc), gst_buf); /* push buffer to pipeline */
	g_free(GST_BUFFER_MALLOCDATA(gst_buf)); gst_buf = NULL; /* gst_buffer_finalize(gst_buf) { buffer->free_func (buffer->malloc_data); } */
	return ret;
}


static int
_mm_imgp_gstcs_processing( gstreamer_s* pGstreamer_s, unsigned char *src, unsigned char *dst, image_format_s* input_format, image_format_s* output_format, imgp_info_s* pImgp_info)
{
	GstBus *bus = NULL;
	GstStateChangeReturn ret_state;
	int ret = MM_ERROR_NONE;
	/*create pipeline*/
	ret = _mm_create_pipeline(pGstreamer_s);
	if(ret != MM_ERROR_NONE) {
		debug_error("ERROR - mm_create_pipeline ");
	}


	/* Make appsink emit the "new-preroll" and "new-buffer" signals. This option is by default disabled because signal emission is expensive and unneeded when the application prefers to operate in pull mode. */
	 gst_app_sink_set_emit_signals ((GstAppSink*)pGstreamer_s->appsink, TRUE);

	bus = gst_pipeline_get_bus (GST_PIPELINE (pGstreamer_s->pipeline)); /* GST_PIPELINE (pipeline)); */
	/* gst_bus_add_watch (bus, (GstBusFunc) _mm_on_sink_message, pGstreamer_s); thow to appplicaton */
	gst_object_unref(bus);

	debug_log("Start mm_push_buffer_into_pipeline");
	ret = _mm_push_buffer_into_pipeline(pImgp_info, src, dst, pGstreamer_s, input_format->caps);
	if(ret != MM_ERROR_NONE) {
		debug_error("ERROR - mm_push_buffer_into_pipeline ");
	}
	debug_log("End mm_push_buffer_into_pipeline");

	/*link pipeline*/
	debug_log("Start mm_link_pipeline");
	_mm_link_pipeline( pGstreamer_s, input_format, output_format, pImgp_info->angle);
	debug_log("End mm_link_pipeline");

	/* Conecting to the new-buffer signal emited by the appsink*/
	debug_log("Start G_CALLBACK (_mm_sink_preroll)");
	g_signal_connect (pGstreamer_s->appsink, "new-preroll", G_CALLBACK (_mm_sink_preroll), pGstreamer_s);
	debug_log("End G_CALLBACK (mm_sink_preroll)");

	/* GST_STATE_PLAYING*/
	debug_log("Start GST_STATE_PLAYING");
	ret_state = gst_element_set_state (pGstreamer_s->pipeline, GST_STATE_PLAYING);
	debug_log("End GST_STATE_PLAYING ret_state: %d", ret_state);
	ret_state = gst_element_get_state (pGstreamer_s->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
	debug_log("Success Get GST_STATE_PLAYING ret_state: %d", ret_state);
	#if 0
	/* Conecting to the new-buffer signal emited by the appsink*/
	debug_log("Start G_CALLBACK (mm_sink_buffer)");
	g_signal_connect (pGstreamer_s->appsink, "new-buffer", G_CALLBACK (_mm_sink_buffer), pGstreamer_s);
	debug_log("End G_CALLBACK (mm_sink_buffer)");
	#endif
	if(ret_state == 1) {
		debug_log("GST_STATE_PLAYING ret = %d( GST_STATE_CHANGE_SUCCESS)", ret_state);
	}else if( ret_state == 2) {
		debug_log("GST_STATE_PLAYING ret = %d( GST_STATE_CHANGE_ASYNC)", ret_state);
	}

	debug_log("Sucess GST_STATE_CHANGE");

	/* error */
	if (ret_state == 0) 	{
		debug_error("GST_STATE_CHANGE_FAILURE");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}else {
		#if 0
			g_main_loop_run (pGstreamer_s->loop);
		#endif
		debug_log("Set GST_STATE_NULL");

		/*GST_STATE_NULL*/
		gst_element_set_state (pGstreamer_s->pipeline, GST_STATE_NULL);
		debug_log("End GST_STATE_NULL");

		debug_log("###pGstreamer_s->output_buffer### : %p", pGstreamer_s->output_buffer);

		ret_state = gst_element_get_state (pGstreamer_s->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);

		if(ret_state == 1) {
			debug_log("GST_STATE_NULL ret_state = %d (GST_STATE_CHANGE_SUCCESS)\n", ret_state);
		}else if( ret_state == 2) {
			debug_log("GST_STATE_NULL ret_state = %d (GST_STATE_CHANGE_ASYNC)\n", ret_state);
		}

		debug_log("Success gst_element_get_state\n");

		if (ret_state == 0) 	{
			debug_error("GST_STATE_CHANGE_FAILURE");
		}else {
			if(pGstreamer_s->output_buffer != NULL) {
				int buffer_size = GST_BUFFER_SIZE(pGstreamer_s->output_buffer);
				debug_log("buffer size: %d\n", buffer_size);
				if( buffer_size != mm_setup_image_size(pImgp_info->output_format_label, pImgp_info->output_stride, pImgp_info->output_elevation)) {
					debug_log("Buffer size is different stride:%d elevation: %d\n", pImgp_info->output_stride, pImgp_info->output_elevation);
				}
				debug_log("pGstreamer_s->output_buffer: 0x%2x\n", pGstreamer_s->output_buffer);
				memcpy(dst, (char*)GST_BUFFER_DATA(pGstreamer_s->output_buffer), buffer_size);
			}else {
				debug_log("pGstreamer_s->output_buffer is NULL");
			}
		}
		gst_object_unref (pGstreamer_s->pipeline);
		g_free(GST_BUFFER_MALLOCDATA(pGstreamer_s->output_buffer));
		pGstreamer_s->output_buffer = NULL;
		g_free (pGstreamer_s);

		debug_log("End mm_convert_colorspace \n");
	}
	debug_log("dst: %p", dst);
	return ret;
}


static int
mm_setup_image_size(const char* _format_label, int width, int height)
{
	int size=0;

	if(strcmp(_format_label, "I420") == 0) {
		setup_image_size_I420(width, height); /*width * height *1.5; */
	}else if(strcmp(_format_label, "Y42B") == 0) {
		setup_image_size_Y42B(width, height); /*width * height *2; */
	}else if(strcmp(_format_label, "YUV422") == 0) {
		setup_image_size_Y42B(width, height); /*width * height *2; */
	}else if(strcmp(_format_label, "Y444") == 0) {
		setup_image_size_Y444(width, height); /* width * height *3; */
	}else if(strcmp(_format_label, "YV12") == 0) {
		setup_image_size_YV12(width, height); /* width * height *1; */
	}else if(strcmp(_format_label, "NV12") == 0) {
		setup_image_size_NV12(width, height); /* width * height *1.5; */
	}else if(strcmp(_format_label, "RGB565") == 0) {
		setup_image_size_RGB565(width, height); /* width * height *2; */
	}else if(strcmp(_format_label, "RGB888") == 0) {
		setup_image_size_RGB888(width, height); /* width * height *3; */
	}else if(strcmp(_format_label, "BGR888") == 0) {
		setup_image_size_BGR888(width, height); /* width * height *3; */
	}else if(strcmp(_format_label, "UYVY") == 0) {
		setup_image_size_UYVY(width, height); /* width * height *2; */
	}else if(strcmp(_format_label, "YUYV") == 0) {
		setup_image_size_YUYV(width, height); /* width * height *2; */
	}else if(strcmp(_format_label, "ARGB8888") == 0) {
	size = width * height *4; debug_log("file_size: %d\n", size);
	}else if(strcmp(_format_label, "BGRA8888") == 0) {
		size = width * height *4; debug_log("file_size: %d\n", size);
	}else if(strcmp(_format_label, "RGBA8888") == 0) {
		size = width * height *4; debug_log("file_size: %d\n", size);
	}else if(strcmp(_format_label, "ABGR8888") == 0) {
		size = width * height *4; debug_log("file_size: %d\n", size);
	}else if(strcmp(_format_label, "BGRX") == 0) {
		size = width * height *4; debug_log("file_size: %d\n", size);
	}

	return size;
}

static int
_mm_imgp_gstcs(imgp_info_s* pImgp_info, unsigned char *src, unsigned char *dst)
{
	image_format_s* input_format=NULL, *output_format=NULL;
	gstreamer_s* pGstreamer_s;
	int ret = MM_ERROR_NONE;
	static const int max_argc = 50;
	gint* argc = NULL;
	gchar** argv = NULL;

	g_type_init();
	if(pImgp_info == NULL) {
		debug_error("imgp_info_s is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	if(dst == NULL) {
		debug_error("dst is NULL");
		return MM_ERROR_IMAGE_INVALID_VALUE;
	}

	debug_log("[input] format label : %s width: %d height: %d\t[output] format label: %s width: %d height: %d rotation vaule: %d",
		pImgp_info->input_format_label, pImgp_info->src_width, pImgp_info->src_height, pImgp_info->output_format_label, pImgp_info->dst_width, pImgp_info->dst_height, pImgp_info->angle);

	input_format= _mm_set_input_image_format_s_struct(pImgp_info);
	output_format= _mm_set_output_image_format_s_struct(pImgp_info);

	pImgp_info->output_stride = output_format->stride;
	pImgp_info->output_elevation = output_format->elevation;

	debug_log("mm_check_resize_format&&mm_check_rotate_format ");

	if(__mm_check_resize_format(pImgp_info->input_format_label, pImgp_info->src_width, pImgp_info->src_height, pImgp_info->output_format_label, pImgp_info->dst_width, pImgp_info->dst_height)
		&& __mm_check_rotate_format(pImgp_info->angle, pImgp_info->input_format_label, pImgp_info->output_format_label) ) {
		argc = malloc(sizeof(int));
		argv = malloc(sizeof(gchar*) * max_argc);

		if (!argc || !argv) {
			debug_error("argc ||argv is NULL");
			free(input_format);
			free(output_format);
			if (argc != NULL) {
				free(argc);
			}
			if (argv != NULL) {
				free(argv);
			}
			return MM_ERROR_IMAGE_INVALID_VALUE;
		}
		memset(argv, 0, sizeof(gchar*) * max_argc);
		debug_log("memset argv");

		/* add initial */
		*argc = 1;
		argv[0] = g_strdup( "mmutil_gstcs" );
		/* check disable registry scan */
		argv[*argc] = g_strdup("--gst-disable-registry-update");
		(*argc)++;
		debug_log("--gst-disable-registry-update");

		gst_init(argc, &argv);

		pGstreamer_s = g_new0 (gstreamer_s, 1);

		/* _format_label : I420, RGB888 etc*/
		debug_log("Start _mm_imgp_gstcs_processing ");
		ret =_mm_imgp_gstcs_processing(pGstreamer_s, src, dst, input_format, output_format, pImgp_info); /* input: buffer pointer for input image , input image format, input image width, input image height, output: buffer porinter for output image */

		if(ret == MM_ERROR_NONE) {
			debug_log("End _mm_imgp_gstcs_processing [dst: %p]", dst);
		}else if (ret != MM_ERROR_NONE) {
			debug_error("ERROR - _mm_imgp_gstcs_processing");
		}
	}else {
		debug_error("Error - Check your input / ouput image input_format_label: %s src_width: %d src_height: %d output_format_label: %s output_stride: %d output_elevation: %d angle: %d ",
		pImgp_info->input_format_label, pImgp_info->src_width, pImgp_info->src_height, pImgp_info->output_format_label, pImgp_info->output_stride, pImgp_info->output_elevation, pImgp_info->angle);
		ret = MM_ERROR_IMAGE_INVALID_VALUE;
	}

	GSTCS_FREE(argv);
	GSTCS_FREE(argc)

	GSTCS_FREE(input_format->format_label);
	GSTCS_FREE(input_format->colorspace);
	GSTCS_FREE(input_format);

	GSTCS_FREE(output_format->format_label);
	GSTCS_FREE(output_format->colorspace);
	GSTCS_FREE(output_format);

	return ret;
}

int
mm_imgp(imgp_info_s* pImgp_info, unsigned char *src, unsigned char *dst, imgp_type_e _imgp_type)
{
	if (pImgp_info == NULL) {
		debug_error("input vaule is error");
	}
	return _mm_imgp_gstcs(pImgp_info ,src, dst);
}
