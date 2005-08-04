/***************************************************************************
 *   Copyright (C) 2004-2005 by Nicolas Boichat                            *
 *   nicolas@boichat.ch                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "notebook.h"

/* Protos */
void refresh_profile_manager();

/* Profile manager */
void cancelprofile_callback(GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(saveprofile_button);
	gtk_widget_hide(cancelprofile_button);
	gtk_widget_show(profile_manager_button);
	show_profile_checks(FALSE);
}

void saveprofile_callback(GtkWidget *widget, gpointer data)
{
	char controls[256];
	int size;
	struct profile* profile;
	
	gtk_widget_hide(saveprofile_button);
	gtk_widget_hide(cancelprofile_button);
	gtk_widget_show(profile_manager_button);
	show_profile_checks(FALSE);
	
	size = get_profile_checked_controls(&controls[0]);
	
	set_status(_("Creating profile..."));
	
	profile = ddcci_create_profile(mon, controls, size);
	
	if (!profile) {
		GtkWidget* dialog = gtk_message_dialog_new(
				GTK_WINDOW(main_app_window),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				_("Error while creating profile."));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
		set_status("");
		return;
	}
	
	show_profile_information(profile, TRUE);
		
	set_status("");
	
	ddcci_free_profile(profile);
}

/* Callbacks */
static void apply_callback(GtkWidget *widget, gpointer data)
{
	struct profile* profile = (struct profile*)data;
	
	set_status(_("Applying profile..."));
	
	ddcci_apply_profile(profile, mon);
	
	refresh_all_controls(widget, data);
	
	set_current_main_component(0);
}

static void show_info_callback(GtkWidget *widget, gpointer data)
{
	struct profile* profile = (struct profile*)data;
	
	show_profile_information(profile, FALSE);
}

static void delete_callback(GtkWidget *widget, gpointer data)
{
	struct profile* profile = (struct profile*)data;
	
	GtkWidget* dialog = gtk_message_dialog_new(
		GTK_WINDOW(main_app_window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
		_("Are you sure you want to delete the profile '%s'?"), profile->name);
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case GTK_RESPONSE_YES:
		ddcci_delete_profile(profile, mon);
		refresh_profile_manager();
		break;
	case GTK_RESPONSE_NO:
	default:
		break;
	}
	
	gtk_widget_destroy(dialog);
}

static void create_callback(GtkWidget *widget, gpointer data)
{
	show_profile_checks(TRUE);
	
	set_current_main_component(0);
	
	gtk_widget_hide(profile_manager_button);
	gtk_widget_show(saveprofile_button);
	gtk_widget_show(cancelprofile_button);
}

static void close_profile_manager(GtkWidget *widget, gpointer data)
{
	set_current_main_component(0);
}

/* Initializers */
/* Fill profile manager with components */
void fill_profile_manager() {
	GtkWidget* table;
	GtkWidget* label;
	GtkWidget* button;
	GtkWidget* hsep;
	GtkWidget* hbox;
	GtkWidget* scrolled_window;
	struct profile* profile;
	int count = 0;
	int crow = 0;
	
	profile = mon->profiles;
	
	while (profile != NULL) {
		count++;
		profile = profile->next;
	}
	
	if (count == 0) {
		profile_manager = NULL;
		return;
	}
	
	profile = mon->profiles;
	
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), g_strdup_printf("<span size='large' weight='ultrabold'>%s</span>", _("Profile Manager")));
	gtk_box_pack_start(GTK_BOX(profile_manager), label, 0, 0, 0);
	gtk_widget_show(label);
	
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	
	gtk_container_set_border_width(GTK_CONTAINER(scrolled_window), 10);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
	                                GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	
	table = gtk_table_new((count*2)-1, 5, FALSE);
	
	while (profile != NULL) {
		label = gtk_label_new(profile->name);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, crow, crow+1, GTK_FILL_EXPAND, GTK_SHRINK, 0, 5);
		gtk_widget_show(label);
		
		button = stock_label_button(GTK_STOCK_APPLY, NULL, _("Apply profile"));
		gtk_table_attach(GTK_TABLE(table), button, 1, 2, crow, crow+1, GTK_SHRINK, 0, 5, 5);
		gtk_widget_show(button);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(apply_callback), profile);
		
		button = stock_label_button(GTK_STOCK_EDIT, NULL, _("Show profile details / Rename profile"));
		gtk_table_attach(GTK_TABLE(table), button, 2, 3, crow, crow+1, GTK_SHRINK, 0, 5, 5);
		gtk_widget_show(button);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(show_info_callback), profile);
		
		button = stock_label_button(GTK_STOCK_DELETE, NULL, _("Delete profile"));
		gtk_table_attach(GTK_TABLE(table), button, 3, 4, crow, crow+1, GTK_SHRINK, 0, 5, 5);
		gtk_widget_show(button);
		g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(delete_callback), profile);
		
		crow++;
		
		if (profile->next) {
			hsep = gtk_hseparator_new();
			gtk_table_attach(GTK_TABLE(table), hsep, 0, 3, crow, crow+1, GTK_FILL_EXPAND, GTK_SHRINK, 0, 5);
			crow++;
			gtk_widget_show(hsep);
		}
		
		profile = profile->next;
	}
	
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), table);
	gtk_widget_show(table);
	
	gtk_box_pack_start(GTK_BOX(profile_manager), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show(scrolled_window);
	
	GtkWidget* salign = gtk_alignment_new(0.5, 0, 0, 0);
	hbox = gtk_hbox_new(FALSE, 10);
	
	button = stock_label_button(GTK_STOCK_SAVE, _("Create profile"), NULL);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(create_callback), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
	gtk_widget_show(button);
	
	button = stock_label_button(GTK_STOCK_CLOSE, _("Close profile manager"), NULL);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_profile_manager), NULL);

	gtk_box_pack_start(GTK_BOX(hbox), button, 0, 0, 0);
	gtk_widget_show(button);
	
	gtk_container_add(GTK_CONTAINER(salign), hbox);
	gtk_widget_show(hbox);
	
	gtk_box_pack_start(GTK_BOX(profile_manager), salign, 0, 0, 0);
	gtk_widget_show(salign);
}

