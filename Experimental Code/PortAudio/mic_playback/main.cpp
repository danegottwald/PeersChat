#include <iostream>
#include <portaudio.h>
#include <cstring>

#define NUM_SECONDS 20
#define VOLUME_MULTIPLIER 0.75 //75% volume

long long SIZE = 0;

static int patestCallback(const void* input,
                          void* output,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags status_flags,
                          void *userData)
{
  float* in = (float*) input;
  float* out = (float*) output;
  SIZE += framesPerBuffer;
  memcpy(out, in, framesPerBuffer * 4);
  for(unsigned int i = 0; i < framesPerBuffer; ++i)
    out[i] *= VOLUME_MULTIPLIER;
  return 0;
}

int main()
{
  // Version #
  std::cout << "Version: " << Pa_GetVersionText() << std::endl;

  // Initialize Port Audio
  PaError val = Pa_Initialize();
  if(val == paNoError)
    std::cout << "No Errors" << std::endl;
  else return 1;

  // How many devices available
  std::cout << "# of Devices: " << Pa_GetDeviceCount() << std::endl;

  // What are the defaults?
  PaDeviceIndex default_in, default_out;
  default_in = Pa_GetDefaultInputDevice();
  default_out = Pa_GetDefaultOutputDevice();
  std::cout << "Default In: " << default_in << "\nDefault Out: " << default_out<<std::endl;

  // Open Stream
  PaStream *stream = NULL;
  val = Pa_OpenDefaultStream(&stream, 1, 1, paFloat32, 44100, 0, patestCallback, NULL);
  if(val != paNoError)
  {
    std::cerr << "Opened with error ";
    fprintf(stderr, "%s\n", Pa_GetErrorText(val));
    Pa_Terminate();
    return 3;
  }

  // Start Stream
  val = Pa_StartStream(stream);
  if(val != paNoError)
  {
    std::cerr << "Start with error ";
    fprintf(stderr, "%s\n", Pa_GetErrorText(val));
    Pa_Terminate();
    return 4;
  }

  // Sleep main thread so stream stays up
  std::cout << "\nBegin Microphone Playback\n"
                 "-------------------------"<< std::endl;
  Pa_Sleep(NUM_SECONDS * 1000);
  std::cout << "BYTES: " << SIZE*4 << std::endl;

  // End Stream
  Pa_StopStream(stream);
  if(val != paNoError)
  {
    std::cerr << "Stop with error ";
    fprintf(stderr, "%s\n", Pa_GetErrorText(val));
    Pa_Terminate();
    return 4;
  }

  // End Program
  Pa_CloseStream(stream);
  stream = NULL;
  Pa_Terminate();
  return 0;
}

