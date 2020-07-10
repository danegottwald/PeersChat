#include <iostream>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <portaudio.h>
#include <opus.h>		// https://opus-codec.org/docs/opus_api-1.3.1/index.html

#include "PC_Network.h"
#include "PC_Audio.h"

// TODO: Merging multiple streams and decoding multiple streams
// TODO: PC_AudioHandler::beginVoiceStream() to run constantly

int main() {

	// Print library versions to console
	std::cout << Pa_GetVersionText() << std::endl;
	std::cout << opus_get_version_string() << std::endl;

	PC_AudioHandler handler;
	handler.beginVoiceStream();
	PC_AudioHandler::outputVolume = 0.5f;
	handler.beginVoiceStream();

	return 0;
}
