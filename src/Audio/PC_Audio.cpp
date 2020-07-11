#include "PC_Audio.hpp"

// Forward Declarations
void opus_error_check(const std::string& message, int error, bool critical);
void Pa_ErrorCheck(const std::string& message, int error, bool critical);

// Initialize static variables
OpusEncoder *PC_AudioHandler::encoder = nullptr;
OpusDecoder *PC_AudioHandler::decoder = nullptr;
uint8_t PC_AudioHandler::encodedAudio[]{};
opus_int32 PC_AudioHandler::encodedFrame{};
opus_int32 PC_AudioHandler::decodedFrame{};
float PC_AudioHandler::inputVolume = 1.0f;
float PC_AudioHandler::outputVolume = 0.1f;

/* PC_AudioHandler Constructor
 * Initialize PortAudio, set default devices, and create an
 * encoder and decoder state
 */
PC_AudioHandler::PC_AudioHandler() {
	// Initialize PortAudio and open an audio stream
	portaudioError = Pa_Initialize();
	Pa_ErrorCheck("Failed to initialize portaudio", portaudioError, true);
	portaudioVersion = Pa_GetVersionText();
	portaudioError = Pa_OpenDefaultStream(&stream, 1,
	                                      1,
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
	Pa_Terminate();
	Pa_CloseStream(stream);
	stream = nullptr;
	encoder = nullptr;
	decoder = nullptr;
}

/* beginVoiceStream()
 * Starts the voice stream
 */
void PC_AudioHandler::beginVoiceStream() {
	portaudioError = Pa_StartStream(stream);
	Pa_ErrorCheck("Failed to start stream", portaudioError, true);

	std::cout << std::endl << "Playback Begun" << std::endl;
	Pa_Sleep(5000);

	Pa_StopStream(stream);
	Pa_ErrorCheck("Failed to stop stream", portaudioError, true);
}

/* Pa_Callback()
 * Called automatically every time the PortAudio engine has
 * captured audio data or when it needs more audio data for
 * the output. This function is called as a part of Pa_StartStream()
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
			// Adjust input volume
			in[i] *= inputVolume;
		}
	}

	// Encode and check whether an error occurred
	encodedFrame = opus_encode_float(encoder, in, FRAME_SIZE, encodedAudio, MAX_PACKET_SIZE);
	opus_error_check("Failed to encode frame", encodedFrame, true);

	decodedFrame = opus_decode_float(decoder, encodedAudio, encodedFrame, out, FRAME_SIZE, 0);
	opus_error_check("Failed to decode frame", decodedFrame, true);

	// Adjust the volume of the output if the modifier is not 1.0
	if (outputVolume != 1.0f) {
		//memcpy(out, in, framesPerBuffer * 4);
		for(unsigned int i = 0; i < framesPerBuffer; ++i) {
			// Adjust output volume of input playback
			out[i] *= outputVolume;
		}
	}
	return 0;
}

/* getEncodedPacket()
 * Returns the encoded audio packet
 */
uint8_t PC_AudioHandler::getEncodedPacket() {
	return reinterpret_cast<uint8_t>(encodedAudio);
}

float PC_AudioHandler::getInputVolume() {
	return inputVolume;
}

float PC_AudioHandler::getOutputVolume() {
	return outputVolume;
}

void PC_AudioHandler::setInputVolume(float x) {
	inputVolume = x;
}

void PC_AudioHandler::setOutputVolume(float x) {
	outputVolume = x;
}

std::string PC_AudioHandler::getDefaultInput() {
	return defaultInput;
}

std::string PC_AudioHandler::getDefaultOutput() {
	return defaultOutput;
}

/* opus_ErrorCheck()
 * Used to check if an opus error has occurred.
 * message: Note as to where and/or what is being checked
 * error: object being checked
 * critical: false/0 if non critical, true/1 if critical
 */
void opus_error_check(const std::string& message, int error, bool critical) {
	if (error < 0) {
		std::cerr << message << ": " << opus_strerror(error);
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
		std::cerr << message << ": " << Pa_GetErrorText(error);
		Pa_Terminate();
		if (critical) {
			exit(EXIT_FAILURE);
		}
	}
}

