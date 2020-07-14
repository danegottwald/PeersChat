#include "PC_Audio.h"

// Forward Declarations
void opus_error_check(const std::string& message, int error, bool critical);
void Pa_ErrorCheck(const std::string& message, int error, bool critical);

// Initialize static variables
OpusEncoder *PC_AudioHandler::encoder = nullptr;
OpusDecoder *PC_AudioHandler::decoder = nullptr;
char PC_AudioHandler::encodedAudio[]{};
opus_int32 PC_AudioHandler::encodedFrame{};
opus_int32 PC_AudioHandler::decodedFrame{};
float PC_AudioHandler::inputVolume = 1.0f;
float PC_AudioHandler::outputVolume = 0.5f;
bool PC_AudioHandler::streamState = false;
char *PC_AudioHandler::sendOutPtr = sendOut;

/* PC_AudioHandler Constructor
 * Initialize PortAudio, set default devices, and create an
 * encoder and decoder state
 */
PC_AudioHandler::PC_AudioHandler() {
	// Initialize PortAudio and open an audio stream
	portaudioError = Pa_Initialize();
	Pa_ErrorCheck("Failed to initialize portaudio", portaudioError, true);
	portaudioVersion = Pa_GetVersionText();
	portaudioError = Pa_OpenDefaultStream(&stream, 1,1,
	                                      paFloat32,
	                                      SAMPLE_RATE,
	                                      FRAME_SIZE,
	                                      Pa_Callback,
	                                      nullptr);
	Pa_ErrorCheck("Failed to open audio stream", portaudioError, true);

	// Check default devices
	defaultInput = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->name;
	defaultOutput = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->name;

	opusVersion = opus_get_version_string();
	// Create and error check encoder and decoder states
	encoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, OPUS_APPLICATION_VOIP, &opusError);
	opus_error_check("Failed to create encoder", opusError, true);
	decoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &opusError);
	opus_error_check("Failed to create decoder", opusError, true);

	// Set some encoder settings
	opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
	opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
	opus_encoder_ctl(encoder, OPUS_SET_BITRATE(BITRATE));
}

/* PC_AudioHandler Destructor
 * Terminates PortAudio and destroys the encoder and decoder state
 */
PC_AudioHandler::~PC_AudioHandler() {
	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);
	Pa_CloseStream(stream);
	Pa_Terminate();
}

/* startVoiceStream()
 * Starts the voice stream, the stream will remain to stay open until
 * stopVoiceStream() is called.  Must be called every time a stream to
 * reopen the stream.
 */
void PC_AudioHandler::startVoiceStream() {
	if (Pa_IsStreamStopped(stream)) {
		streamState = true;
		portaudioError = Pa_StartStream(stream);
		Pa_ErrorCheck("Failed to start stream", portaudioError, true);
		std::cout << "Audio Stream Opened" << std::endl;
	}
	else {
		std::cout << "Stream already open" << std::endl;
	}
}

/* stopVoiceStream()
 * Closes the voice stream if one is currently open
 */
void PC_AudioHandler::stopVoiceStream() {
	if (Pa_IsStreamActive(stream)) {
		streamState = false;
		Pa_StopStream(stream);
		Pa_ErrorCheck("Failed to stop stream", portaudioError, true);
		std::cout << "Audio Stream Stopped" << std::endl;
	}
	else {
		std::cout << "No stream currently open" << std::endl;
	}
	//networkThread.detach();
}

/* Pa_Callback()
 * Called automatically every time the PortAudio engine has
 * captured audio data or when it needs more audio data for
 * the output. This function is called as a part of startVoiceStream().
 */
