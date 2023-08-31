#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

struct thread
{
    std::string s;
    std::string RLE;
    std::vector<int> freq;
    int argc;
    char **argv;
    int id;
};

void *messenger(void *arg_ptr)
{
    thread *x_ptr = static_cast<thread *>(arg_ptr);
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(x_ptr->argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }

    server = gethostbyname(x_ptr->argv[1]);
    if (server == NULL)
    {
        std::cerr << "ERROR, no such host" << std::endl;
        exit(0);
    }

    //intialize all data members
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    //copy info from address
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    //set port number to port number we receive
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR connecting";
        exit(0);
    }
    
    std::string message = x_ptr->s;
    int size = message.length() + 1;
    char *msg = new char[size];
    strcpy(msg, message.c_str());

    // write size of message
    n = write(sockfd, &size, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket" << std::endl;
        exit(0);
    }

    // write message
    n = write(sockfd, msg, size);
    if (n < 0)
    {
        std::cerr << "ERROR writing to socket" << std::endl;
        exit(0);
    }

    int rle_string_size;
    n = read(sockfd, &rle_string_size, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    char *rlestring = new char[rle_string_size];
    bzero(rlestring, rle_string_size);

    // read the RLE string
    n = read(sockfd, rlestring, rle_string_size);
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    x_ptr->RLE = std::string(rlestring);

    int freq_size;
    n = read(sockfd, &freq_size, sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    int *freq = new int[freq_size];

    // read the freq
    n = read(sockfd, freq, freq_size * sizeof(int));
    if (n < 0)
    {
        std::cerr << "ERROR reading from socket" << std::endl;
        exit(0);
    }

    // store freq 
    std::vector<int> freqVector(freq, freq + freq_size);
    x_ptr->freq = freqVector;

    close(sockfd);

    return NULL;
}

// output the results
void print(thread *x_ptr, std::vector<std::string> inputs){
    int size = inputs.size();
    for (int i = 0; i < size; i++) {
        std::cout << "Input string: " << inputs[i] << std::endl;
        std::cout << "RLE String: " << x_ptr[i].RLE << std::endl;
        std::cout << "RLE Frequencies:";
        int freqSize = x_ptr[i].freq.size();
        for (int j = 0; j < freqSize; j++)
        {
            std::cout << " " << x_ptr[i].freq[j];
        }
        std::cout << std::endl << std::endl;
    }

}

int main(int argc, char *argv[]) {
    std::vector<std::string> inputs;
    std::string input;

    // storing input in vector
    while (std::cin >> input)
        inputs.push_back(input);

    int size = inputs.size();
    pthread_t tid[size];
    static thread *x_ptr = new thread[size];

    // create threads
    for (int i = 0; i < size; i++) {
        x_ptr[i].argc = argc;
        x_ptr[i].argv = argv;
        x_ptr[i].s = inputs[i];
        if (pthread_create(&tid[i], NULL, messenger, &x_ptr[i]) != 0)
        {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    // join threads
    for (int i = 0; i < size; i++)
        pthread_join(tid[i], NULL);

    // print function for output
    print(x_ptr, inputs);

    delete[] x_ptr;

    return 0;
}
