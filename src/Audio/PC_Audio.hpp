#ifndef _PC_Audio_HPP
#define _PC_Audio_HPP

#include <iostream>
#include <portaudio.h>    // http://portaudio.com/docs/v19-doxydocs/
#include <opus.h>         // https://opus-codec.org/docs/opus_api-1.3.1/index.html

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
 * @member encodedAudio[]  Contains the array of encoded audio data
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
 * @method Pa_Callback(6)  Called by the portaudio engine whenever it has
 *                         captured or when it needs more audio data for output
 *
 * @constructor PC_AudioHandler(0)  Default constructor, initialize opus and
 *                                  portaudio
 *
 * @destructor ~PC_AudioHandler(0)  Default destructor, destroy opus encoder
 *                                  and decoder, and the portaudio stream
 *
 * @member inputVolume  Adjust the volume of the audio input stream on a
 *                      0.0 to 1.0 scale (0% to 100%)
 *
 * @member outputVolume  Adjust the volume of the audio output stream on a
 *                       0.0 to 1.0 scale (0% to 100%)
 *
 * @member defaultInput  Holds the name of the default input that is detected
 *                       by portaudio.
 *
 * @member defaultOutput  Holds the name of the default output that is detected
 *                        by portaudio.
 *
 * @method beginVoiceStream(0)  Used to begin audio data collection for
 *                              portaudio and the opus encoder
 *
 * @method getEncodePacket(0)  Used to get the private member 'encodedAudio'
 *
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

	// Getters
	static uint8_t getEncodedPacket();
	std::string getDefaultInput();
	std::string getDefaultOutput();
	float getInputVolume();
	float getOutputVolume();

	// Setters
	void setInputVolume(float);
	void setOutputVolume(float);

};


#endif //_PC_Audio_HPP

