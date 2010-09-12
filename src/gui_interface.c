/* nekobee DSSI software synthesizer GUI
 *
 * Copyright (C) 2004 Sean Bolton
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gtkknob.h"

#include "nekobee.h"
#include "nekobee_ports.h"
#include "nekobee_synth.h"
#include "gui_callbacks.h"
#include "gui_interface.h"

GtkWidget *main_window;
GtkWidget *save_file_selection;

GtkWidget *about_window;
GtkWidget *about_label;

GtkWidget *osc1_waveform_pixmap;

GtkObject *tuning_adj;
GtkObject *polyphony_adj;
GtkWidget *monophonic_option_menu;
GtkWidget *glide_option_menu;
GtkObject *bendrange_adj;

GtkObject *voice_widget[XSYNTH_PORTS_COUNT];

#if GTK_CHECK_VERSION(2, 0, 0)
#define GTK20SIZEGROUP  GtkSizeGroup
#else
#define GTK20SIZEGROUP  gpointer
#endif

/*
 * create_voice_slider
 *
 * create a patch edit 'slider' - actually a label, a knob, and a spinbutton
 *
 * example:
 *    osc1_pitch = create_voice_slider(main_window, XSYNTH_PORT_TUNING,
 *                                     table1, 0, 0, "pitch", sizegroup);
 */
static void
create_voice_slider(GtkWidget *main_window, int index, GtkWidget *table,
                    gint column, gint row, const char *text,
                    GTK20SIZEGROUP *labelgroup)
{
    GtkWidget *label, *knob;
    GtkObject *adjustment;

    label = gtk_label_new (text);
    gtk_widget_ref (label);
    gtk_object_set_data_full (GTK_OBJECT (main_window), text, label,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, column, column + 1,
                      row, row + 1, (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0.25, 0);
#if GTK_CHECK_VERSION(2, 0, 0)
    if (labelgroup != NULL)
        gtk_size_group_add_widget (labelgroup, label);
#endif

    adjustment = gtk_adjustment_new (0, 0, 10, 0.005, 1, 0);
    voice_widget[index] = adjustment;

    knob = GTK_WIDGET(neko_knob_new (GTK_ADJUSTMENT (adjustment)));
    gtk_widget_ref (knob);
    gtk_object_set_data_full (GTK_OBJECT (main_window), text, knob,
                              (GtkDestroyNotify) gtk_widget_unref);
    gtk_widget_show (knob);
    gtk_table_attach (GTK_TABLE (table), knob, column, column + 1,
                      row+1, row + 2, (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
}

void
create_main_window (const char *tag)
{
  GtkWidget *vbox1;
  GtkWidget *menubar1;
  GtkWidget *file1;
  GtkWidget *file1_menu;
#if !GTK_CHECK_VERSION(2, 0, 0)
  GtkAccelGroup *file1_menu_accels;
  GtkAccelGroup *help1_menu_accels;
#endif
  GtkWidget *menu_quit;
  GtkWidget *help1;
  GtkWidget *help1_menu;
  GtkWidget *menu_about;

  GtkWidget *patch_edit_table;
    GTK20SIZEGROUP *col1_sizegroup;
    GTK20SIZEGROUP *col2_sizegroup;
    GTK20SIZEGROUP *col3_sizegroup;


  GtkWidget *test_note_frame;
  GtkWidget *test_note_table;
  GtkWidget *label10;
  GtkWidget *test_note_button;
  GtkWidget *test_note_key;
  GtkWidget *test_note_velocity;

  GtkWidget *label53;

  GtkAccelGroup *accel_group;
    int i;

  accel_group = gtk_accel_group_new ();

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (main_window), "main_window", main_window);
  gtk_window_set_title (GTK_WINDOW (main_window), tag);
  gtk_widget_realize(main_window);  /* window must be realized for create_*_pixmap() */

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox1);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "vbox1", vbox1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (main_window), vbox1);

// menu bar
  menubar1 = gtk_menu_bar_new ();
  gtk_widget_ref (menubar1);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "menubar1", menubar1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menubar1);
  gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);

// File Menu
  file1 = gtk_menu_item_new_with_label ("File");
  gtk_widget_ref (file1);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "file1", file1,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (file1);
  gtk_container_add (GTK_CONTAINER (menubar1), file1);

  file1_menu = gtk_menu_new ();
  gtk_widget_ref (file1_menu);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "file1_menu", file1_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file1), file1_menu);
#if !GTK_CHECK_VERSION(2, 0, 0)
  file1_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (file1_menu));
