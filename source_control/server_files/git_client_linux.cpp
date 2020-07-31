#include <iostream>
#include <fstream>
#include <string>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Linux Headers
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

#if 0
    struct hostent * hp = gethostbyname( "localhost" );  /* 127.0.0.1 */
    struct hostent * hp = gethostbyname( "127.0.0.1" );  // IPv4
    struct hostent * hp = gethostbyname( "::1" );  // IPv6
#endif

/* Convenient way to transfer files. Unfortunately, there is no way
    to do this between sockets. */
/*
std::string src("copypasta.txt"), dst("./test_files/copypasta.txt");

std::ifstream infile(src.c_str(), std::ios::binary);
if (!infile.good()) {
    std::cerr << "Can't open " << src << " to read.\n";
    return EXIT_FAILURE;
}

std::ofstream outfile(dst.c_str(), std::ios::binary);
if (!outfile.good()) {
    std::cerr << "Can't open " << dst << " to write.\n";
    return EXIT_FAILURE;
}

outfile << infile.rdbuf();
*/

// Sends a file with the given filename to the server
bool upload_file(int sd, const std::string & filename) {
    char buffer[BUFFER_SIZE];

    // Notify the server a file is being sent.
    send(sd, "FILE", std::strlen("FILE"), 0);

    // Wait for the server to read before sending more data.
    int n = recv(sd, buffer, strlen("FILE found"), 0);

    std::cout << "bytes received: " << n << std::endl;

    buffer[n] = '\0';
    std::cout << buffer << std::endl;
    assert(n == strlen("FILE found"));

    send(sd, filename.c_str(), filename.size(), 0);
    assert(recv(sd, buffer, BUFFER_SIZE, 0) > 0);

    std::ifstream infile(filename);
    std::string word;

    if (!infile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return false;
    }

    // Send the contents of the file into the client socket
    
    while (infile.getline(buffer, BUFFER_SIZE))
    {
        if (send(sd, buffer, strlen(buffer), 0) < 0)
        {
            std::cerr << "send() failed\n";
            return false;
        }
        // send a new line.
        send(sd, " ", strlen(" "), 0);
        assert(recv(sd, buffer, 1, 0) > 0);
    }
    infile.close();
    return true;
}

int main()
{
    const std::string port = "8123";
    struct addrinfo hints;
    struct addrinfo *client_info;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    const std::string name = "127.0.0.1";   // localhost for testing

    // get ready to connect
    if (getaddrinfo(name.c_str(), port.c_str(), &hints, &client_info) != 0)
    {
        std::cerr << "getaddrinfo() failed!\n";
    }

    /* create TCP client socket (endpoint) */
    int sd = socket(client_info->ai_family, client_info->ai_socktype, client_info->ai_protocol);
    if (sd == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server = *(sockaddr_in *)client_info->ai_addr;

    std::cout << "connecting to server.....\n";
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        std::perror("connect() failed\n");
        return EXIT_FAILURE;
    }
    
    if (upload_file(sd, "copypasta.txt"))
    {
        std::cout << "File successfully sent to server.\n";
    }
    else
    {
        std::cerr << "Failed to send file.\n";
        return EXIT_FAILURE;
    }
    /*
    int n = recv(sd, buffer, BUFFER_SIZE - 1, 0);    // BLOCKING

    if ( n == -1 )
    {
        perror( "recv() failed" );
        return EXIT_FAILURE;
    }
    else if ( n == 0 )
    {
        printf( "Rcvd no data; also, server socket was closed\n" );
    }
    else  // n > 0
    {
        buffer[n] = '\0';    // assume we rcvd text-based data
        printf( "Rcvd from server: %s\n", buffer );
    }
    */
    sleep(2);
    close(sd);
    freeaddrinfo(client_info);

    return EXIT_SUCCESS;
}