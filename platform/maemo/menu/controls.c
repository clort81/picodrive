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

#define NUM_PLAYERS 2
#define MAEMO_VERSION 5

static GtkDialog* dialog;
static HildonCheckButton* keys_chk[NUM_PLAYERS];
static HildonButton* keys_btn[NUM_PLAYERS];
static HildonCheckButton* touch_chk[NUM_PLAYERS];
static HildonCheckButton* touch_show_chk[NUM_PLAYERS];
static HildonCheckButton* accel_chk[NUM_PLAYERS];
static HildonPickerButton* accel_sens[NUM_PLAYERS];

static void load_settings()
{
	int player;
	for( player = 0; player < NUM_PLAYERS; player++ ) {
		gchar key_base[kGConfPlayerPathBufferLen];
		const int key_len = sprintf(key_base, kGConfPlayerPath, player+1);
		gchar *key = key_base + key_len;

		strcpy(key, kGConfPlayerKeyboardEnable);
		hildon_check_button_set_active(keys_chk[player],
			gconf_client_get_bool(gcc, key_base, NULL));

		strcpy(key, kGConfPlayerTouchscreenEnable);
		hildon_check_button_set_active(touch_chk[player],
			gconf_client_get_bool(gcc, key_base, NULL));
		strcpy(key, kGConfPlayerTouchscreenShow);
		hildon_check_button_set_active(touch_show_chk[player],
			gconf_client_get_bool(gcc, key_base, NULL));

		strcpy(key, kGConfPlayerAccelerometerEnable);
		hildon_check_button_set_active(accel_chk[player],
			gconf_client_get_bool(gcc, key_base, NULL));
		//strcpy(key, kGConfPlayerAccelerometerZero);
		//hildon_button_set_value(accel_zero,
			//gconf_client_get_string(gcc, key_base, NULL));
		strcpy(key, kGConfPlayerAccelerometerSensitivity);
		hildon_picker_button_set_active(accel_sens[player], gconf_client_get_int(gcc, key_base, NULL));
	}
}

static void save_settings()
{
	int player;
	for( player = 0; player < NUM_PLAYERS; player++ ) {
		gchar key_base[kGConfPlayerPathBufferLen];
		const int key_len = sprintf(key_base, kGConfPlayerPath, player+1);
		gchar *key = key_base + key_len;

		strcpy(key, kGConfPlayerKeyboardEnable);
		gconf_client_set_bool(gcc, key_base,
			hildon_check_button_get_active(keys_chk[player]), NULL);

		strcpy(key, kGConfPlayerTouchscreenEnable);
		gconf_client_set_bool(gcc, key_base,
			hildon_check_button_get_active(touch_chk[player]), NULL);
		strcpy(key, kGConfPlayerTouchscreenShow);
		gconf_client_set_bool(gcc, key_base,
			hildon_check_button_get_active(touch_show_chk[player]), NULL);

		strcpy(key, kGConfPlayerAccelerometerEnable);
		gconf_client_set_bool(gcc, key_base,
			hildon_check_button_get_active(accel_chk[player]), NULL);
	/*	strcpy(key, kGConfPlayerAccelerometerZero);
		gconf_client_set_string(gcc, key_base,
				hildon_button_get_value(accel_zero), NULL);*/
		strcpy(key, kGConfPlayerAccelerometerSensitivity);
		gconf_client_set_int(gcc, key_base,
				hildon_picker_button_get_active(accel_sens[player]), NULL);
	}
}

static void keys_btn_callback(GtkWidget * button, gpointer data)
{
	keys_dialog(GTK_WINDOW(dialog), GPOINTER_TO_INT(data));
}

static void accel_btn_callback( GtkWidget * button, gpointer data ) {
/*	int x, y, z;
	FILE* f = fopen( "/sys/class/i2c-adapter/i2c-3/3-001d/coord", "r" );
	if( !f ) return;
	fscanf( f, "%d %d %d", &x, &y, &z );
	fclose(f);
	gchar* zero[64];
	sprintf( zero, "%d %d %d", x, y, z );
	hildon_button_set_value(accel_zero,zero);*/
}

