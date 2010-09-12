#! /usr/bin/env python
# encoding: utf-8

import os

# the following two variables are used by the target "waf dist"
VERSION='0.2'
APPNAME='nekobee'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')

    conf.check_cfg(package='dssi', args='--cflags --libs')
    conf.check_cfg(package='liblo', args='--cflags --libs')
    conf.check_cfg(package='gtk+-2.0', args='--cflags --libs')

    conf.env['DSSI_DIR'] = os.path.normpath(os.path.join(conf.env['PREFIX'], 'lib', 'dssi'))
    conf.env['INSTALL_DIR'] = os.path.join(conf.env['DSSI_DIR'], 'nekobee')

    conf.env.CCFLAGS +=  ['-DG_DISABLE_SINGLE_INCLUDES','-DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES', '-DGTK_DISABLE_SINGLE_INCLUDES']
    conf.env.CCFLAGS +=  ["-DG_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED"]

    conf.define('INSTALL_DIR', conf.env['INSTALL_DIR'])
    conf.write_config_header('config.h')

def build(bld):
    # DSSI plugin
    plugin_dssi = bld.new_task_gen('cc', 'shlib')
    plugin_dssi.env['shlib_PATTERN'] = '%s.so'
    plugin_dssi.env.append_value("LINKFLAGS", "-module -avoid-version -Wc,-nostartfiles")
    plugin_dssi.includes = ['.', 'src']
    plugin_dssi.defines = 'HAVE_CONFIG_H'
    plugin_dssi.source = [
	'src/nekobee-dssi.c',
	'src/nekobee_data.c',
	'src/nekobee_ports.c',
	'src/nekobee_synth.c',
	'src/nekobee_voice.c',
	'src/nekobee_voice_render.c',
	'src/minblep_tables.c',
        ]
    plugin_dssi.target = 'nekobee'
    plugin_dssi.install_path = '${DSSI_DIR}/'
    bld.install_files('${INSTALL_DIR}', 'extra/*')

    # DSSI UI executable
    gui_gtk = bld.new_task_gen('cc', 'program')
    gui_gtk.includes = ['.', 'src']
    gui_gtk.defines = 'HAVE_CONFIG_H'
    gui_gtk.source = [
	'src/gui_callbacks.c',
	'src/gui_data.c',
	'src/gui_interface.c',
	'src/nekoknob.c',
#	'src/gtk/slider.c',
	'src/gui_main.c',
	'src/nekobee_data.c',
	'src/nekobee_ports.c',
        ]
    gui_gtk.uselib = 'GTK+-2.0 LIBLO'
    gui_gtk.target = 'nekobee_gtk'
    gui_gtk.install_path = '${INSTALL_DIR}/'

