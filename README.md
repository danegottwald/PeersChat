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
* [GTK](https://www.gtk.org/) : GUI Library

### Install for Ubuntu
```bash
# Installing Opus and GTK
sudo apt-get install libopus-dev libopus0 opus-tools libgtk-3-dev

# Installing PortAudio
git clone https://git.assembla.com/portaudio.git

cd portaudio

./configure & make

sudo make install
```

### Install for Arch Linux
```bash
# Installing all libraries
sudo pacman -S opus libopusenc portaudio gtk3
```

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
  <img src="./Release Documents/images/lobby.png" title="PeersChat Lobby" alt="PeersChat Lobby">
  <img src="./Release Documents/images/room.png" title="PeersChat Room" alt="PeersChat Room">
</p>

##### CLI-mode
Running the PeersChat application without flags will open up the GUI-mode of this application. To run with **CLI-mode**, add any combination of arguments following the execute command.
```bash
./PeersChat username [-j address:port]
```
Running the application only inputting a username will create a room with you as the host. To join a room, add the -j flag followed by the address of the room you want to join.