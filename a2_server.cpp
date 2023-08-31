#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// using namespace std;

// struct containing output for each RLE string and frequencies
struct answer
{
  std::string RLE;
  std::vector<int> freq;
};

struct arg
{
  std::string s;
  std::string RLE;
  std::vector<int> freq;
};

// function running the RLE returning an answer struct
answer runEncoder(std::string input)
{
  std::string result;
  std::vector<int> frequencies;

  int length = input.length();
  int pos1 = 0, pos2 = 1, count = 1;

  // iterate through length of string
  for (int i = 0; i < length; i++) {
    // assign first two characters of string
    char position1 = input[pos1];
    char position2 = input[pos2];

    // check if chars are matching and begin counting
    if (position1 == position2)
    {
      count++;
    }
    else
    {
      // add to string if count greater than 1
      if (count > 1)
      {
        frequencies.push_back(count); // add frequency count to vector
        result += position1;
        result += position1;

        // if char not repeated, add one char to string
      }
      else
      {
        result += position1;
      }
      count = 1; // reset count
    }
    // increment through the input string
    pos1++;
    pos2++;
  }

  // create answer struct and assign values
  answer ans;
  ans.RLE = result;
  ans.freq = frequencies;

  return ans;
}

// referenced from thread2 file in Canvas
void *rleThread(void *x_void_ptr)
{
  struct arg *x_ptr = (struct arg *)x_void_ptr;
  x_ptr->RLE = runEncoder(x_ptr->s).RLE;
  x_ptr->freq = runEncoder(x_ptr->s).freq;
  return NULL;
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno, clilen, n;
  struct sockaddr_in serv_addr, cli_addr;
  if (argc < 2)
  {
    std::cerr << "ERROR, no port provided\n";
    exit(0);
  }
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    std::cerr << "ERROR opening socket";
    exit(0);
  }

   // Populate the sockaddr_in structure
  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

   // Bind the socket with the sockaddr_in structure
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    std::cerr << "ERROR on binding";
    exit(0);
  }
   // Set the max number of concurrent connections
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);

  while (true) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
    int pid = fork();
    if (pid == 0) {
      // read the size of the msg
      int inputsize;
      n = read(newsockfd, &inputsize, sizeof(int));
      if (n < 0) {
        std::cerr << "Error reading the inputsize from socket" << std::endl;
        exit(0);
      }

      char *input = new char[inputsize];
      bzero(input, inputsize);

      // read the actual msg
      n = read(newsockfd, input, inputsize);
      if (n < 0)
      {
        std::cerr << "Error reading msg from socket" << std::endl;
        exit(0);
      }

      answer ans = runEncoder(std::string(input));
      std::string x = ans.RLE;
      std::vector<int> y = ans.freq;

      std::string newstring = x;
      int rle_size = newstring.length() + 1;
      n = write(newsockfd, &rle_size, sizeof(int));
      if (n < 0)
      {
        std::cerr << "Error writing rle size to socket" << std::endl;
        exit(0);
      }

      n = write(newsockfd, newstring.c_str(), rle_size);
      if (n < 0)
      {
        std::cerr << "Error writing rle to socket" << std::endl;
        exit(0);
      }

      int freq_size = y.size();
      n = write(newsockfd, &freq_size, sizeof(int));
      if (n < 0)
      {
        std::cerr << "Error writing freq size to socket" << std::endl;
        exit(0);
      }

      n = write(newsockfd, y.data(), freq_size * sizeof(int));
      if (n < 0)
      {
        std::cerr << "Error writing freq to socket" << std::endl;
        exit(0);
      }

      close(newsockfd);
      _exit(0);
    }
  }
  close(sockfd);
  return 0;
}