void create_profile_manager()
{
	profile_manager = gtk_vbox_new(FALSE, 10);
	
	fill_profile_manager();
}

void refresh_profile_manager()
{
	/* Remove all elements from the container */
	GList *list = gtk_container_get_children(GTK_CONTAINER(profile_manager));
	GList *iter;
	
	if (list) {
		for (iter = g_list_copy(list); iter; iter = g_list_next(iter)) {
			GtkWidget *child = GTK_WIDGET(iter->data);
			gtk_container_remove(GTK_CONTAINER(profile_manager), child);
		}
		g_list_free(iter);
	}
	
	fill_profile_manager();
}

/* Profile information dialog */

static void entry_modified_callback(GtkWidget* entry, GtkWidget* dialog) {
	GtkWidget* ok_button = g_object_get_data(G_OBJECT(dialog), "ok_button");
	
	if (ok_button) {
		gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(ok_button)), ok_button);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_SAVE,   GTK_RESPONSE_OK);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		g_object_set_data(G_OBJECT(dialog), "ok_button", NULL);
	}
}

/* Creates a profile information dialog and show it.
 *  new_profile - indicates if the profile has just been created.
 */
void show_profile_information(struct profile* profile, gboolean new_profile) {
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *hbox;
	int rc;
	
	gchar* title = g_strdup_printf("%s %s", _("Profile information:"), profile->name);
	gchar* tmp;
	
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
		title,
		GTK_WINDOW(main_app_window),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		NULL);
	
	if (new_profile) {
		g_object_set_data(G_OBJECT(dialog), "ok_button", NULL);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_SAVE,   GTK_RESPONSE_OK);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	}
	else {
		GtkWidget* ok_button = gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_ACCEPT);
		g_object_set_data(G_OBJECT(dialog), "ok_button", ok_button);
	}
	
	label = gtk_label_new(NULL);
	tmp = g_strdup_printf("<span size='large' weight='ultrabold'>%s %s</span>", _("Profile information:"), profile->name);
	gtk_label_set_markup(GTK_LABEL(label), tmp);
	g_free(tmp);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);
	
	hbox = gtk_hbox_new(FALSE,0);
	
	label = gtk_label_new(NULL);
	tmp = g_strdup_printf(_("File name: %s"), profile->filename);
	gtk_label_set_text(GTK_LABEL(label), tmp);
	g_free(tmp);
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);
	
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);
	
	hbox = gtk_hbox_new(FALSE,0);
	
	label = gtk_label_new(_("Profile name:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);
	
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), profile->name);
	gtk_box_pack_start(GTK_BOX(hbox), entry, FALSE, FALSE, 5);
	g_signal_connect(GTK_ENTRY(entry), "changed", G_CALLBACK(entry_modified_callback), dialog);
	gtk_widget_show(entry);
	
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, FALSE, FALSE, 5);
	gtk_widget_show(hbox);
	
	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	switch (result)
	{
	case GTK_RESPONSE_OK: /* Save */
		set_status(_("Saving profile..."));
		ddcci_set_profile_name(profile, gtk_entry_get_text(GTK_ENTRY(entry)));
		rc = ddcci_save_profile(profile, mon);
		if (!rc) {
			GtkWidget* dialog = gtk_message_dialog_new(
					GTK_WINDOW(main_app_window),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_CLOSE,
					_("Error while saving profile."));
			gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			set_status("");
			return;
		}
		refresh_profile_manager();
		set_status("");
		break;
	case GTK_RESPONSE_ACCEPT: /* Ok */
	case GTK_RESPONSE_CANCEL: /* Cancel */
	default:
		break;
	}
	
	g_free(title);
	gtk_widget_destroy (dialog);
}
