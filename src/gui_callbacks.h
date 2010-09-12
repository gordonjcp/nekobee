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

#ifndef _GUI_CALLBACKS_H
#define _GUI_CALLBACKS_H

#include <gtk/gtk.h>

#include "nekobee_types.h"

void on_menu_quit_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_menu_about_activate(GtkMenuItem *menuitem, gpointer user_data);
gint on_delete_event_wrapper(GtkWidget *widget, GdkEvent *event,
                             gpointer data);
void on_about_dismiss(GtkWidget *widget, gpointer data);
void on_notebook_switch_page(GtkNotebook *notebook, GtkNotebookPage *page,
                             guint page_num);
void on_voice_slider_change(GtkWidget *widget, gpointer data);
void on_test_note_slider_change(GtkWidget *widget, gpointer data);
void on_test_note_button_press(GtkWidget *widget, gpointer data);
void on_polyphony_change(GtkWidget *widget, gpointer data);
void update_voice_widget(int port, float value);
void update_voice_widgets_from_patch(nekobee_patch_t *patch);
void update_patch_from_voice_widgets(nekobee_patch_t *patch);

#endif  /* _GUI_CALLBACKS_H */
