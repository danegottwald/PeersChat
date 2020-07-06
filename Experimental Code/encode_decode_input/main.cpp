#include <iostream>
#include <thread>
#include <portaudio.h>
#include <opus.h>		// https://opus-codec.org/docs/opus_api-1.3.1/index.html

/*	Constants
 *  	SAMPLE_RATE of input signal (Hz) Must be either 8000, 12000, 16000, 24000, or 48000
 *		CHANNELS is the number of channels (1 or 2) in input signal
 *		FRAME_SIZE is the duration of the frame in samples (per channel)
 *		BITRATE is for setting the bitrate of the encoder
 *		MAX_PACKET_SIZE is the max number of bytes that can be written in a packet
 */
#define SAMPLE_RATE 48000
#define CHANNELS 1
#define FRAME_SIZE 1920
#define BITRATE 64000
#define MAX_PACKET_SIZE 4000

/*	IN_VOLUME_MULTIPLIER (0.0 - 1.0) varies the input volume
 *	OUT_VOLUME_MULTIPLIER (0.0 - 1.0) varies the output volume
 */
#define IN_VOLUME_MULTIPLIER  1.0
#define OUT_VOLUME_MULTIPLIER 0.1
double ENCODE_SIZE = 0;

// Holds the state of the encoder and decoder
OpusEncoder *encoder = nullptr;
OpusDecoder *decoder = nullptr;

// Forward Declarations
void voiceStream();
void opus_ErrorCheck(const std::string& message, int error, bool critical);
void Pa_ErrorCheck(const std::string& message, int error, bool critical);

int main() {

	// Print library versions to console
	std::cout << Pa_GetVersionText() << std::endl;
	std::cout << opus_get_version_string() << std::endl;

	//std::thread voiceStreamThread(voiceStream);

	// Declare variables
	int error = 0;

	// Creates and error checks a new encoder state
	encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &error);
	opus_ErrorCheck("Failed to create encoder", error, true);

	// Creates and error checks a new decoder state
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &error);
	opus_ErrorCheck("Failed to create encoder", error, true);

	/*	OPUS_SET_SIGNAL() tells encoder that a voice is being encoded
	 *	OPUS_SET_VBR() is variable bitrate, 0 off, 1 on
	 *	OPUS_SET_BITRATE() sets the bitrate of the encoder
	 */
	opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
	opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));

	voiceStream();

	// Destroy the encoder and decoders
	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);

	//voiceStreamThread.join();

	return 0;
}

/*	Pa_Callback()
 * 	This function is called everything the PortAudio engine has captured audio data
 * 	or when it needs more audio data for the output. This function is called as a part
 * 	of Pa_StartStream().
 *		input: This is the incoming audio input data
 *		output: This is used to send audio data to output
 */
static int Pa_Callback(	const void *input,
						   void *output,
						   unsigned long framesPerBuffer,
						   const PaStreamCallbackTimeInfo *timeInfo,
						   PaStreamCallbackFlags status_flags,
						   void *userData)
{
	// Buffer for the encoded data to be stored
	static uint8_t buffer[MAX_PACKET_SIZE];
	// Pointers to the input and output
	auto *in = (float*) input;
	auto *out = (float*) output;

	// Adjust the volume of the input if the modifier is not 1.0
	if (IN_VOLUME_MULTIPLIER != 1.0) {
		for(unsigned int i = 0; i < framesPerBuffer; ++i) {
			// Adjust input volume
			in[i] *= IN_VOLUME_MULTIPLIER;
		}
	}

	// Encode and check whether an error occurred
	opus_int32 encodedFrame = opus_encode_float(encoder, in, FRAME_SIZE, buffer, MAX_PACKET_SIZE);
	opus_ErrorCheck("Failed to encode frame", encodedFrame, true);

	// Sum up the total bytes encoded
	ENCODE_SIZE += encodedFrame;

	// Decode and check whether an error occurred
	int decodedFrame = opus_decode_float(decoder, buffer, encodedFrame, out, FRAME_SIZE, 0);
	opus_ErrorCheck("Failed to decode frame", decodedFrame, true);

	/*	Enable or disable input playback
	 * 	TODO - Tie to variable and GUI to allow the testing of a user's input
	 */
	if (false) {
		memcpy(out, in, framesPerBuffer * 4);
		for(unsigned int i = 0; i < framesPerBuffer; ++i) {
			// Adjust output volume of input playback
			out[i] *= OUT_VOLUME_MULTIPLIER;
		}
	}
	return 0;
}

/*	voiceStream()
 * 	This function is used to begin voice streaming
 *	Has no parameters and returns void
 */
void voiceStream() {

	// Initialize Port Audio
	PaError val = Pa_Initialize();
	Pa_ErrorCheck("Failed to initialize portaudio", val, false);

	// Print information about devices
	std::cout << "Devices available: " << Pa_GetDeviceCount() << std::endl;
	std::cout << "Default input: " << Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->name << std::endl;
	std::cout << "Default output: " << Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name << std::endl;

	// Open Audio Stream
	PaStream *stream = nullptr;
	val = Pa_OpenDefaultStream(&stream,1, 1, paFloat32, SAMPLE_RATE, FRAME_SIZE, Pa_Callback, nullptr);
	Pa_ErrorCheck("Failed to open stream", val, false);

	// Start Stream
	val = Pa_StartStream(stream);
	Pa_ErrorCheck("Failed to start stream", val, false);

	// Sleep main thread so stream stays open
	std::cout << std::endl << "Playback Begun" << std::endl;
	Pa_Sleep(10000);
	std::cout << ENCODE_SIZE / 1024.0 << " Kilobytes of encoded audio" << std::endl;

	// End audio stream
	Pa_StopStream(stream);
	Pa_ErrorCheck("Failed to stop stream", val, false);

	// Close and terminate audio stream
	Pa_CloseStream(stream);
	stream = nullptr;
	Pa_Terminate();
}

/*	opus_ErrorCheck()
 * 	Used to check if an opus error has occurred.
 * 		message: Note as to where and/or what is being checked
 * 		error: object being checked
 * 		critical: false/0 if non critical, true/1 if critical
 */
void opus_ErrorCheck(const std::string& message, int error, bool critical) {
	if (error < 0) {
		std::cerr << message << ": " << opus_strerror(error);
		if (critical) {
			exit(EXIT_FAILURE);
		}
	}
}

/*	Pa_ErrorCheck()
 * 	Used to check if a portaudio error has occurred.
 * 		message: Note as to where and/or what is being checked
 * 		error: object being checked
 * 		critical: false/0 if non critical, true/1 if critical
 */
void Pa_ErrorCheck(const std::string& message, int error, bool critical) {
	if (error != 0) {
		std::cerr << message << ": " << Pa_GetErrorText(error);
		if (critical) {
			exit(EXIT_FAILURE);
		}
		Pa_Terminate();
	}
}