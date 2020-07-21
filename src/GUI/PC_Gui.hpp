#ifndef _PC_GUIHANDLER_HPP
#define _PC_GUIHANDLER_HPP


/*
 *  PeersChat GUI Handler Class Header
 */


#include <gtk/gtk.h> // Using GTK+ 3.0: https://developer.gnome.org/gtk3/stable/
#include <regex>
#include <cstring>
#include <PC_Audio.hpp>
#include <PC_Network.hpp>
#include <GuiCallbacks.hpp>
#include <chrono>
#include <string>
#include <cinttypes>

// Pre-Compiler Constants
#define DEFAULT_WINDOW_WIDTH 360
#define DEFAULT_WINDOW_HEIGHT 240

// Window padding: pads around border of window
#define DEFAULT_WINDOW_PADDING 32
// Widget padding: pads between widgets
#define DEFAULT_WIDGET_PADDING 16
// Value that volume slider begins at (values in 0-1.0 range)
#define DEFAULT_VOLUME_SLIDER_VALUE 0.5

// Max name length: Cannot exceed # of characters
#define MAX_NAME_LEN 18


// GuiHandler Class -----------------------------------------------------------------------
/* GuiHandler: Class for encapsulating GUI functionality 
 *
 * @member app  Pointer to the GtkApplication, a GTK+ object that automatically
 *              initializes GTK+ library and acts as entry point to GUI program. 
 *
 * @member widget_box  Pointer to the primary GtkWidget container, used
 *                     in order to access GtkWidget data from callback functions
 *
 * @member name_list  Pointer to GtkWidget holding rows that hold names of users,
 *                    also containing kick/mute buttons
 *
 * @member user_name  Holds text data for username from textbox entry
 *
 * @member user_link  Holds text data for joining link from textbox entry
 *
 * @member is_host  Boolean that's true if GuiHandler user is hosting a session
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
 * @method void add_host_to_session(1)  Adds a host to the GUI name_list for an ongoing
 *                                      session.
 *                                        @prereq: GUI is already running through runGui()  
 *                                        @param name: User's name to be displayed
 *                                                     in session
 *
 * @method void add_user_to_session(1)  Adds a user (non-host) to the GUI name_list for an 
 *                                      ongoing session.
 *                                        @prereq: GUI is already running through runGui()  
 *                                        @param name: User's name to be displayed
 *                                                     in session
 *
 * @method void remove_name_from_session(1)  Removes a user from the GUI name_list
 *                                           of an ongoing session.
 *                                             @prereq: GUI is already running through runGui()  
 *                                             @param name: User's name to be removed
 *                                                          from session
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
	GtkWidget *name_list;
	
	gchar *user_name;
	gchar *user_link;
	gchar *user_port;
	
	bool is_host;
	
public:
	// Maps user id -> name
	std::map<int, gchar*> users;
	
// Constructor, destructor, and initializer
public:
	PC_GuiHandler();
	~PC_GuiHandler();
	int runGui(int argc, char *argv[]);

// Public functions for managing user session GUI
	void add_host_to_session(const gchar *name);
	void add_user_to_session(const gchar *name, bool kickable);
	void remove_name_from_session(const gchar *name);
	void refresh_name_list();
	
// Callback Functions
	void activate(GtkApplication *app);
	void hostButtonPressed(GtkWidget *widget, gpointer data);
	void joinButtonPressed(GtkWidget *widget, gpointer data);
	void leaveButtonPressed(GtkWidget *widget, gpointer data);	

// Setters + Getters providing access for GuiCallbacks.cpp
	GtkWidget* get_widget_by_name(GtkWidget *container, const gchar *widget_name);
	gchar *get_child_entry_text(GtkWidget *container, const gchar *entry_name);
	GtkWidget* get_widget_box();
	
	void set_user_name(gchar *entry_text, size_t pos);
	gchar* get_user_name();
	void set_user_link(gchar *entry_text);
	gchar* get_user_link();
	void set_user_port(gchar *entry_text);
	gchar* get_user_port();
	
// Utility Functions
private:
	void hide_all_child_widgets(GtkWidget *container);
	bool entry_text_is_valid(gchar *entry_text);
	GtkWidget* create_new_user_row(const gchar *name, bool is_host, bool kickable);
	void setup_lobby(GtkWidget *parent, GtkWidget *lobby_box);
	GtkWidget* create_volume_slider();
	GtkWidget* create_indirect_join_toggle();
	void show_error_popup(const gchar *message);
	void username_popup();
	
};

#endif
