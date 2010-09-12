/* nekobee DSSI software synthesizer GUI
 *
 * Copyright (C) 2004 Sean Bolton and others.
 *
 * Portions of this file may have come from Steve Brookes'
 * nekobee, copyright (C) 1999 S. J. Brookes.
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

#define _BSD_SOURCE    1
#define _SVID_SOURCE   1
#define _ISOC99_SOURCE 1

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <gtk/gtk.h>
#include <lo/lo.h>

#include "nekobee_types.h"
#include "nekobee.h"
#include "nekobee_ports.h"
#include "nekobee_voice.h"
#include "gui_main.h"
#include "gui_callbacks.h"
#include "gui_interface.h"
#include "gui_data.h"

extern GtkObject *voice_widget[XSYNTH_PORTS_COUNT];

extern nekobee_patch_t *patches;

extern lo_address osc_host_address;
extern char *     osc_configure_path;
extern char *     osc_control_path;
extern char *     osc_midi_path;
extern char *     osc_program_path;
extern char *     osc_update_path;

static int internal_gui_update_only = 0;

static unsigned char test_note_noteon_key = 60;
static unsigned char test_note_noteoff_key;
static unsigned char test_note_velocity = 96;

static gchar *file_selection_last_filename = NULL;
extern char  *project_directory;

void
file_selection_set_path(GtkWidget *file_selection)
{
    if (file_selection_last_filename) {
        gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection),
                                        file_selection_last_filename);
    } else if (project_directory && strlen(project_directory)) {
        if (project_directory[strlen(project_directory) - 1] != '/') {
            char buffer[PATH_MAX];
            snprintf(buffer, PATH_MAX, "%s/", project_directory);
            gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection),
                                            buffer);
        } else {
            gtk_file_selection_set_filename(GTK_FILE_SELECTION(file_selection),
                                            project_directory);
        }
    }
}

void
on_menu_quit_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_main_quit();
}


void
on_menu_about_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    char buf[256];
    snprintf(buf, 256, "nekobee version: " VERSION "\n"
                       "plugin URL: %s\n"
                       "host URL: %s\n", osc_self_url, osc_host_url);
    gtk_label_set_text (GTK_LABEL (about_label), buf);
    gtk_widget_show(about_window);
}

gint
on_delete_event_wrapper( GtkWidget *widget, GdkEvent *event, gpointer data )
{
    void (*handler)(GtkWidget *, gpointer) = (void (*)(GtkWidget *, gpointer))data;

    /* call our 'close', 'dismiss' or 'cancel' callback (which must not need the user data) */
    (*handler)(widget, NULL);

    /* tell GTK+ to NOT emit 'destroy' */
    return TRUE;
}

void
on_about_dismiss( GtkWidget *widget, gpointer data )
{
    gtk_widget_hide(about_window);
}

void
on_notebook_switch_page(GtkNotebook     *notebook,
                        GtkNotebookPage *page,
                        guint            page_num)
{
    GDB_MESSAGE(GDB_GUI, " on_notebook_switch_page: page %d selected\n", page_num);
}

void
on_voice_slider_change( GtkWidget *widget, gpointer data )
{
    int index = (int)data;
    struct nekobee_port_descriptor *xpd = &nekobee_port_description[index];
    float cval = GTK_ADJUSTMENT(widget)->value / 10.0f;
    float value, a, b, c, d;
	
	a=0; b=0; c=0;
	
    if (internal_gui_update_only) {
        /* GDB_MESSAGE(GDB_GUI, " on_voice_slider_change: skipping further action\n"); */
        return;
    }

    if (xpd->type == XSYNTH_PORT_TYPE_LINEAR) {
        // used in updating: cval = (value - xpd->c) / xpd->b;
        value = (xpd->a * cval + xpd->b) * cval + xpd->c;  /* linear or quadratic */
		// 01:01 < CHodapp> gordonjcp, use quadratic formula . . . d = sqrt(b*b - 4*a*(c-y)), x1 = (-b + d) / (2*a), x2 = (-b - d) / (2*a), throw 
        //         out any x values out of [0,1]
		
		
		d = sqrt(b*b - 4*a*(c-value));
	} else { /* XSYNTH_PORT_TYPE_LOGARITHMIC */
        value = xpd->a * exp(xpd->c * cval * log(xpd->b));
     }

    GDB_MESSAGE(GDB_GUI, " on_voice_slider_change: slider %d changed to %10.6f => %10.6f\n",
            index, GTK_ADJUSTMENT(widget)->value, value);
    lo_send(osc_host_address, osc_control_path, "if", index, value);
}

void
on_test_note_slider_change(GtkWidget *widget, gpointer data)
{
    unsigned char value = lrintf(GTK_ADJUSTMENT(widget)->value);

    if ((int)data == 0) {  /* key */

        test_note_noteon_key = value;
        GDB_MESSAGE(GDB_GUI, " on_test_note_slider_change: new test note key %d\n", test_note_noteon_key);

    } else {  /* velocity */

        test_note_velocity = value;
        GDB_MESSAGE(GDB_GUI, " on_test_note_slider_change: new test note velocity %d\n", test_note_velocity);

    }
}

void
on_test_note_button_press(GtkWidget *widget, gpointer data)
{
    unsigned char midi[4];

    if ((int)data) {  /* button pressed */

        midi[0] = 0;
        midi[1] = 0x90;
        midi[2] = test_note_noteon_key;
        midi[3] = test_note_velocity;
        lo_send(osc_host_address, osc_midi_path, "m", midi);
        test_note_noteoff_key = test_note_noteon_key;

    } else { /* button released */

        midi[0] = 0;
        midi[1] = 0x80;
        midi[2] = test_note_noteoff_key;
        midi[3] = 0x40;
        lo_send(osc_host_address, osc_midi_path, "m", midi);

    }
}

