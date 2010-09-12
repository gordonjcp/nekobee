/*  nekosynth cairo knobs
    
    Copyright (C) 2010 Gordon JC Pearce <gordon@gjcp.net>
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/


#ifndef __NEKO_KNOB_H
#define __NEKO_KNOB_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _nekoKnob            nekoKnob;
typedef struct _nekoKnobClass       nekoKnobClass;
typedef struct _nekoKnobPrivate     nekoKnobPrivate;

struct _nekoKnob {
    GtkDrawingArea parent;
    GtkAdjustment *adjustment;
    cairo_surface_t *pixbuf;
    guint8 state;
    gint saved_x, saved_y;

};

struct _nekoKnobClass {
    GtkDrawingAreaClass parent_class;
};

struct _nekoKnobPrivate {
};

#define NEKO_TYPE_KNOB             (neko_knob_get_type ())
#define NEKO_KNOB(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), NEKO_TYPE_KNOB, nekoKnob))
#define NEKO_KNOB_CLASS(obj)       (G_TYPE_CHECK_CLASS_CAST ((obj), NEKO_KNOB,  nekoKnobClass))
#define NEKO_IS_KNOB(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NEKO_TYPE_KNOB))
#define NEKO_IS_KNOB_CLASS(obj)    (G_TYPE_CHECK_CLASS_TYPE ((obj), NEKO_TYPE_KNOB))
#define NEKO_KNOB_GET_CLASS        (G_TYPE_INSTANCE_GET_CLASS ((obj), NEKO_TYPE_KNOB, nekoKnobClass))
#define NEKO_KNOB_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NEKO_TYPE_KNOB, nekoKnobPrivate))

G_END_DECLS

GtkWidget *neko_knob_new();

#define STATE_IDLE		0
#define STATE_PRESSED		1
#define STATE_DRAGGING		2

#endif /* __NEKO_KNOB_H */


