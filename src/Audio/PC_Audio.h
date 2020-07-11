#ifndef _PC_Audio_H
#define _PC_Audio_H

#include <iostream>
#include <thread>
#include <portaudio.h>    // http://portaudio.com/docs/v19-doxydocs/
#include <opus.h>         // https://opus-codec.org/docs/opus_api-1.3.1/index.html

/* PeersChat Audio Handler Library
 *
 */

/* Constants
 * SAMPLE_RATE of input signal (Hz) Must be either 8000, 12000, 16000, 24000, or 48000
 * CHANNELS is the number of channels (1 or 2) in input signal
 * FRAME_SIZE is the duration of the frame in samples (per channel)
 * BITRATE is for setting the bitrate of the encoder
 * MAX_PACKET_SIZE is the max number of bytes that can be written in a packet
 */
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 960
#define BITRATE 24000
#define MAX_PACKET_SIZE 4000

// AudioHandler Class ----------------------------------------------------------
/* AudioHandler: A class for handling audio input/output and encoding/decoding
 *
 * @member encoder  Holds the state of the encoder, used for encoding
 *
 * @member decoder  Holds the state of the decoder, used for decoding
 *
 * @member encodedFrame  Contains the size of the encoded packet in bytes or a
 *                       negative error code if the encode has failed
 *
 * @member decodedFrame  Contains the number of decoded samples after a
 *                       successful decode or a negative error code if the
 *                       decode has failed
 *
 * @member encodedAudio  Contains the array of encoded audio data
 *
 * @member inputVolume  Volume multiplier for the input device
 *
 * @member outputVolume  Volume multiplier for the output device
 *
 * @member opusVersion  String that contains the name of the current
 *                      opus version
 *
 * @member opusError  Holds any error codes that may be set after the use of the
 *                    encoder and decoder. Can be used for debugging
 *
 * @member stream  Holds the state of the portaudio stream. Used for
 *                 initializing an audio stream
 *
 * @member portaudioError  Holds any error codes that may be set after the use
 *                         of the portaudio stream. Can be used for debugging
 *
 * @member portaudioVersion  String that contains the name of the current
 *                           portaudio version
 *
 * @member defaultInput  String that contains the name of the current input
 *                       device
 *
 * @member defaultOutput  String that contains the name of the current output
 *                        device
 *
 * @member streamState  Boolean that holds the current state of the portaudio
 *                      stream
 *
 * @method Pa_Callback(6)  Called by the portaudio engine whenever it has
 *                         captured or when it needs more audio data for output
 *
 * @constructor PC_AudioHandler()  Default constructor, initialize opus and
 *                                  portaudio
 *
 * @destructor ~PC_AudioHandler()  Default destructor, destroy opus encoder
 *                                  and decoder, and the portaudio stream
 *
 * @method beginVoiceStream()  Begins a portaudio audio stream that will
 *                             run until stopVoiceStream() is called
 *
 * @method beginVoiceStreamThreaded()  Begin portaudio audio stream on a new
 *                                     thread
 *
 * @method stopVoiceStream()  Stops a currently active voice stream
 *
 * @member audioThread  Thread for beginVoiceStreamThreaded()
 *
 * @method getEncodePacket()  Returns member 'encodedAudio'
 *
 * @method getEncodeFrameSize()  Returns member 'encodedFrame'
 *
 * @method getOpusVersion()  Returns member 'opusVersion'
 *
 * @method getPortAudioVersion()  Returns member 'portaudioVersion'
 *
 * @method getDefaultInput()  Returns member 'defaultInput'
 *
 * @method getDefaultOutput()  Returns member 'defaultOutput'
 *
 * @method getInputVolume()  Returns member 'inputVolume'
 *
 * @method getOutputVolume()  Returns member 'outputVolume'
 *
 * @method getStreamState()  Returns member 'streamState'
 *
 * @method setInputVolume(1)  Sets 'inputVolume' to parameter
 *
 * @method setOutputVolume(1)  Sets 'outputVolume' to parameter
 */
class PC_AudioHandler {
private:
	// Opus
	static OpusEncoder *encoder;
	static OpusDecoder *decoder;
	static opus_int32 encodedFrame;
	static opus_int32 decodedFrame;
	static uint8_t encodedAudio[MAX_PACKET_SIZE];
	static float inputVolume;
	static float outputVolume;
	std::string opusVersion;
	int opusError{};

	// PortAudio
	PaStream *stream = nullptr;
	PaError portaudioError{};
	std::string portaudioVersion;
	std::string defaultInput;
	std::string defaultOutput;
	static bool streamState;
	static int Pa_Callback(const void *input,
	                       void *output,
	                       unsigned long framesPerBuffer,
	                       const PaStreamCallbackTimeInfo *timeInfo,
	                       PaStreamCallbackFlags status_flags,
	                       void *userData);

public:
	PC_AudioHandler();
	~PC_AudioHandler();

	void beginVoiceStream();
	void beginVoiceStreamThreaded();
	void stopVoiceStream();
	std::thread audioThread;

	// Getters
	static uint8_t getEncodedPacket();
	static uint8_t getEncodedFrameSize();
	std::string getOpusVersion();
	std::string getPortAudioVersion();
	std::string getDefaultInput();
	std::string getDefaultOutput();
	float getInputVolume();
	float getOutputVolume();
	bool getStreamState();

	// Setters
	void setInputVolume(float);
	void setOutputVolume(float);

};

#endif //_PC_Audio_H
