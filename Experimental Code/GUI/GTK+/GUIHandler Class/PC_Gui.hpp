#ifndef _PC_GUIHANDLER_HPP
#define _PC_GUIHANDLER_HPP


/*
 *  PeersChat GUI Handler Header
 */


#include <gtk/gtk.h>
#include <regex>

class PC_GuiHandler 
{
	
// Member Variables
private:
	GtkApplication *app;
	GtkWidget *widget_box;
	
	gchar *user_name;
	gchar *user_link;
	
// Constructor, destructor, and initializer
public:
	PC_GuiHandler();
	~PC_GuiHandler();
	int RunGui(int argc, char *argv[]);

// Callback Functions
	void activate(GtkApplication *app, gpointer data);
	void hostButtonPressed(GtkWidget *widget, gpointer data);
	void joinButtonPressed(GtkWidget *widget, gpointer data);
	void leaveButtonPressed(GtkWidget *widget, gpointer data);	

// Setters + Getters providing access for GuiCallbacks.cpp
	GtkWidget* get_widget_box();
	
	void set_user_name(gchar *entry_text);
	gchar* get_user_name();
	void set_user_link(gchar *entry_text);
	gchar* get_user_link();
	
// Utility Functions
private:
	GtkWidget* get_widget_by_name(GtkWidget *container, const gchar *widget_name);
	void hide_all_child_widgets(GtkWidget *container);
	gchar *get_child_entry_text(GtkWidget *container, const gchar *entry_name);
	bool entry_text_is_valid(gchar *entry_text);
	void add_name_to_list(GtkWidget *list, gchar *name);
	void setup_lobby(GtkWidget *parent, GtkWidget *lobby_box);
	void show_error_popup(const gchar *message);
	void username_popup();
	
};

#endif

