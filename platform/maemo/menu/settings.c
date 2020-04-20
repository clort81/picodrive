/*
* This file is part of PicoDrive
*
* Copyright (C) 2010 javicq
*
* This software is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation; either version 2.1 of
* the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#include <string.h>
#include <gtk/gtk.h>
#include <hildon/hildon-helper.h>

#include <hildon/hildon-gtk.h>
#include <hildon/hildon-pannable-area.h>
#include <hildon/hildon-button.h>
#include <hildon/hildon-check-button.h>
#include <hildon/hildon-picker-button.h>
#include <hildon/hildon-touch-selector.h>
#include <pango/pango-attributes.h>

#include "plugin.h"
#include "gconf.h"
#include "i18n.h"

static GtkDialog* dialog;
static HildonCheckButton* saver_check;
static HildonPickerButton* scaler_picker;
static HildonPickerButton* framerate_picker;
static HildonCheckButton* display_fps_check;
static HildonCheckButton* turbo_check;

static void fill_scaler_list(GtkWidget* w)
{
}

static void load_settings()
{
	int scaler_num = gconf_client_get_int(gcc, kGConfScaler, NULL);

	hildon_check_button_set_active(saver_check,
		gconf_client_get_bool(gcc, kGConfSaver, NULL));
	hildon_picker_button_set_active(scaler_picker, scaler_num);
	hildon_picker_button_set_active(framerate_picker,
		gconf_client_get_int(gcc, kGConfFrameskip, NULL));
	hildon_check_button_set_active(display_fps_check,
		gconf_client_get_bool(gcc, kGConfDisplayFramerate, NULL));
	//hildon_check_button_set_active(turbo_check,
		//gconf_client_get_bool(gcc, kGConfTurboMode, NULL));
}

static void save_settings()
{
	gconf_client_set_bool(gcc, kGConfSaver,
		hildon_check_button_get_active(saver_check), NULL);
	gconf_client_set_int(gcc, kGConfScaler, hildon_picker_button_get_active(scaler_picker), NULL);
	gconf_client_set_bool(gcc, kGConfDisplayFramerate,
		hildon_check_button_get_active(display_fps_check), NULL);
	gconf_client_set_int(gcc, kGConfFrameskip,
		hildon_picker_button_get_active(framerate_picker), NULL);
	//gconf_client_set_bool(gcc, kGConfTurboMode,
		//hildon_check_button_get_active(turbo_check), NULL);
}


void settings_update_controls(int player)
{
	/*switch (player) {
		case 1:
			hildon_button_set_value(player1_btn, controls_describe(1));
			break;
		case 2:
			hildon_button_set_value(player2_btn, controls_describe(2));
			break;
	}*/
}

static void cb_dialog_response(GtkWidget * button, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_OK) {
		save_settings();
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void set_button_layout(HildonButton* button,
 GtkSizeGroup* titles_size_group, GtkSizeGroup* values_size_group)
{
	hildon_button_add_title_size_group(button, titles_size_group);
	hildon_button_add_value_size_group(button, values_size_group);
	hildon_button_set_alignment(button, 0.0, 0.5, 1.0, 0.0);
}

void settings_dialog(GtkWindow* parent)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Settings"),
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));

	GtkBox * box = GTK_BOX(gtk_vbox_new(FALSE, HILDON_MARGIN_HALF));
	HildonPannableArea * pannable =
		HILDON_PANNABLE_AREA(hildon_pannable_area_new());
	GtkSizeGroup * titles_size_group =
		 gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	GtkSizeGroup * values_size_group =
		 gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	PangoAttrList *pattrlist = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new(22 * PANGO_SCALE);
	attr->start_index = 0;
	attr->end_index = G_MAXINT;
	pango_attr_list_insert(pattrlist, attr);
