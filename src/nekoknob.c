/*  nekosynth cairo knobs
    (C) 2010 Gordon JC Pearce

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#include <gtk/gtk.h>
#include "config.h"
#include "nekoknob.h"

#define KNOB_SIZE 50

static GtkWidgetClass *parent_class = NULL;
G_DEFINE_TYPE (nekoKnob, neko_knob, GTK_TYPE_DRAWING_AREA);

static void neko_knob_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void neko_knob_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static gboolean neko_knob_expose(GtkWidget *widget, GdkEventExpose *event);
static void neko_knob_realize(GtkWidget *widget);
static gint neko_knob_button_press(GtkWidget *widget, GdkEventButton *event);
static gint neko_knob_button_release(GtkWidget *widget, GdkEventButton *event);
static gint neko_knob_motion_notify(GtkWidget *widget, GdkEventMotion *event);

static void neko_knob_class_init (nekoKnobClass *class) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);
    GObjectClass *gobject_class = G_OBJECT_CLASS (class);
    parent_class = gtk_type_class(GTK_TYPE_DRAWING_AREA);
    widget_class->size_request = neko_knob_size_request;
    widget_class->size_allocate = neko_knob_size_allocate;
    widget_class->expose_event = neko_knob_expose;
    widget_class->realize = neko_knob_realize;
    widget_class->button_press_event = neko_knob_button_press;
    widget_class->button_release_event = neko_knob_button_release;
  widget_class->motion_notify_event = neko_knob_motion_notify;
}


static gint neko_knob_button_press(GtkWidget *widget, GdkEventButton *event) {
  nekoKnob *knob;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(NEKO_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  knob = NEKO_KNOB(widget);

  switch (knob->state) {
    case STATE_IDLE:
	  //gtk_grab_add(widget);
	  knob->state = STATE_PRESSED;
	  knob->saved_x = event->x;
	  knob->saved_y = event->y;
	  break;
    default:
      break;
  }

  return FALSE;
}
static gint neko_knob_button_release(GtkWidget *widget, GdkEventButton *event) {
  nekoKnob *knob;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(NEKO_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  knob = NEKO_KNOB(widget);

  knob->state = STATE_IDLE;

  return FALSE;
}

static void neko_knob_update_mouse(nekoKnob *knob, gdouble x, gdouble y,
                                  gboolean absolute)
{
    gdouble old_value, new_value, dv, dh;
    gdouble angle;

    g_return_if_fail(knob != NULL);
    g_return_if_fail(NEKO_IS_KNOB(knob));

    old_value = knob->adjustment->value;

    dv = knob->saved_y - y; // inverted cartesian graphics coordinate system
    printf("%f\n", dv);

        dh = x - knob->saved_x;
        knob->saved_x = x;
        knob->saved_y = y;
/*
        if (x >= 0 && x <= KNOB_SIZE)
            dh = 0;  // dead zone 
        else {
            angle = cos(angle);
            dh *= angle * angle;
        }
*/
        new_value = knob->adjustment->value +
                    dh * knob->adjustment->step_increment +
                    dv * (knob->adjustment->upper -
                          knob->adjustment->lower) / 200.0f;
    

    new_value = MAX(MIN(new_value, knob->adjustment->upper),
                    knob->adjustment->lower);

    knob->adjustment->value = new_value;

    gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");

}

static gint neko_knob_motion_notify(GtkWidget *widget, GdkEventMotion *event) {
    nekoKnob *knob;
    gdouble x, y;

    g_return_val_if_fail(widget != NULL, FALSE);
    g_return_val_if_fail(NEKO_IS_KNOB(widget), FALSE);
    g_return_val_if_fail(event != NULL, FALSE);

    knob = NEKO_KNOB(widget);

    x = event->x;
    y = event->y;

    if (knob->state == STATE_PRESSED) neko_knob_update_mouse(knob, x, y, TRUE);   // coarse

    return FALSE;
}



