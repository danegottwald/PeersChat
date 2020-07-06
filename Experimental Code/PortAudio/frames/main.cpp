#include <iostream>
#include <portaudio.h>

#define NUM_SECONDS 1

long long FRAMES = 0;
static int patestCallback(const void* input,
                          void *output,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags status_flags,
                          void *userData)
{
	FRAMES += framesPerBuffer;
	return 0;
}

int main()
{
	// Initialize
	PaError val = Pa_Initialize();
	if(val != paNoError)
		return 1;

	// Open Stream
	PaStream *stream = NULL;
	val = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, 48000, 0, patestCallback, NULL);
	if(val != paNoError)
	{
		std::cerr << "Opened with error " << Pa_GetErrorText(val) << std::endl;
		Pa_Terminate();
		return 2;
	}

	// Start Stream
	val = Pa_StartStream(stream);
	if(val != paNoError)
	{
		std::cerr << "Start with error " << Pa_GetErrorText(val) << std::endl;
		Pa_Terminate();
		return 3;
	}

	// Sleep Main Thread
	Pa_Sleep(NUM_SECONDS * 1000);

	// End Stream
	Pa_StopStream(stream);
	if(val != paNoError)
	{
		std::cerr << "Stop with error " << Pa_GetErrorText(val) << std::endl;
		Pa_Terminate();
		return 4;
	}

	// Print Out Number of Frames
	std::cout << "NUMBER FRAMES = " << FRAMES << std::endl;

	// End Program
	Pa_CloseStream(stream);
	stream = NULL;
	Pa_Terminate();
	return 0;
}
