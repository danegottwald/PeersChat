#include "PC_Gui.hpp"

/*
 *	PeersChar GUI Callback Functions
 *
 *	Note: Once GuiHandler's corresponding member function is called,
 *	      program control returns to GUI event loop.
 */

void activate_callback(GtkApplication *app, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	gh->activate(app, NULL);
}

void host_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	gh->set_user_name(name_text);
	
	gh->hostButtonPressed(widget, widget_box);
}

void join_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	
	gchar *name_text;
	gchar *link_text;
	name_text = gh->get_child_entry_text(widget_box, "NameEntry");
	link_text = gh->get_child_entry_text(widget_box, "LinkEntry");
	gh->set_user_name(name_text);
	gh->set_user_link(link_text);
	
	gh->joinButtonPressed(widget, widget_box);
}

void mute_button_callback(GtkWidget *widget, gpointer data)
{
	GtkWidget* list_row = gtk_widget_get_parent(widget);
	const gchar* name = gtk_widget_get_name(list_row);
	
	g_print("Mute Button Pressed\n");
	g_print("Name of user: %s\n", name); 
}

void kick_button_callback(GtkWidget *widget, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* list_row = gtk_widget_get_parent(widget);
	const gchar* name = gtk_widget_get_name(list_row);
	
	g_print("Kick Button Pressed\n");
	g_print("Name of user: %s\n", name);
	
	gh->remove_name_from_session(name);
}

void volume_callback(GtkVolumeButton *v1, gdouble value, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->outputVolChanged(v1, value, widget_box);
}

void leave_button_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->leaveButtonPressed(widget, widget_box);
}

