## Process this file with automake to produce Makefile.in

if BUILD_GUI
bin_PROGRAMS = nekobee_gtk
else
bin_PROGRAMS =
endif

bindir = $(dssidir)/nekobee
plugindir = $(dssidir)

plugin_LTLIBRARIES = nekobee.la

nekobee_la_LDFLAGS = -module -avoid-version -Wc,-nostartfiles

nekobee_gtk_SOURCES = \
	gui_callbacks.c \
	gui_callbacks.h \
	gui_data.c \
	gui_data.h \
	gui_interface.c \
	gui_interface.h \
	gtkknob.c \
	gtkknob.h \
	gui_main.c \
	gui_main.h \
	nekobee.h \
	nekobee_data.c \
	nekobee_ports.c \
	nekobee_ports.h \
	nekobee_synth.h \
	nekobee_types.h \
	nekobee_voice.h

nekobee_gtk_CFLAGS = @GTK_CFLAGS@ $(AM_CFLAGS) -DINSTALL_DIR=\"$(bindir)\"

nekobee_gtk_LDADD = @GTK_LIBS@ -lm -llo -lpthread 

nekobee_la_SOURCES = \
	gui_data.h \
	nekobee-dssi.c \
	nekobee.h \
	nekobee_data.c \
	nekobee_ports.c \
	nekobee_ports.h \
	nekobee_synth.c \
	nekobee_synth.h \
	nekobee_types.h \
	nekobee_voice.c \
	nekobee_voice.h \
	nekobee_voice_render.c \
	minblep_tables.c

nekobee_la_LIBADD = -lm