static void cb_dialog_response(GtkWidget * button, gint response, gpointer data)
{
	if (response == GTK_RESPONSE_OK) {
		save_settings();
		//settings_update_controls(current_player);
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

gchar* controls_describe(int player)
{
	static gchar description[128];

	gchar key_base[kGConfPlayerPathBufferLen];
	const int key_len = sprintf(key_base, kGConfPlayerPath, player);
	gchar *key = key_base + key_len;

	description[0] = '\0';

	strcpy(key, kGConfPlayerKeyboardEnable);
	if (gconf_client_get_bool(gcc, key_base, NULL)) {
		strcpy(description, _("Keyboard"));
	}

	if (description[0] == '\0') {
		strcpy(description, _("Disabled"));
	}

	return description;
}

void controls_dialog(GtkWindow* parent, int p)
{
	dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(_("Controls"),
		parent, GTK_DIALOG_MODAL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL));

	//current_player = player;

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

	int player;
	for( player = 0; player < NUM_PLAYERS; player++ ) {
		gchar* label = g_strdup_printf(_("Player %d Keys"), player+1);
		GtkLabel* separator_1 = GTK_LABEL(gtk_label_new(label));
		gtk_label_set_attributes(separator_1, pattrlist);
		gtk_label_set_justify(separator_1, GTK_JUSTIFY_LEFT);
		g_free(label);

		keys_chk[player] = HILDON_CHECK_BUTTON(hildon_check_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
		gtk_button_set_label(GTK_BUTTON(keys_chk[player]), _("Enable keyboard"));
		set_button_layout(HILDON_BUTTON(keys_chk[player]),
			titles_size_group, values_size_group);

		keys_btn[player] = HILDON_BUTTON(hildon_button_new_with_text(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
			HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
			_("Configure keys…"), NULL));
		set_button_layout(HILDON_BUTTON(keys_btn[player]),
			titles_size_group, values_size_group);
		g_signal_connect(G_OBJECT(keys_btn[player]), "clicked",
						G_CALLBACK(keys_btn_callback), GINT_TO_POINTER(player+1));

		label = g_strdup_printf(_("Player %d Touchscreen"), player+1);
		GtkLabel* separator_2 = GTK_LABEL(gtk_label_new(label));
		gtk_label_set_attributes(separator_2, pattrlist);
		gtk_label_set_justify(separator_2, GTK_JUSTIFY_LEFT);
		g_free(label);

		touch_chk[player] = HILDON_CHECK_BUTTON(hildon_check_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
		gtk_button_set_label(GTK_BUTTON(touch_chk[player]),
			_("Enable touchscreen buttons"));
		set_button_layout(HILDON_BUTTON(touch_chk[player]),
			titles_size_group, values_size_group);

		touch_show_chk[player] = HILDON_CHECK_BUTTON(hildon_check_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
		gtk_button_set_label(GTK_BUTTON(touch_show_chk[player]),
			_("Show on-screen button grid"));
		set_button_layout(HILDON_BUTTON(touch_show_chk[player]),
			titles_size_group, values_size_group);

		label = g_strdup_printf(_("Player %d Accelerometer"), player+1);
		GtkLabel* separator_3 = GTK_LABEL(gtk_label_new(label));
		gtk_label_set_attributes(separator_3, pattrlist);
		gtk_label_set_justify(separator_3, GTK_JUSTIFY_LEFT);
		g_free(label);

		accel_chk[player] = HILDON_CHECK_BUTTON(hildon_check_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT));
		gtk_button_set_label(GTK_BUTTON(accel_chk[player]),
			_("Enable accelerometer input"));
		set_button_layout(HILDON_BUTTON(accel_chk[player]),
			titles_size_group, values_size_group);
	/*
		accel_zero = HILDON_BUTTON(hildon_button_new_with_text(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
			HILDON_BUTTON_ARRANGEMENT_HORIZONTAL,
			_("Set zero position"), "0 0 -1000"));
		set_button_layout(HILDON_BUTTON(accel_zero),
			titles_size_group, values_size_group);
		g_signal_connect(G_OBJECT(accel_zero), "clicked",
						G_CALLBACK(accel_btn_callback), GINT_TO_POINTER(player));
	*/
		accel_sens[player] = HILDON_PICKER_BUTTON(hildon_picker_button_new(
			HILDON_SIZE_AUTO_WIDTH | HILDON_SIZE_FINGER_HEIGHT,
			HILDON_BUTTON_ARRANGEMENT_HORIZONTAL));
		hildon_button_set_title(HILDON_BUTTON(accel_sens[player]),
			_("Accelerometer sensitivity"));
		set_button_layout(HILDON_BUTTON(accel_sens[player]),
			titles_size_group, values_size_group);
		HildonTouchSelector* sens_sel =
			HILDON_TOUCH_SELECTOR(hildon_touch_selector_new_text());
		hildon_touch_selector_append_text(sens_sel, "Low");
		hildon_touch_selector_append_text(sens_sel, "Medium");
		hildon_touch_selector_append_text(sens_sel, "High");
		hildon_picker_button_set_selector(accel_sens[player], sens_sel);

	/*
		GtkLabel* separator_4 = GTK_LABEL(gtk_label_new(_("Wiimote")));
		gtk_label_set_attributes(separator_4, pattrlist);
		gtk_label_set_justify(separator_4, GTK_JUSTIFY_CENTER);

		GtkLabel* separator_5 = GTK_LABEL(gtk_label_new(_("Zeemote")));
		gtk_label_set_attributes(separator_5, pattrlist);
		gtk_label_set_justify(separator_5, GTK_JUSTIFY_CENTER);
	*/
		gtk_box_pack_start(box, GTK_WIDGET(separator_1),
			FALSE, FALSE, HILDON_MARGIN_HALF);
		gtk_box_pack_start(box, GTK_WIDGET(keys_chk[player]),
			FALSE, FALSE, 0);
		gtk_box_pack_start(box, GTK_WIDGET(keys_btn[player]),
			FALSE, FALSE, 0);
		gtk_box_pack_start(box, GTK_WIDGET(separator_2),
			FALSE, FALSE, HILDON_MARGIN_HALF);
		gtk_box_pack_start(box, GTK_WIDGET(touch_chk[player]),
			FALSE, FALSE, 0);
		gtk_box_pack_start(box, GTK_WIDGET(touch_show_chk[player]),
			FALSE, FALSE, 0);
		gtk_box_pack_start(box, GTK_WIDGET(separator_3),
			FALSE, FALSE, HILDON_MARGIN_HALF);
		gtk_box_pack_start(box, GTK_WIDGET(accel_chk[player]),
			FALSE, FALSE, 0);
		//gtk_box_pack_start(box, GTK_WIDGET(accel_zero),
			//FALSE, FALSE, 0);
		gtk_box_pack_start(box, GTK_WIDGET(accel_sens[player]),
			FALSE, FALSE, 0);
		/*gtk_box_pack_start(box, GTK_WIDGET(separator_4),
			FALSE, FALSE, HILDON_MARGIN_HALF);
		gtk_box_pack_start(box, GTK_WIDGET(separator_5),
			FALSE, FALSE, HILDON_MARGIN_HALF);*/
	}

	hildon_pannable_area_add_with_viewport(pannable, GTK_WIDGET(box));
	gtk_box_pack_start_defaults(GTK_BOX(dialog->vbox), GTK_WIDGET(pannable));

	pango_attr_list_unref(pattrlist);
	g_object_unref(titles_size_group);
	g_object_unref(values_size_group);

	load_settings();

	gtk_window_resize(GTK_WINDOW(dialog), 800, 360);

	g_signal_connect(G_OBJECT(dialog), "response",
					G_CALLBACK (cb_dialog_response), NULL);

	gtk_widget_show_all(GTK_WIDGET(dialog));
}