void
on_polyphony_change(GtkWidget *widget, gpointer data)
{
    int polyphony = lrintf(GTK_ADJUSTMENT(widget)->value);
    char buffer[4];
    
    if (internal_gui_update_only) {
        /* GUIDB_MESSAGE(DB_GUI, " on_polyphony_change: skipping further action\n"); */
        return;
    }

    GDB_MESSAGE(GDB_GUI, " on_polyphony_change: polyphony set to %d\n", polyphony);

    snprintf(buffer, 4, "%d", polyphony);
    lo_send(osc_host_address, osc_configure_path, "ss", "polyphony", buffer);
}

void
update_voice_widget(int port, float value)
{
    struct nekobee_port_descriptor *xpd;
    GtkAdjustment *adj;
    float cval, a, b, c, d;
	
	
    if (port < XSYNTH_PORT_WAVEFORM || port >= XSYNTH_PORTS_COUNT) {
        return;
    }

    xpd = &nekobee_port_description[port];
	
	if (value < xpd->lower_bound)
        value = xpd->lower_bound;
    else if (value > xpd->upper_bound)
        value = xpd->upper_bound;
    
    internal_gui_update_only = 1;

    switch (xpd->type) {

      case XSYNTH_PORT_TYPE_LINEAR:
		  a=xpd->a; b=xpd->b; c=xpd->c;
		  // used in "on_voice_slider_change" value = (xpd->a * cval + xpd->b) * cval + xpd->c;
	  d = sqrt(b*b - 4*a*(c-value));
	  	cval=(-b+d)/(2*a);
        //cval = (value - xpd->c) / xpd->b;  /* assume xpd->a == 0, which was always true for nekobee */
        GDB_MESSAGE(GDB_GUI, " update_voice_widget: change of %s to %f => %f\n", xpd->name, value, cval); 	  
 
        adj = (GtkAdjustment *)voice_widget[port];
        adj->value = cval * 10.0f;
        /* gtk_signal_emit_by_name (GTK_OBJECT (adj), "changed");        does not cause call to on_voice_slider_change callback */
        gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");  /* causes call to on_voice_slider_change callback */
        break;

      case XSYNTH_PORT_TYPE_LOGARITHMIC:
	        cval = log(value / xpd->a) / (xpd->c * log(xpd->b));
	        if (port == XSYNTH_PORT_TUNING) {
            if (cval < -1.2f)	cval = -1.2f;
        	if (cval > 1.2f) cval = 1.2f;
        	} 
		else {
        if (cval > 1.0f - 1.0e-6f)
            cval = 1.0f;
	}
         GDB_MESSAGE(GDB_GUI, " update_voice_widget: change of %s to %f => %f\n", xpd->name, value, cval); 
        adj = (GtkAdjustment *)voice_widget[port];
        adj->value = cval * 10.0f;
        gtk_signal_emit_by_name (GTK_OBJECT (adj), "value_changed");  /* causes call to on_voice_slider_change callback */
        break;
      default:
			GDB_MESSAGE(GDB_GUI," updade_voice_widget: don't know what this is...\n");
        break;
		

    }

    internal_gui_update_only = 0;
}

void
update_voice_widgets_from_patch(nekobee_patch_t *patch)
{
    update_voice_widget(XSYNTH_PORT_TUNING, patch->tuning);
    update_voice_widget(XSYNTH_PORT_WAVEFORM, (float)patch->waveform);
    update_voice_widget(XSYNTH_PORT_CUTOFF, patch->cutoff);
    update_voice_widget(XSYNTH_PORT_RESONANCE, patch->resonance);
    update_voice_widget(XSYNTH_PORT_ENVMOD, patch->envmod);
    update_voice_widget(XSYNTH_PORT_DECAY, patch->decay);
    update_voice_widget(XSYNTH_PORT_ACCENT, patch->accent);
    update_voice_widget(XSYNTH_PORT_VOLUME, patch->volume);
}

static float
get_value_from_slider(int index)
{
    struct nekobee_port_descriptor *xpd = &nekobee_port_description[index];
    float cval = GTK_ADJUSTMENT(voice_widget[index])->value / 10.0f;

    if (xpd->type == XSYNTH_PORT_TYPE_LINEAR) {

        return (xpd->a * cval + xpd->b) * cval + xpd->c;  /* linear or quadratic */

    } else { /* XSYNTH_PORT_TYPE_LOGARITHMIC */

        return xpd->a * exp(xpd->c * cval * log(xpd->b));

    }
}

static unsigned char
get_value_from_detent(int index)
{
    return lrintf(GTK_ADJUSTMENT(voice_widget[index])->value);
}

void
update_patch_from_voice_widgets(nekobee_patch_t *patch)
{
    patch->tuning           = get_value_from_slider(XSYNTH_PORT_TUNING);        
    patch->waveform        = get_value_from_detent(XSYNTH_PORT_WAVEFORM);     
    patch->cutoff      = get_value_from_slider(XSYNTH_PORT_CUTOFF);   
    patch->resonance           = get_value_from_slider(XSYNTH_PORT_RESONANCE);        
    patch->envmod          = get_value_from_slider(XSYNTH_PORT_ENVMOD);
	patch->decay      = get_value_from_slider(XSYNTH_PORT_DECAY);   
    patch->accent        = get_value_from_slider(XSYNTH_PORT_ACCENT);
    patch->volume               = get_value_from_slider(XSYNTH_PORT_VOLUME);

}
