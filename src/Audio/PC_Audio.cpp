#include "PC_Audio.hpp"

uint32_t PACKETS_LOST = 0;
uint32_t TOTAL_PACKETS = 0;

// Forward Declarations
void opus_error_check(const std::string &message, int error, bool critical);
void Pa_ErrorCheck(const std::string &message, int error, bool critical);

// Static variables for use within the static Pa_Callback() portaudio function
OpusEncoder *APeer::encoder = nullptr;
OpusDecoder *APeer::decoder = nullptr;
float APeer::inputVolume = 1.0f;
float APeer::outputVolume = 0.5f;
bool APeer::micMute = false;
bool APeer::deafen = false;

NPeer network;
PeersChatNetwork *pcn;

/* APeer Constructor
 * Initialize PortAudio, set default devices, create an encoder and decoder
 * state, and set encoder settings.
 */
APeer::APeer() {
	#ifdef AUDIO_DEBUG
	std::cout << "APeer Constructor Called" << std::endl;
	#endif
	// Initialize PortAudio and open an audio stream
	portaudioError = Pa_Initialize();
	Pa_ErrorCheck("Failed to initialize portaudio", portaudioError, true);
	portaudioVersion = Pa_GetVersionText();
	portaudioError = Pa_OpenDefaultStream(&stream, 1, 1,
	                                      paFloat32,
	                                      SAMPLE_RATE,
	                                      FRAME_SIZE,
	                                      Pa_Callback,
	                                      &network);
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
	//opus_encoder_ctl(encoder, OPUS_SET_INBAND_FEC(1));
	//opus_encoder_ctl(encoder, OPUS_SET_PACKET_LOSS_PERC(50));
	#ifdef AUDIO_DEBUG
	std::cout << "APeer Constructor Completed" << std::endl;
	#endif
}

/* APeer Destructor
 * Closes and terminates the PortAudio stream and destroys the encoder and
 * decoder state.
 */
APeer::~APeer() {
	#ifdef AUDIO_DEBUG
	std::cout << "APeer Destructor Called" << std::endl;
	#endif
	Pa_AbortStream(stream);
	Pa_CloseStream(stream);
	opus_encoder_destroy(encoder);
	opus_decoder_destroy(decoder);
	Pa_Terminate();
	#ifdef AUDIO_DEBUG
	std::cout << "Apeer Destructor Completed" << std::endl;
	#endif
}

/* Pa_Callback()
 * Called automatically every time the PortAudio engine has
 * captured audio data.  Encoding/decoding, input/output volumes, and enqueueing
 * audio packets is done here.
 */
int APeer::Pa_Callback(const void *input,
                       void *output,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags status_flags,
                       void *userData) {
	auto *net = (NPeer*) userData;
	auto *in = (float *) input;
	auto *out = (float *) output;
	if (inputVolume != 1.0f && (inputVolume > 0 && inputVolume <= 1.5f)) {
		for (unsigned int i = 0; i < framesPerBuffer; ++i) {
			in[i] *= inputVolume;
		}
	}

	AudioOutPacket *outPacket = net->getEmptyOutPacket();
	if (!micMute || inputVolume != 0) {
		outPacket->packet_len = opus_encode_float(encoder,
		                                          in,
		                                          FRAME_SIZE,
		                                          outPacket->packet.get(),
		                                          BUFFER_SIZE);
		#ifdef AUDIO_DEBUG
		opus_error_check("Failed to encode frame", outPacket->packet_len, true);
		#endif
		net->enqueue_out(outPacket);
	}

	for (int i = 0; i < pcn->getNumberPeers(); i++) {
		NPeer *peer = (*pcn)[i];
		uint32_t lastPacketID = peer->getInPacketId();
		AudioInPacket *inPacket = peer->getAudioInPacket();
		if (inPacket != nullptr) {
			PACKETS_LOST += inPacket->packet_id - lastPacketID - 1;
			TOTAL_PACKETS = inPacket->packet_id;
			if (!deafen) {  // May cause port audio to continuously play last packet, needs testing, check else
				int decodedFrame = opus_decode_float(decoder,
				                                     inPacket->packet.get(),
				                                     inPacket->packet_len,
				                                     out,
				                                     FRAME_SIZE,
				                                     0);
				#ifdef AUDIO_DEBUG
				opus_error_check("Failed to decode frame", decodedFrame, false);
				#endif
			}
			else {
				out = nullptr;
			}
			peer->retireEmptyInPacket(inPacket);
			if ((!deafen && outputVolume != 1.0f)
			    && (outputVolume >= 0 && outputVolume <= 2.0f)) {
				for (unsigned int j = 0; j < framesPerBuffer; ++j) {
					out[i] *= outputVolume;
				}
			}
		}
	}
	return 0;
}

