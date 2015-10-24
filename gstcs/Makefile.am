ACLOCAL_AMFLAGS='-I m4'

lib_LTLIBRARIES = libmmutil_imgp_gstcs.la
#bin_PROGRAMS = mmutil_gstcs 

noinst_HEADERS = include/mm_util_gstcs.h \
		 include/mm_util_gstcs_internal.h

libmmutil_imgp_gstcs_la_SOURCES = mm_util_gstcs.c
	
libmmutil_imgp_gstcs_la_CFLAGS = -I$(srcdir)/include \
 	                     $(MMCOMMON_CFLAGS) \
			     $(GLIB_CFLAGS) \
			     $(GST_CFLAGS) \
			     $(GSTAPP_CFLAGS)  \
                             $(GST_PLUGIN_BASE_CFLAGS) \
                             $(GST_INTERFACE_CFLAGS) \
                             $(GSTPBUTILS_CFLAGS) \
                             $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x0100 -DMMF_DEBUG_PREFIX=\"MMF-IMAGE\"

libmmutil_imgp_gstcs_la_LIBADD = $(MMCOMMON_LIBS) \
			    $(GLIB_LIBS) \
			    $(GST_LIBS) \
			    $(GSTAPP_LIBS) \
                            $(GST_VIDEO_LIBS) \
                            $(GST_PLUGIN_BASE_LIBS) \
                            $(GST_INTERFACE_LIBS) \
                            $(GSTPBUTILS_LIBS) \
			    $(MMLOG_LIBS)