/*
	GtkLabel* separator_1 = GTK_LABEL(gtk_label_new(_("Controls")));
	gtk_label_set_attributes(separator_1, pattrlist);
	gtk_label_set_justify(separator_1, GTK_JUSTIFY_CENTER);

	player1_btn = HILDON_BUTTON(hildon_button_new_with_text(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
		_("Player 1"), NULL));
	set_button_layout(HILDON_BUTTON(player1_btn),
		titles_size_group, values_size_group);
	g_signal_connect(G_OBJECT(player1_btn), "clicked",
					G_CALLBACK(controls_btn_callback), GINT_TO_POINTER(1));

	player2_btn = HILDON_BUTTON(hildon_button_new_with_text(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
		_("Player 2"), NULL));
	set_button_layout(HILDON_BUTTON(player2_btn),
		titles_size_group, values_size_group);
	g_signal_connect(G_OBJECT(player2_btn), "clicked",
					G_CALLBACK(controls_btn_callback), GINT_TO_POINTER(2));

	GtkLabel* separator_2 = GTK_LABEL(gtk_label_new(_("Advanced")));
	gtk_label_set_attributes(separator_2, pattrlist);
	gtk_label_set_justify(separator_2, GTK_JUSTIFY_CENTER);

	accu_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(accu_check), _("Accurate graphics"));
	set_button_layout(HILDON_BUTTON(accu_check),
		titles_size_group, values_size_group);
*/

	framerate_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
	hildon_button_set_title(HILDON_BUTTON(framerate_picker),
		_("Frameskip"));
	set_button_layout(HILDON_BUTTON(framerate_picker),
		titles_size_group, values_size_group);

	HildonTouchSelector* framerate_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	int i;
	for ( i = -1; i < 9; i++) {
		gchar buffer[20];
		sprintf(buffer, "Skip %d", i);
		hildon_touch_selector_append_text(framerate_sel, i == -1 ? "Auto" : i == 0 ? "Disabled" : buffer);
	}
	hildon_picker_button_set_selector(framerate_picker, framerate_sel);
	GtkBox* framerate_sel_box = GTK_BOX(gtk_hbox_new(FALSE, HILDON_MARGIN_DEFAULT));

	display_fps_check =
		HILDON_CHECK_BUTTON(hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(display_fps_check),
		_("Show FPS"));
	/*turbo_check =
		HILDON_CHECK_BUTTON(hildon_check_button_new(HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(turbo_check),
		_("Turbo mode"));*/

	gtk_box_pack_start_defaults(framerate_sel_box, GTK_WIDGET(display_fps_check));
	//gtk_box_pack_start_defaults(framerate_sel_box, GTK_WIDGET(turbo_check));
	gtk_box_pack_start(GTK_BOX(framerate_sel), GTK_WIDGET(framerate_sel_box), FALSE, FALSE, 0);
	gtk_widget_show_all(GTK_WIDGET(framerate_sel_box));


	saver_check = HILDON_CHECK_BUTTON(hildon_check_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
	gtk_button_set_label(GTK_BUTTON(saver_check),
		_("Pause on power saving"));
	set_button_layout(HILDON_BUTTON(saver_check),
		titles_size_group, values_size_group);

	scaler_picker = HILDON_PICKER_BUTTON(hildon_picker_button_new(
		HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
		HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
	hildon_button_set_title(HILDON_BUTTON(scaler_picker), _("Screen scaling"));
	set_button_layout(HILDON_BUTTON(scaler_picker),
		titles_size_group, values_size_group);

	HildonTouchSelector* scaler_sel =
		HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
	hildon_touch_selector_append_text(scaler_sel,
			_("Pixel doubling"));
	hildon_touch_selector_append_text(scaler_sel,
			_("Original size"));
	hildon_touch_selector_append_text(scaler_sel,
			_("Stretch to fill"));
	hildon_picker_button_set_selector(scaler_picker, scaler_sel);

//	gtk_box_pack_start(box, GTK_WIDGET(accu_check),
	//	FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(framerate_picker),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(scaler_picker),
		FALSE, FALSE, 0);
	gtk_box_pack_start(box, GTK_WIDGET(saver_check),
		FALSE, FALSE, 0);
//	gtk_box_pack_start(box, GTK_WIDGET(speedhacks_picker),
	//	FALSE, FALSE, 0);

	hildon_pannable_area_add_with_viewport(pannable, GTK_WIDGET(box));
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(pannable));

	pango_attr_list_unref(pattrlist);
	g_object_unref(titles_size_group);
	g_object_unref(values_size_group);

	load_settings();

	gtk_window_resize(GTK_WINDOW(dialog), 800, 380);

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);

	gtk_widget_show_all(GTK_WIDGET(dialog));
}
