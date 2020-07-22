#ifndef _GUICALLBACKS_HPP
#define _GUICALLBACKS_HPP

#include <PC_Network.hpp>
#include <PC_Gui.hpp>
#include <memory>
#include <thread>

/*
 *	PeersChar GUI Callback Functions
 *
 *	Note: Once GuiHandler's corresponding member function is called,
 *	      program control returns to GUI event loop.
 */

/* 
 * @method void activate_callback(2)  Called following startup of app, used for showing
 *                      			  the first window of GUI. In addition, sets up widgets and layout
 *                      			  of GUI.
 *
 * @method host_button_callback(2)  Called when "Host Session" button 
 *                                  is pressed.
 *                                    @param widget: Pointer to widget that emitted signal
 *                                    @param gpointer: void* pointer to data being passed
 *                                                     into callback function.
 * 
 * @method join_button_callback(2)  Called when "Join Session" button 
 *                                  is pressed.
 *                                    @param widget: Pointer to widget that emitted signal
 *                                    @param gpointer: void* pointer to data being passed
 *                                                     into callback function
 *
 * @method mute_button_callback(2)  Called when "Mute" button 
 *                                  is pressed.
 *                                    @param widget: Pointer to widget that emitted signal
 *                                    @param gpointer: void* pointer to data being passed
 *                                                     into callback function
 *
 * @method kick_button_callback(2)  Called when "Kick" button 
 *                                  is pressed.
 *                                    @param widget: Pointer to widget that emitted signal
 *                                    @param gpointer: void* pointer to data being passed
 *                                                     into callback function
 *
 * @method direct_checkmark_callback(1)  Called when "Allow Direct Joins" checkmark 
 *                                  is clicked.
 *                                    @param widget: Pointer to widget that emitted signal
 *
 * @method indirect_checkmark_callback(1)  Called when "Allow Indirect Joins" checkmark 
 *                                  is clicked.
 *                                    @param widget: Pointer to widget that emitted signal
 *
 * @method volume_callback(2)  Called whenever volume slider is
 *                             adjusted.
 *                               @param v1: Pointer to volume button widget that
 *                                          emitted signal to callback
 *                               @param value: Double precision value updated every
 *                                             time slider has been moved
 *
 * @method leave_button_callback(2) Called when "Leave Session" button 
 *                                  is pressed.
 *                                    @param widget: Pointer to widget that emitted signal
 *                                    @param gpointer: void* pointer to data being passed
 *                                                     into callback function 
 */

void activate_callback(GtkApplication *app, gpointer data);
void host_button_callback(GtkWidget *widget, gpointer data); 
void join_button_callback(GtkWidget *widget, gpointer data); 
void mute_button_callback(GtkWidget *widget, gpointer data);
void kick_button_callback(GtkWidget *widget, gpointer data);
void direct_checkmark_callback(GtkWidget *widget);
void indirect_checkmark_callback(GtkWidget *widget);
void volume_callback(GtkVolumeButton *v1, gdouble value);
void leave_button_callback(GtkWidget *widget, gpointer data);

#endif 