#endif
// menu item quit
  menu_quit = gtk_menu_item_new_with_label ("Quit");
  gtk_widget_ref (menu_quit);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "menu_quit", menu_quit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menu_quit);
  gtk_container_add (GTK_CONTAINER (file1_menu), menu_quit);
  gtk_widget_add_accelerator (menu_quit, "activate", accel_group,
                              GDK_Q, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

// menu item about
  help1 = gtk_menu_item_new_with_label ("About");
  gtk_widget_ref (help1);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "help1", help1,
                            (GtkDestroyNotify) gtk_widget_unref);
//  gtk_widget_show (help1);
  gtk_container_add (GTK_CONTAINER (menubar1), help1);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help1));

// help menu
  help1_menu = gtk_menu_new ();
  gtk_widget_ref (help1_menu);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "help1_menu", help1_menu,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help1), help1_menu);
#if !GTK_CHECK_VERSION(2, 0, 0)
  help1_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (help1_menu));
#endif
// menu item about
  menu_about = gtk_menu_item_new_with_label ("About nekobee-DSSI");
  gtk_widget_ref (menu_about);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "menu_about", menu_about,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (menu_about);
  gtk_container_add (GTK_CONTAINER (help1_menu), menu_about);

    /* Patch Edit tab */
    for (i = 0; i < XSYNTH_PORTS_COUNT; i++) voice_widget[i] = NULL;

  patch_edit_table = gtk_table_new (1, 8, FALSE);
  gtk_widget_ref (patch_edit_table);
  gtk_object_set_data_full (GTK_OBJECT (vbox1), "patch_edit_table", patch_edit_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (patch_edit_table);
  gtk_container_add (GTK_CONTAINER (vbox1), patch_edit_table);
  gtk_container_set_border_width (GTK_CONTAINER (patch_edit_table), 6);
  gtk_table_set_row_spacings (GTK_TABLE (patch_edit_table), 7);
  gtk_table_set_col_spacings (GTK_TABLE (patch_edit_table), 7);

#if GTK_CHECK_VERSION(2, 0, 0)
    col1_sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    col2_sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    col3_sizegroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
#endif

    create_voice_slider(main_window, XSYNTH_PORT_TUNING, patch_edit_table, 0, 0, "Tuning", col1_sizegroup);
    GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_TUNING])->lower = -12.0f;
	GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_TUNING])->upper = 12.0f;
    gtk_adjustment_changed (GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_TUNING]));
    create_voice_slider(main_window, XSYNTH_PORT_WAVEFORM, patch_edit_table, 1, 0, "Square/Saw", col1_sizegroup);
	GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_WAVEFORM])->upper = 1.0f;
	GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_WAVEFORM])->step_increment = 1.0f;
	gtk_adjustment_changed (GTK_ADJUSTMENT(voice_widget[XSYNTH_PORT_WAVEFORM]));
   
    create_voice_slider(main_window, XSYNTH_PORT_CUTOFF, patch_edit_table, 2, 0, "Cutoff", col1_sizegroup);
    create_voice_slider(main_window, XSYNTH_PORT_RESONANCE, patch_edit_table, 3, 0, "Resonance", col1_sizegroup);
    create_voice_slider(main_window, XSYNTH_PORT_ENVMOD, patch_edit_table, 4, 0, "Env Mod", col1_sizegroup);
    create_voice_slider(main_window, XSYNTH_PORT_DECAY, patch_edit_table, 5, 0, "Decay", col1_sizegroup);
    create_voice_slider(main_window, XSYNTH_PORT_ACCENT, patch_edit_table, 6, 0, "Accent", col1_sizegroup);
    create_voice_slider(main_window, XSYNTH_PORT_VOLUME, patch_edit_table, 7, 0, "Volume", col1_sizegroup);

  test_note_frame = gtk_frame_new ("Test Note");
  gtk_widget_ref (test_note_frame);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "test_note_frame", test_note_frame,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (test_note_frame);
  gtk_container_set_border_width (GTK_CONTAINER (test_note_frame), 5);

  test_note_table = gtk_table_new (3, 3, FALSE);
  gtk_widget_ref (test_note_table);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "test_note_table", test_note_table,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (test_note_table);
  gtk_container_add (GTK_CONTAINER (test_note_frame), test_note_table);
  gtk_container_set_border_width (GTK_CONTAINER (test_note_table), 2);
  gtk_table_set_row_spacings (GTK_TABLE (test_note_table), 1);
  gtk_table_set_col_spacings (GTK_TABLE (test_note_table), 5);

  label10 = gtk_label_new ("key");
  gtk_widget_ref (label10);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "label10", label10,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label10);
  gtk_table_attach (GTK_TABLE (test_note_table), label10, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  label53 = gtk_label_new ("velocity");
  gtk_widget_ref (label53);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "label53", label53,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (label53);
  gtk_table_attach (GTK_TABLE (test_note_table), label53, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label53), 0, 0.5);

  test_note_button = gtk_button_new_with_label ("Send Test Note");
  gtk_widget_ref (test_note_button);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "test_note_button", test_note_button,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (test_note_button);
  gtk_table_attach (GTK_TABLE (test_note_table), test_note_button, 2, 3, 0, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 4, 0);

  test_note_key = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (60, 12, 120, 1, 12, 12)));
  gtk_widget_ref (test_note_key);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "test_note_key", test_note_key,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (test_note_key);
  gtk_table_attach (GTK_TABLE (test_note_table), test_note_key, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_scale_set_value_pos (GTK_SCALE (test_note_key), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (test_note_key), 0);
  gtk_range_set_update_policy (GTK_RANGE (test_note_key), GTK_UPDATE_DELAYED);

  test_note_velocity = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (96, 1, 137, 1, 10, 10)));
  gtk_widget_ref (test_note_velocity);
  gtk_object_set_data_full (GTK_OBJECT (main_window), "test_note_velocity", test_note_velocity,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (test_note_velocity);
  gtk_table_attach (GTK_TABLE (test_note_table), test_note_velocity, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_scale_set_value_pos (GTK_SCALE (test_note_velocity), GTK_POS_RIGHT);
  gtk_scale_set_digits (GTK_SCALE (test_note_velocity), 0);
  gtk_range_set_update_policy (GTK_RANGE (test_note_velocity), GTK_UPDATE_DELAYED);

  gtk_box_pack_start (GTK_BOX (vbox1), test_note_frame, FALSE, FALSE, 0);

    gtk_signal_connect(GTK_OBJECT(main_window), "destroy",
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (main_window), "delete_event",
                        (GtkSignalFunc)on_delete_event_wrapper,
                        (gpointer)on_menu_quit_activate);
    
  gtk_signal_connect (GTK_OBJECT (menu_quit), "activate",
                      GTK_SIGNAL_FUNC (on_menu_quit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (menu_about), "activate",
                      GTK_SIGNAL_FUNC (on_menu_about_activate),
                      NULL);

    /* connect patch edit widgets */
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_TUNING]),
                       "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                       (gpointer)XSYNTH_PORT_TUNING);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_WAVEFORM]),
                       "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                       (gpointer)XSYNTH_PORT_WAVEFORM);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_CUTOFF]),
                        "value_changed", GTK_SIGNAL_FUNC (on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_CUTOFF);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_RESONANCE]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_RESONANCE);

    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_ENVMOD]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_ENVMOD);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_DECAY]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_DECAY);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_ACCENT]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_ACCENT);
    gtk_signal_connect (GTK_OBJECT (voice_widget[XSYNTH_PORT_VOLUME]),
                        "value_changed", GTK_SIGNAL_FUNC(on_voice_slider_change),
                        (gpointer)XSYNTH_PORT_VOLUME);

    /* connect test note widgets */
    gtk_signal_connect (GTK_OBJECT (gtk_range_get_adjustment (GTK_RANGE (test_note_key))),
                        "value_changed", GTK_SIGNAL_FUNC(on_test_note_slider_change),
                        (gpointer)0);
    gtk_signal_connect (GTK_OBJECT (gtk_range_get_adjustment (GTK_RANGE (test_note_velocity))),
                        "value_changed", GTK_SIGNAL_FUNC(on_test_note_slider_change),
                        (gpointer)1);
    gtk_signal_connect (GTK_OBJECT (test_note_button), "pressed",
                        GTK_SIGNAL_FUNC (on_test_note_button_press),
                        (gpointer)1);
    gtk_signal_connect (GTK_OBJECT (test_note_button), "released",
                        GTK_SIGNAL_FUNC (on_test_note_button_press),
                        (gpointer)0);


    gtk_window_add_accel_group (GTK_WINDOW (main_window), accel_group);
}

void
create_windows(const char *instance_tag)
{
    char tag[50];

    /* build a nice identifier string for the window titles */
    if (strlen(instance_tag) == 0) {
        strcpy(tag, "nekobee-DSSI");
    } else if (strstr(instance_tag, "nekobee-DSSI") ||
               strstr(instance_tag, "nekobee-dssi")) {
        if (strlen(instance_tag) > 49) {
            snprintf(tag, 50, "...%s", instance_tag + strlen(instance_tag) - 46); /* hope the unique info is at the end */
        } else {
            strcpy(tag, instance_tag);
        }
    } else {
        if (strlen(instance_tag) > 37) {
            snprintf(tag, 50, "nekobee-DSSI ...%s", instance_tag + strlen(instance_tag) - 34);
        } else {
            snprintf(tag, 50, "nekobee-DSSI %s", instance_tag);
        }
    }

    create_main_window(tag);
}
