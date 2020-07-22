#ifndef _PC_Audio_HPP
#define _PC_Audio_HPP

#ifdef DEBUG
#define AUDIO_DEBUG
#endif

#include <iostream>
#include <opus.h>     // https://opus-codec.org/docs/opus_api-1.3.1/index.html
#include <portaudio.h>// http://portaudio.com/docs/v19-doxydocs/
#include <memory>

#include "PC_Network.hpp"

/* Constants
 * SAMPLE_RATE of input signal (Hz) Must be either 8000, 12000, 16000, 24000, or 48000
 * CHANNELS is the number of channels (1 or 2) in input signal
 * FRAME_SIZE is the duration of the frame in samples (per channel)
 * BITRATE is for setting the bitrate of the encoder
 */
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960
#define BITRATE 24000

// APeer Class -----------------------------------------------------------------
/* APeer: A class for handling audio input/output and encoding/decoding
 *
 * @constructor APeer()  Default constructor
 *
 * @method Pa_Callback(6)  Called by the portaudio engine whenever it has
 *                         captured audio data.
 *
 * @method startVoiceStream()  Begins a portaudio audio stream that will
 *                             run until stopVoiceStream() is called. The
 *                             voice stream is ran on its own unique thread.
 *
 * @method stopVoiceStream()  Stops a currently active voice stream, if one is
 *                            currently active.
 *
 * @method getOpusVersion()  Returns string of current opus version
 *
 * @method getPortAudioVersion()  Returns string of current portaudio version
 *
 * @method getDefaultInput()  Returns string of current default input device
 *
 * @method getDefaultOutput()  Returns string of current default output device
 *
 * @method getInputVolume()  Returns input device audio multiplier
 *
 * @method getOutputVolume()  Returns output device audio multiplier
 *
 * @method setInputVolume(float)  Sets the input device audio multiplier
 *
 * @method setOutputVolume(float)  Sets the input device audio multiplier
 */
class APeer {
private:
	// Opus Related
	static OpusEncoder *encoder;
	static OpusDecoder *decoder;
	static float inputVolume;
	static float outputVolume;
	std::string opusVersion;
	int opusError = 0;

	// PortAudio Related
	PaStream *stream = nullptr;
	PaError portaudioError = 0;
	std::string portaudioVersion;
	std::string defaultInput;
	std::string defaultOutput;
	static bool micMute;
	static bool deafen;
	static int Pa_Callback(const void *input,
	                       void *output,
	                       unsigned long framesPerBuffer,
	                       const PaStreamCallbackTimeInfo *timeInfo,
	                       PaStreamCallbackFlags status_flags,
	                       void *userData);

public:
	APeer();
	~APeer();

	void startVoiceStream();
	void stopVoiceStream();

	// Getters
	const PaStreamInfo *getStreamInfo();
	double getCPULoad();
	std::string getOpusVersion();
	std::string getPortAudioVersion();
	std::string getDefaultInput();
	std::string getDefaultOutput();
	float getInputVolume();
	float getOutputVolume();

	// Setters
	void setInputVolume(float);
	void setOutputVolume(float);
	void setMuteMic(bool);
	void setDeafen(bool);
};

#endif//_PC_Audio_H
