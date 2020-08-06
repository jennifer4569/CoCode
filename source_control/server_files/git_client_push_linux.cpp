// git_client_push_linux
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
    to open files between sockets. */
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

// Sends a file with the given filename from the directory specified in dir to the server
bool upload_file(int sd, const std::string & filename, const std::string dir = "./") {
    std::string word, path = dir;
    std::ifstream infile;
    char buffer[BUFFER_SIZE];
    int n;

    if (dir.back() != '/') path += '/';
    path += filename;

    infile = std::ifstream(dir + filename);
    if (!infile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return false;
    }

    // Notify the server a file is being uploaded.
    send(sd, "UPLOAD", strlen("UPLOAD"), 0);

    // Wait for the server to read before sending more data.
    n = recv(sd, buffer, strlen("UPLOAD found"), 0);
    buffer[n] = '\0';

    // Send the name of the file
    send(sd, filename.c_str(), filename.size(), 0);
    assert(recv(sd, buffer, BUFFER_SIZE, 0) > 0);

    // Send the contents of the file into the client socket
    while (infile.getline(buffer, BUFFER_SIZE))
    {
        if (send(sd, buffer, strlen(buffer), MSG_MORE) < 0)
        {
            std::cerr << "send() failed\n";
            return false;
        }
        // send a new line until getline() reaches eof.
        if (!infile.eof()) send(sd, "\n", strlen("\n"), 0);
        assert(recv(sd, buffer, 1, 0) > 0);
    }
    infile.close();
    std::cout << "File successfully sent to server.\n";
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

    const std::string name = "127.0.0.1";   //"161.35.48.64";   // Digital Ocean Server

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
    
    if (!upload_file(sd, "copypasta.txt", "./test_client_repo/"))
    {
        std::cerr << "Failed to send file.\n";
        return EXIT_FAILURE;
    }
    
    close(sd);
    freeaddrinfo(client_info);

    return EXIT_SUCCESS;
}