/* nekobee DSSI software synthesizer GUI
 *
 * Most of this code comes from gAlan 0.2.0, copyright (C) 1999
 * Tony Garnock-Jones, with modifications by Sean Bolton,
 * copyright (c) 2004.  (gtkdial.c rolls over in its grave.)
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

#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "config.h"
#include "gtkknob.h"

#ifndef M_PI
# define M_PI		3.14159265358979323846	/* pi */
#endif

#define SCROLL_DELAY_LENGTH	300
#define KNOB_SIZE		50

#define STATE_IDLE		0
#define STATE_PRESSED		1
#define STATE_DRAGGING		2

static void gtk_knob_class_init(GtkKnobClass *klass);
static void gtk_knob_init(GtkKnob *knob);
static void gtk_knob_destroy(GtkObject *object);
static void gtk_knob_realize(GtkWidget *widget);
static void gtk_knob_size_request(GtkWidget *widget, GtkRequisition *requisition);
static void gtk_knob_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_knob_expose(GtkWidget *widget, GdkEventExpose *event);
static gint gtk_knob_button_press(GtkWidget *widget, GdkEventButton *event);
static gint gtk_knob_button_release(GtkWidget *widget, GdkEventButton *event);
static gint gtk_knob_motion_notify(GtkWidget *widget, GdkEventMotion *event);
static gint gtk_knob_timer(GtkKnob *knob);

static void gtk_knob_update_mouse_update(GtkKnob *knob);
static void gtk_knob_update_mouse(GtkKnob *knob, gint x, gint y, gboolean absolute);
static void gtk_knob_update(GtkKnob *knob);
static void gtk_knob_adjustment_changed(GtkAdjustment *adjustment, gpointer data);
static void gtk_knob_adjustment_value_changed(GtkAdjustment *adjustment, gpointer data);

GError *gerror;

/* Local data */

static GtkWidgetClass *parent_class = NULL;

guint gtk_knob_get_type(void) {
  static guint knob_type = 0;

  if (!knob_type) {
    GtkTypeInfo knob_info = {
      "GtkKnob",
      sizeof (GtkKnob),
      sizeof (GtkKnobClass),
      (GtkClassInitFunc) gtk_knob_class_init,
      (GtkObjectInitFunc) gtk_knob_init,
      NULL,
      NULL,
    };

    knob_type = gtk_type_unique(gtk_widget_get_type(), &knob_info);
  }

  return knob_type;
}

static void gtk_knob_class_init (GtkKnobClass *class) {
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = gtk_type_class(gtk_widget_get_type());

  object_class->destroy = gtk_knob_destroy;

  widget_class->realize = gtk_knob_realize;
  widget_class->expose_event = gtk_knob_expose;
  widget_class->size_request = gtk_knob_size_request;
  widget_class->size_allocate = gtk_knob_size_allocate;
  widget_class->button_press_event = gtk_knob_button_press;
  widget_class->button_release_event = gtk_knob_button_release;
  widget_class->motion_notify_event = gtk_knob_motion_notify;
}

static void gtk_knob_init (GtkKnob *knob) {
  knob->policy = GTK_UPDATE_CONTINUOUS;
  knob->state = STATE_IDLE;
  knob->saved_x = knob->saved_y = 0;
  knob->timer = 0;
  knob->pixbuf = NULL;
  knob->mask = NULL;
  knob->mask_gc = NULL;
  knob->red_gc = NULL;
  knob->old_value = 0.0;
  knob->old_lower = 0.0;
  knob->old_upper = 0.0;
  knob->adjustment = NULL;
}

