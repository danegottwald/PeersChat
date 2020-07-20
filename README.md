# PeersChat

PeersChat is a desktop application for people looking for temporary voice chatrooms. Host chatrooms, join and leave rooms easily, rooms are deleted when everyone leaves.

# Features
* Host a chatroom and invite friends
* Join rooms using host's ip address and port number
* **Indirect Join**: Allow peers to invite their friends
* Change output volume
* Mute other peers
* Vote kick a peer the group doesn't like
* Option for Command-Line Only mode

Currently only supports Linux, with plans on becoming cross-platform!

# How to Use
This applications requires the following libraries:
* [PortAudio](http://www.portaudio.com/) : Audio I/O Library
* [Opus](https://opus-codec.org/) : Audio Codec Library
* [GTK+](https://www.gtk.org/) : GUI Library

To clone and run this repository, you will need [Git](https://git-scm.com/) and make.
From the command line:
```bash
# Clone this repository
$ git clone https://github.com/danegottwald/PeersChat.git

# Go to the src folder in the repository
$ cd PeersChat/src

# Run make
$ make

# Run the application
$ ./PeersChat
```

##### GUI-mode
<p align="left">
  <img src="./Release Documents/images/lobby.png" alt="GUI Lobby">
</p>

Running the PeersChat application without flags will open up the GUI-mode of this application. To run with **CLI-mode**, add the following flags when running the application:
```
./PeersChat -u username
            -j join a room
            -i ip address & port
            -h host a room
```
