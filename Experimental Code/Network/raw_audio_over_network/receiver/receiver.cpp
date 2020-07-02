#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <portaudio.h>

#define PORT 8080

int main(const int argc, char *argv[])
{
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);
  char buffer[4096] = {0};
  char const *hello = "Server got your message";

  // Creating socket file descriptor 
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
  {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080 
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    perror("setsockopt"); 
    exit(EXIT_FAILURE); 
  }   
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons( PORT );  
  
  // Forcefully attaching socket to the port 8080 
  if (bind(server_fd, (struct sockaddr *)&address,  
                               sizeof(address))<0) 
  {   
    perror("bind failed"); 
    exit(EXIT_FAILURE); 
  }   
  if (listen(server_fd, 3) < 0)  
  {   
    perror("listen"); 
    exit(EXIT_FAILURE); 
  }   

  // Accept
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
  {
    perror("accept");
    exit(EXIT_FAILURE);
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
  val = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, 44100, 256, NULL, NULL);
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

  // Receive
  printf("BEGIN RECEPTION    =============================\n");
  long long SIZE = 0;
  while(true)
  {
    valread = read( new_socket , buffer, 4096);
    SIZE += valread;
    val = Pa_WriteStream(stream, buffer, valread / 4);
    if(val != paNoError)
    {
      std::cerr << "Write to Stream with error" << std::endl;
      fprintf(stderr, "%s\n", Pa_GetErrorText(val));
//      Pa_Terminate();
//      return 8;
    }
  }
  printf("END RECEPTION      =============================\n");
  std::cout << "Receieved   " << SIZE << " bytes" << std::endl;

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
