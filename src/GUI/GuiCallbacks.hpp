#ifndef _GUICALLBACKS_HPP
#define _GUICALLBACKS_HPP

#include <PC_Gui.hpp>

extern PeersChatNetwork *Network;
extern APeer *Audio;

/*
 *	PeersChar GUI Callback Functions
 *
 *	Note: Once GuiHandler's corresponding member function is called,
 *	      program control returns to GUI event loop.
 */

void activate_callback(GtkApplication *app, gpointer data);
void host_button_callback(GtkWidget *widget, gpointer data); 
void join_button_callback(GtkWidget *widget, gpointer data); 
void mute_button_callback(GtkWidget *widget);
void kick_button_callback(GtkWidget *widget, gpointer data);
void volume_callback(GtkVolumeButton *v1, gdouble value);
void leave_button_callback(GtkWidget *widget, gpointer data);

#endif 


