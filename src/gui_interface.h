/* nekobee DSSI software synthesizer GUI
 *
 * Copyright (C) 2004 Sean Bolton and others.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef _GUI_INTERFACE_H
#define _GUI_INTERFACE_H

#include <gtk/gtk.h>

extern GtkWidget *main_window;
extern GtkWidget *save_file_selection;

extern GtkWidget *about_window;
extern GtkWidget *about_label;

extern GtkWidget *osc1_waveform_pixmap;

extern GtkObject *tuning_adj;
extern GtkObject *polyphony_adj;
extern GtkWidget *monophonic_option_menu;
extern GtkWidget *glide_option_menu;
extern GtkObject *bendrange_adj;

extern GtkObject *voice_widget[];

void create_windows(const char *instance_tag);

#endif /* _GUI_INTERFACE_H */
