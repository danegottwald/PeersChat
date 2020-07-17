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

void host_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->hostButtonPressed(widget, widget_box);
}

void join_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->joinButtonPressed(widget, widget_box);
}

void volume_callback(GtkVolumeButton *v1, gdouble value, gpointer data)
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->outputVolChanged(v1, value, widget_box);
}

void leave_callback(GtkWidget *widget, gpointer data) 
{
	PC_GuiHandler* gh = static_cast<PC_GuiHandler*>(data);
	GtkWidget* widget_box = gh->get_widget_box();
	gh->leaveButtonPressed(widget, widget_box);
}