GtkWidget *gtk_knob_new(GtkAdjustment *adjustment) {
  GtkKnob *knob;

  knob = gtk_type_new(gtk_knob_get_type());

  if (!adjustment)
    adjustment = (GtkAdjustment*) gtk_adjustment_new(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

  gtk_knob_set_adjustment(knob, adjustment);

  return GTK_WIDGET(knob);
}

static void gtk_knob_destroy(GtkObject *object) {
  GtkKnob *knob;

  g_return_if_fail(object != NULL);
  g_return_if_fail(GTK_IS_KNOB(object));

  knob = GTK_KNOB(object);

  if (knob->adjustment) {
    gtk_object_unref(GTK_OBJECT(knob->adjustment));
    knob->adjustment = NULL;
  }

  if (knob->pixbuf) {
    gdk_pixbuf_unref(knob->pixbuf);
    knob->pixbuf = NULL;
  }

  if (knob->mask) {
    gdk_bitmap_unref(knob->mask);
    knob->mask = NULL;
  }

  if (knob->mask_gc) {
    gdk_gc_unref(knob->mask_gc);
    knob->mask_gc = NULL;
  }
  if (knob->red_gc) {
    gdk_gc_unref(knob->red_gc);
    knob->red_gc = NULL;
  }

  if (GTK_OBJECT_CLASS(parent_class)->destroy)
    (*GTK_OBJECT_CLASS(parent_class)->destroy)(object);
}

GtkAdjustment* gtk_knob_get_adjustment(GtkKnob *knob) {
  g_return_val_if_fail(knob != NULL, NULL);
  g_return_val_if_fail(GTK_IS_KNOB(knob), NULL);

  return knob->adjustment;
}

void gtk_knob_set_update_policy(GtkKnob *knob, GtkUpdateType policy) {
  g_return_if_fail (knob != NULL);
  g_return_if_fail (GTK_IS_KNOB (knob));

  knob->policy = policy;
}

void gtk_knob_set_adjustment(GtkKnob *knob, GtkAdjustment *adjustment) {
  g_return_if_fail (knob != NULL);
  g_return_if_fail (GTK_IS_KNOB (knob));

  if (knob->adjustment) {
    gtk_signal_disconnect_by_data(GTK_OBJECT(knob->adjustment), (gpointer)knob);
    gtk_object_unref(GTK_OBJECT(knob->adjustment));
  }

  knob->adjustment = adjustment;
  gtk_object_ref(GTK_OBJECT(knob->adjustment));
  gtk_object_sink(GTK_OBJECT( knob->adjustment ) ); 

  gtk_signal_connect(GTK_OBJECT(adjustment), "changed",
		     GTK_SIGNAL_FUNC(gtk_knob_adjustment_changed), (gpointer) knob);
  gtk_signal_connect(GTK_OBJECT(adjustment), "value_changed",
		     GTK_SIGNAL_FUNC(gtk_knob_adjustment_value_changed), (gpointer) knob);

  knob->old_value = adjustment->value;
  knob->old_lower = adjustment->lower;
  knob->old_upper = adjustment->upper;

  gtk_knob_update(knob);
}

static void gtk_knob_realize(GtkWidget *widget) {
  GtkKnob *knob;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GdkColor color = { 0, 0xffff, 0, 0 };

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_KNOB(widget));

  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
  knob = GTK_KNOB(widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask =
    gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual(widget);
  attributes.colormap = gtk_widget_get_colormap(widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new(widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach(widget->parent->style, widget->window);

  gdk_window_set_user_data(widget->window, widget);

  knob->pixbuf = gdk_pixbuf_new_from_file(INSTALL_DIR"/knob.png",&gerror);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

  knob->mask_gc = gdk_gc_new(widget->window);
  gdk_gc_copy(knob->mask_gc, widget->style->bg_gc[GTK_STATE_NORMAL]);
  gdk_gc_set_clip_mask(knob->mask_gc, knob->mask);

  knob->red_gc = gdk_gc_new(widget->window);
  gdk_gc_copy(knob->red_gc, widget->style->bg_gc[GTK_STATE_NORMAL]);
  gdk_colormap_alloc_color(attributes.colormap, &color, FALSE, TRUE);
  gdk_gc_set_foreground(knob->red_gc, &color);
}

static void gtk_knob_size_request (GtkWidget *widget, GtkRequisition *requisition) {
  requisition->width = KNOB_SIZE;
  requisition->height = KNOB_SIZE;
}

static void gtk_knob_size_allocate (GtkWidget *widget, GtkAllocation *allocation) {
  GtkKnob *knob;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_KNOB(widget));
  g_return_if_fail(allocation != NULL);

  widget->allocation = *allocation;
  knob = GTK_KNOB(widget);

  if (GTK_WIDGET_REALIZED(widget)) {
    gdk_window_move_resize(widget->window,
			   allocation->x, allocation->y,
			   allocation->width, allocation->height);
  }
}

static gint gtk_knob_expose(GtkWidget *widget, GdkEventExpose *event) {
  GtkKnob *knob;
  gfloat dx, dy, throw;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;
        
  knob = GTK_KNOB(widget);

  // FIXME - somewhere in here, we need to read this from the knob size

  // basically we need to work out if the step size is integer
  // if it is, centre the knob about the vertical
  
  dx = knob->adjustment->value - knob->adjustment->lower;	// value, from 0
  dy = knob->adjustment->upper - knob->adjustment->lower;	// range

  
  if (knob->adjustment->step_increment != 1.0f) {
	  dx=(int)(51*dx/dy)*50;
  } else {
	  throw=4;
	  dx=(int)(51*dx/throw+(24-throw))*50;
  }
  
  
  
  gdk_draw_pixbuf(widget->window, knob->mask_gc, knob->pixbuf,
                  dx, 0, 0, 0, KNOB_SIZE, KNOB_SIZE,GDK_RGB_DITHER_NONE,0,0);


  return FALSE;
}

static gint gtk_knob_button_press(GtkWidget *widget, GdkEventButton *event) {
  GtkKnob *knob;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  knob = GTK_KNOB(widget);

  switch (knob->state) {
    case STATE_IDLE:
      switch (event->button) {
	case 1:
	case 3:
	  gtk_grab_add(widget);
	  knob->state = STATE_PRESSED;
	  knob->saved_x = event->x;
	  knob->saved_y = event->y;
	  break;

	default:
	  break;
      }
      break;

    default:
      break;
  }

  return FALSE;
}

static gint gtk_knob_button_release(GtkWidget *widget, GdkEventButton *event) {
  GtkKnob *knob;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  knob = GTK_KNOB(widget);

  switch (knob->state) {
    case STATE_PRESSED:
      gtk_grab_remove(widget);
      knob->state = STATE_IDLE;

      switch (event->button) {
	case 1:
	  knob->adjustment->value -= knob->adjustment->page_increment;
	  gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");
	  break;

	case 3:
	  knob->adjustment->value += knob->adjustment->page_increment;
	  gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");
	  break;

	default:
	  break;
      }
      break;

    case STATE_DRAGGING:
      gtk_grab_remove(widget);
      knob->state = STATE_IDLE;

      if (knob->policy != GTK_UPDATE_CONTINUOUS && knob->old_value != knob->adjustment->value)
	gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");

      break;

    default:
      break;
  }

  return FALSE;
}

static gint gtk_knob_motion_notify(GtkWidget *widget, GdkEventMotion *event) {
  GtkKnob *knob;
  GdkModifierType mods;
  gint x, y;

  g_return_val_if_fail(widget != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_KNOB(widget), FALSE);
  g_return_val_if_fail(event != NULL, FALSE);

  knob = GTK_KNOB(widget);

  x = event->x;
  y = event->y;

  if (event->is_hint || (event->window != widget->window))
    gdk_window_get_pointer(widget->window, &x, &y, &mods);

  switch (knob->state) {
    case STATE_PRESSED:
      knob->state = STATE_DRAGGING;
      /* fall through */

    case STATE_DRAGGING:
      if (mods & GDK_BUTTON1_MASK) {
	gtk_knob_update_mouse(knob, x, y, TRUE);
	return TRUE;
      } else if (mods & GDK_BUTTON3_MASK) {
	gtk_knob_update_mouse(knob, x, y, FALSE);
	return TRUE;
      }
      break;

    default:
      break;
  }

  return FALSE;
}

static gint gtk_knob_timer(GtkKnob *knob) {
  g_return_val_if_fail(knob != NULL, FALSE);
  g_return_val_if_fail(GTK_IS_KNOB(knob), FALSE);

  if (knob->policy == GTK_UPDATE_DELAYED)
    gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");

  return FALSE;	/* don't keep running this timer */
}

static void gtk_knob_update_mouse_update(GtkKnob *knob) {
  if (knob->policy == GTK_UPDATE_CONTINUOUS)
    gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");
  else {
    gtk_widget_draw(GTK_WIDGET(knob), NULL);

    if (knob->policy == GTK_UPDATE_DELAYED) {
      if (knob->timer)
	gtk_timeout_remove(knob->timer);

      knob->timer = gtk_timeout_add (SCROLL_DELAY_LENGTH, (GtkFunction) gtk_knob_timer,
				     (gpointer) knob);
    }
  }
}

static void gtk_knob_update_mouse(GtkKnob *knob, gint x, gint y,
                                  gboolean absolute)
{
    gfloat old_value, new_value, dv, dh;
    gdouble angle;

    g_return_if_fail(knob != NULL);
    g_return_if_fail(GTK_IS_KNOB(knob));

    old_value = knob->adjustment->value;

    angle = atan2(-y + (KNOB_SIZE>>1), x - (KNOB_SIZE>>1));

    if (absolute) {

        angle /= M_PI;
        if (angle < -0.5)
            angle += 2;

        new_value = -(2.0/3.0) * (angle - 1.25);   /* map [1.25pi, -0.25pi] onto [0, 1] */
        new_value *= knob->adjustment->upper - knob->adjustment->lower;
        new_value += knob->adjustment->lower;

    } else {

        dv = knob->saved_y - y; /* inverted cartesian graphics coordinate system */
        dh = x - knob->saved_x;
        knob->saved_x = x;
        knob->saved_y = y;

        if (x >= 0 && x <= KNOB_SIZE)
            dh = 0;  /* dead zone */
        else {
            angle = cos(angle);
            dh *= angle * angle;
        }

        new_value = knob->adjustment->value +
                    dv * knob->adjustment->step_increment +
                    dh * (knob->adjustment->upper -
                          knob->adjustment->lower) / 200.0f;
    }

    new_value = MAX(MIN(new_value, knob->adjustment->upper),
                    knob->adjustment->lower);

    knob->adjustment->value = new_value;

    if (knob->adjustment->value != old_value)
        gtk_knob_update_mouse_update(knob);
}

static void gtk_knob_update(GtkKnob *knob) {
  gfloat new_value;
        
  g_return_if_fail(knob != NULL);
  g_return_if_fail(GTK_IS_KNOB (knob));

	// gjcp
	
  new_value = knob->adjustment->value;
  if (knob->adjustment->step_increment == 1) new_value = (int)(knob->adjustment->value+0.5);
        
  if (new_value < knob->adjustment->lower)
    new_value = knob->adjustment->lower;

  if (new_value > knob->adjustment->upper)
    new_value = knob->adjustment->upper;

  if (new_value != knob->adjustment->value) {
    knob->adjustment->value = new_value;
    gtk_signal_emit_by_name(GTK_OBJECT(knob->adjustment), "value_changed");
  }

  gtk_widget_draw(GTK_WIDGET(knob), NULL);
}

static void gtk_knob_adjustment_changed(GtkAdjustment *adjustment, gpointer data) {
  GtkKnob *knob;

  g_return_if_fail(adjustment != NULL);
  g_return_if_fail(data != NULL);

  knob = GTK_KNOB(data);

  if ((knob->old_value != adjustment->value) ||
      (knob->old_lower != adjustment->lower) ||
      (knob->old_upper != adjustment->upper)) {
    gtk_knob_update (knob);

    knob->old_value = adjustment->value;
    knob->old_lower = adjustment->lower;
    knob->old_upper = adjustment->upper;
  }
}

static void gtk_knob_adjustment_value_changed (GtkAdjustment *adjustment, gpointer data) {
  GtkKnob *knob;

  g_return_if_fail(adjustment != NULL);
  g_return_if_fail(data != NULL);

  knob = GTK_KNOB(data);

  if (knob->old_value != adjustment->value) {
    gtk_knob_update (knob);

    knob->old_value = adjustment->value;
  }
}