int PC_AudioHandler::Pa_Callback(const void *input,
                                 void *output,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo *timeInfo,
                                 PaStreamCallbackFlags status_flags,
                                 void *userData)
{
	// Pointers to the input and output streams
	auto *in = (float*) input;
	auto *out = (float*) output;
	// Adjust the volume of the input if the modifier is not 1.0
	if (inputVolume != 1.0f) {
		for(unsigned int i = 0; i < framesPerBuffer; ++i) {
			in[i] *= inputVolume;
		}
	}

	// Encode and check whether an error occurred
	// 'encodedAudio' swapped with network packet to encode straight into it
	encodedFrame = opus_encode_float(encoder, in, FRAME_SIZE, reinterpret_cast<unsigned char *>(sendOutPtr), MAX_NETPACKET_SIZE);
	opus_error_check("Failed to encode frame", encodedFrame, true);
	sendto(sendSocket, sendOutPtr, MAX_NETPACKET_SIZE, MSG_DONTROUTE, (SOCKADDR*) &UDPServer, sizeof(UDPServer));

	//decodedFrame = opus_decode_float(decoder, reinterpret_cast<const unsigned char *>(recvBuffer), encodedFrame, out, FRAME_SIZE, 0);
	decodedFrame = opus_decode_float(decoder, reinterpret_cast<const unsigned char *>(recvBuffer), strlen(recvBuffer), out, FRAME_SIZE, 0);
	opus_error_check("Failed to decode frame", decodedFrame, true);

	// Adjust the volume of the output if the modifier is not 1.0
	if (outputVolume != 1.0f) {
		//memcpy(out, in, framesPerBuffer * 4);
		for(unsigned int i = 0; i < framesPerBuffer; ++i) {
			out[i] *= outputVolume;
		}
	}
	return 0;
}

/* getEncodedPacket()
 * Returns the encoded audio packet
 */
char* PC_AudioHandler::getEncodedPacket() {
	return encodedAudio;
}

/* getInputVolume()
 * Returns the current volume multiplier for the input device
 */
float PC_AudioHandler::getInputVolume() {
	return inputVolume;
}

/* getOutputVolume()
 * Returns the current volume multiplier for the output device
 */
float PC_AudioHandler::getOutputVolume() {
	return outputVolume;
}

/* setInputVolume()
 * Sets the input device volume multiplier to the given argument
 */
void PC_AudioHandler::setInputVolume(float x) {
	inputVolume = x;
}

/* setOutputVolume()
 * Sets the output device volume multiplier to the given argument
 */
void PC_AudioHandler::setOutputVolume(float x) {
	outputVolume = x;
}

/* getDefaultInput()
 * Returns a string that contains the name of the current default input device
 */
std::string PC_AudioHandler::getDefaultInput() {
	return defaultInput;
}

/* getDefaultOutput()
 * Returns a string that contains the name of the current default output device
 */
std::string PC_AudioHandler::getDefaultOutput() {
	return defaultOutput;
}

/* getOpusVersion()
 * Returns a string that contains the name of the version of the opus codec
 */
std::string PC_AudioHandler::getOpusVersion() {
	return opusVersion;
}

/* getPortAudioVersion()
 * Returns a string that contains the name of the version of portaudio
 */
std::string PC_AudioHandler::getPortAudioVersion() {
	return portaudioVersion;
}

/* getEncodedFrameSize()
 * Returns the size (in bytes) of the encoded packet
 */
opus_int32 PC_AudioHandler::getEncodedFrameSize() {
	return encodedFrame;
}

/* getStreamState()
 * Returns the state of the audio streamer.  True is currently active and
 * False is not active.
 */
bool PC_AudioHandler::getStreamState() {
	return streamState;
}

/* opus_ErrorCheck()
 * Used to check if an opus error has occurred.
 * message: Note as to where and/or what is being checked
 * error: object being checked
 * critical: false/0 if non critical, true/1 if critical
 */
void opus_error_check(const std::string& message, int error, bool critical) {
	if (error < 0) {
		std::cerr << message << ": " << opus_strerror(error) << '\n';
		if (critical) {
			exit(EXIT_FAILURE);
		}
	}
}

/* Pa_ErrorCheck()
 * Used to check if a portaudio error has occurred.
 * message: Note as to where and/or what is being checked
 * error: object being checked
 * critical: false/0 if non critical, true/1 if critical
 */
void Pa_ErrorCheck(const std::string& message, int error, bool critical) {
	if (error != 0) {
		std::cerr << message << ": " << Pa_GetErrorText(error) << '\n';
		Pa_Terminate();
		if (critical) {
			exit(EXIT_FAILURE);
		}
	}
}
