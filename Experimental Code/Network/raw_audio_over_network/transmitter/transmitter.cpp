#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <portaudio.h>

#define TIME 30

long long SIZE = 0;
int sock = -1;

static int patestCallback(const void* input,
                          void* output,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags status_flags,
                          void *userData)
{
  SIZE += framesPerBuffer;
  if( (framesPerBuffer*4) != send(sock, (void*) input, framesPerBuffer*4, 0))
    std::cerr << "WARNING: Sent wrong number of bytes." << std::endl;
  return 0;
}


int main(const int argc, char *argv[])
{
  if(argc != 3)
  {
    fprintf(stderr, "ERROR: %s {IP} {PORT}\n", argv[0]);
    return 1;
  }

  int valread; 
  struct sockaddr_in serv_addr; 
  char const *msg = argv[1];
  char buffer[1024] = {0}; 
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)  
  {   
      printf("\n Socket creation error \n"); 
      return -1; 
  }   
 
  serv_addr.sin_family = AF_INET; 
  serv_addr.sin_port = htons(atoi(argv[2]));
  
  // Convert IPv4 and IPv6 addresses from text to binary form 
  if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)  
  {   
      printf("\nInvalid address/ Address not supported \n"); 
      return -1; 
  }   

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  
  {   
      printf("\nConnection Failed \n"); 
      return -1; 
  }   


//------------------------------------------------------------------------------
  // Version #
  std::cout << "Version: " << Pa_GetVersionText() << std::endl;

  // Initialize Port Audio
  PaError val = Pa_Initialize();
  if(val == paNoError)
    std::cout << "No Errors" << std::endl;
  else return 1;

  // Open Stream
  PaStream *stream = NULL;
  val = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, 44100, 256, patestCallback, NULL);
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

  // Transmit
  printf("BEGIN TRANSMISSION =============================\n");
  for(int i = TIME; i > 0; i--)
  {
    printf("%03d seconds remaining\n", i);
    Pa_Sleep(1000);
  }
  printf("END TRANSMISSION   =============================\n");
  std::cout << "Transmitted " << SIZE*4 << " bytes" << std::endl;

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
