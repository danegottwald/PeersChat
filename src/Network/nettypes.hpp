#ifndef _NETTYPES_HPP
#define _NETTYPES_HPP

/* NETCODES: An enum that contains all request= types. Requests are sent in
 * a one byte. Request types that contain data will have their left most bit
 * set to 1.
 * 
 * Format: [request type] [content length] [message...]
 * 
*/
enum NETCODES { 
                CONNECT=0x1,    // Request to connect to an existing call
                SHARE=0x81,     // Share user info with host
                SENDP=0x82,     // Share list of peers with joined user
                SENDV=0x83,     // Send voice to other peers
                REQP=0x3,       // Send voice to other peers
                DENY=0x4,       // Deny request to join
                ACCEPT=0x84,    // Accept request to join
                PROPOSE=0x5,    // Ask current peers if new peer can join
                DISCONNECT=0x6, // Disconnect from call; Leave server
                CLOSE=0x7       // Close TCP pipe; end request
};

#endif