/* startVoiceStream()
 * Starts the voice stream if one is not already open. The stream will remain
 * to stay open until stopVoiceStream() is called.
 */
void APeer::startVoiceStream() {
	if (Pa_IsStreamStopped(stream)) {
		portaudioError = Pa_StartStream(stream);
		Pa_ErrorCheck("Failed to start stream", portaudioError, true);
		std::cout << "Audio Stream Opened" << std::endl;
	} else {
		std::cout << "Stream already open" << std::endl;
	}
}

/* stopVoiceStream()
 * Closes the voice stream if one is currently active.
 */
void APeer::stopVoiceStream() {
	if (Pa_IsStreamActive(stream)) {
		Pa_AbortStream(stream);
//		Pa_StopStream(stream);
		Pa_ErrorCheck("Failed to stop stream", portaudioError, true);
		std::cout << "Audio Stream Stopped" << std::endl;
	} else {
		std::cout << "No stream currently open" << std::endl;
	}
}

// APeer Class Getters ---------------------------------------------------------

/* getInputVolume()
 * Returns the current volume multiplier for the input device.
 */
float APeer::getInputVolume() {
	return inputVolume;
}

/* getOutputVolume()
 * Returns the current volume multiplier for the output device.
 */
float APeer::getOutputVolume() {
	return outputVolume;
}

/* getDefaultInput()
 * Returns a string that contains the name of the current default input device.
 */
std::string APeer::getDefaultInput() {
	return defaultInput;
}

/* getDefaultOutput()
 * Returns a string that contains the name of the current default output device.
 */
std::string APeer::getDefaultOutput() {
	return defaultOutput;
}

/* getOpusVersion()
 * Returns a string that contains the name of the version of the opus codec.
 */
std::string APeer::getOpusVersion() {
	return opusVersion;
}

/* getPortAudioVersion()
 * Returns a string that contains the name of the version of PortAudio.
 */
std::string APeer::getPortAudioVersion() {
	return portaudioVersion;
}

/* getStreamInfo()
 * Used to get information such as sample rate and latency of the PortAudio
 * stream.
 */
const PaStreamInfo *APeer::getStreamInfo() {
	return Pa_GetStreamInfo(stream);
}

/* getCPULoad()
 * Returns the current CPU load of the port audio stream.
 */
double APeer::getCPULoad() {
	return Pa_GetStreamCpuLoad(stream);
}

// APeer Class Setters ---------------------------------------------------------

/* setInputVolume()
 * Sets the input device volume multiplier to the given argument.
 *     Parameter Range: 0.0 - 1.25
 */
void APeer::setInputVolume(float x) {
	if (x >= 0 && x <= 1.25f) {
		inputVolume = x;
	}
}

/* setOutputVolume()
 * Sets the output device volume multiplier to the given argument.
 *     Parameter Range: 0.0 - 2.0
 */
void APeer::setOutputVolume(float x) {
	if (x >= 0 && x <= 2.0f) {
		outputVolume = x;
	}
}

/* setMuteMic()
 * Sets the state of the input microphone.  False will disable microphone
 * input and true will enable input.
 */
void APeer::setMuteMic(bool micState) {
	micMute = micState;
}

/* setDeafen()
 * Sets the state of the output audio.  False will cause Pa_Callback() to skip
 * decoding of packets and play no audio.  True will allow the decoding of
 * packets.
 */
void APeer::setDeafen(bool deafenState) {
	deafen = deafenState;
}

// Non class functions ---------------------------------------------------------

/* opus_ErrorCheck()
 * Used to check if an opus error has occurred.
 * message: Note as to where and/or what is being checked
 * error: object being checked
 * critical: false/0 if non critical, true/1 if critical
 */
void opus_error_check(const std::string &message, int error, bool critical) {
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
void Pa_ErrorCheck(const std::string &message, int error, bool critical) {
	if (error != 0) {
		std::cerr << message << ": " << Pa_GetErrorText(error) << '\n';
		Pa_Terminate();
		if (critical) {
			exit(EXIT_FAILURE);
		}
	}
}