static void neko_knob_adjustment_value_changed (GtkAdjustment *adjustment, gpointer data) {
  nekoKnob *knob;

  g_return_if_fail(adjustment != NULL);
  g_return_if_fail(data != NULL);

  knob = NEKO_KNOB(data);
  gtk_widget_queue_draw(GTK_WIDGET(knob));
}

void neko_knob_set_adjustment(nekoKnob *knob, GtkAdjustment *adjustment) {
  g_return_if_fail (knob != NULL);
  g_return_if_fail (NEKO_IS_KNOB (knob));

  if (knob->adjustment) {
    gtk_signal_disconnect_by_data(GTK_OBJECT(knob->adjustment), (gpointer)knob);
    gtk_object_unref(GTK_OBJECT(knob->adjustment));
  }

  knob->adjustment = adjustment;
  gtk_object_ref(GTK_OBJECT(knob->adjustment));
  gtk_object_sink(GTK_OBJECT( knob->adjustment ) ); 

/* FIXME
  gtk_signal_connect(GTK_OBJECT(adjustment), "changed",
		     GTK_SIGNAL_FUNC(gtk_knob_adjustment_changed), (gpointer) knob);
		     */
  gtk_signal_connect(GTK_OBJECT(adjustment), "value_changed",
		     GTK_SIGNAL_FUNC(neko_knob_adjustment_value_changed), (gpointer) knob);

  //knob->old_value = adjustment->value;
  //knob->old_lower = adjustment->lower;
  //knob->old_upper = adjustment->upper;

    gtk_widget_queue_draw(GTK_WIDGET(knob));
}

static void neko_knob_size_request (GtkWidget *widget, GtkRequisition *requisition) {
    // FIXME hardcoded
  requisition->width = KNOB_SIZE;
  requisition->height = KNOB_SIZE;
}

static void neko_knob_size_allocate (GtkWidget *widget, GtkAllocation *allocation) {
  nekoKnob *knob;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(NEKO_IS_KNOB(widget));
  g_return_if_fail(allocation != NULL);

  widget->allocation = *allocation;
  knob = NEKO_KNOB(widget);

  if (GTK_WIDGET_REALIZED(widget)) {
    gdk_window_move_resize(widget->window,
			   allocation->x, allocation->y,
			   allocation->width, allocation->height);
  }
}

GtkWidget *neko_knob_new(GtkAdjustment *adjustment) {
    nekoKnob *knob;
    knob = g_object_new(NEKO_TYPE_KNOB, NULL);
    if (!adjustment)
        adjustment = (GtkAdjustment *) gtk_adjustment_new(0, 0, 0, 0, 0, 0);
    neko_knob_set_adjustment(knob, adjustment);
    
    return GTK_WIDGET(knob);
}

static void neko_knob_init(nekoKnob *knob) {
    gtk_widget_set_events(GTK_WIDGET(knob), GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
}

static void neko_knob_realize(GtkWidget *widget) {
  nekoKnob *knob;
  g_return_if_fail(widget != NULL);
  g_return_if_fail(NEKO_IS_KNOB(widget));
  GTK_WIDGET_CLASS(parent_class)->realize(widget);

  knob = NEKO_KNOB(widget);
  // FIXME hardcoded
  knob->pixbuf = cairo_image_surface_create_from_png(INSTALL_DIR"/knob.png");
}

static gboolean neko_knob_expose(GtkWidget *widget, GdkEventExpose *event) {
  gfloat dx, dy, throw;
  
    nekoKnob *knob = NEKO_KNOB(widget);

    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));

  dx = knob->adjustment->value - knob->adjustment->lower;	// value, from 0
  dy = knob->adjustment->upper - knob->adjustment->lower;	// range
  
  if (knob->adjustment->step_increment != 1.0f) {
	  dx=(int)(51*dx/dy)*50;
  } else {
	  throw=4;
	  dx=(int)(51*dx/throw+(24-throw))*50;
  }



    // FIXME hardcoded
    cairo_rectangle(cr, 0, 0, KNOB_SIZE, KNOB_SIZE);
    cairo_clip(cr);
    // FIXME totally wrong
    cairo_set_source_surface(cr, knob->pixbuf, -dx, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
}
