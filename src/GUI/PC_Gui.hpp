#ifndef _PC_GUIHANDLER_HPP
#define _PC_GUIHANDLER_HPP


/*
 *  PeersChat GUI Handler Class Header
 */


#include <gtk/gtk.h> // https://developer.gnome.org/gtk3/stable/
#include <regex>
#include <cstring>


// Pre-Compiler Constants
#define DEFAULT_WINDOW_WIDTH 360
#define DEFAULT_WINDOW_HEIGHT 240

// Window padding: pads around border of window
#define DEFAULT_WINDOW_PADDING 32
// Widget padding: pads between widgets
#define DEFAULT_WIDGET_PADDING 16

// Max name length: Cannot exceed # of characters
#define MAX_NAME_LEN 18

// GuiHandler Class ----------------------------------------------------------------------
/* GuiHandler: Class for encapsulating GUI functionality 
 *
 * @member app  Pointer to the GtkApplication, a GTK+ object that automatically
 *              initializes GTK+ library and acts as entry point to GUI program. 
 *
 * @member widget_box  Pointer to the primary GtkWidget container, used
 *                     in order to access GtkWidget data from callback functions
 *
 * @member user_name  Holds text data for username from textbox entry
 *
 * @member user_link  Holds text data for joining link from textbox entry
 *
 * @constructor PC_GuiHandler()  Default contructor, initializes private fields as well as
 *                               GtkApplication (serves as root to GtkObjects).
 *
 * @method runGui(2)  Begins GUI event loop, starting by calling activate() callback func.
 *                      @params argc, argv: Takes in main() arguments, GTK+ provides
 *                                          optional command line argument handling.
 *                      @return (int): Returns status code at the end of GUI event loop.
 *                                     (output of g_application_run)
 *
 * ===Callback Functions===
 * Note: Callback functions of GUI class accessed externally through GuiCallbacks.cpp
 *
 * @method activate(2)  Callback function called following startup of app, used for showing
 *                      the first window of GUI. In addition, sets up widgets and layout
 *                      of GUI.
 *
 * @method hostButtonPressed(2)  Callback function called when "Host Session" button 
 *                               is pressed.
 *                                 @param widget: Pointer to widget that emitted signal
 *                                 @param gpointer: void* pointer to data being passed
 *                                                  into callback function.
 * 
 * @method joinButtonPressed(2)  Callback function called when "Join Session" button 
 *                               is pressed.
 *                                 @param widget: Pointer to widget that emitted signal
 *                                 @param gpointer: void* pointer to data being passed
 *                                                  into callback function
 *
 * @method outputVolChanged(3)  Callback function called whenever volume slider is
 *                              adjusted.
 *                                @param v1: Pointer to volume button widget that
 *                                           emitted signal to callback
 *                                @param value: Double precision value updated every
 *                                              time slider has been moved
 *                                @param gpointer: void* pointer to data being passed
 *                                                  into callback function
 *
 * @method leaveButtonPressed(2) Callback function called when "Leave Session" button 
 *                               is pressed.
 *                                 @param widget: Pointer to widget that emitted signal
 *                                 @param gpointer: void* pointer to data being passed
 *                                                  into callback function
 */
class PC_GuiHandler 
{
	
// Member Variables
private:
	GtkApplication *app;
	GtkWidget *widget_box;
	
	gchar *user_name;
	gchar *user_link;
	
	bool is_host;
	
// Constructor, destructor, and initializer
public:
	PC_GuiHandler();
	~PC_GuiHandler();
	int runGui(int argc, char *argv[]);

// Callback Functions
	void activate(GtkApplication *app, gpointer data);
	void hostButtonPressed(GtkWidget *widget, gpointer data);
	void joinButtonPressed(GtkWidget *widget, gpointer data);
	void outputVolChanged(GtkVolumeButton *v1, gdouble value, gpointer data);
	void leaveButtonPressed(GtkWidget *widget, gpointer data);	

// Setters + Getters providing access for GuiCallbacks.cpp
	GtkWidget* get_widget_by_name(GtkWidget *container, const gchar *widget_name);
	gchar *get_child_entry_text(GtkWidget *container, const gchar *entry_name);
	GtkWidget* get_widget_box();
	
	void set_user_name(gchar *entry_text);
	gchar* get_user_name();
	void set_user_link(gchar *entry_text);
	gchar* get_user_link();
	
// Utility Functions
private:
	void hide_all_child_widgets(GtkWidget *container);
	bool entry_text_is_valid(gchar *entry_text);
	void add_name_to_list(GtkWidget *list, gchar *name);
	void setup_lobby(GtkWidget *parent, GtkWidget *lobby_box);
	void show_error_popup(const gchar *message);
	void username_popup();
	
};

#endif

