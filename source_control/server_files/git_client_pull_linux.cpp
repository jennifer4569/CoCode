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

/* Download a file from the server and save it in a directory specified by dir */
bool download_file(int sd, const std::string & dir = "./")
{
    std::string path, filename;
    std::ofstream outfile;
    char buffer[BUFFER_SIZE];
    int n;

    send(sd, "DOWNLOAD", strlen("DOWNLOAD"), 0);

    recv(sd, buffer, strlen("DOWNLOAD found"), 0);

    // Next line will be the file name.
    n = recv(sd, buffer, BUFFER_SIZE - 1, 0);
    buffer[n] = '\0';
    
    if (n == -1)
    {
        std::cerr << "recv() failed\n";
        return false;
    }
    else if (n == 0)
    {
        std::cerr << "missing file name\n";
        return false;
    }

    filename = buffer;

    std::cout << "CLIENT: Downloading file " << filename << std::endl;

    // open a new file with the same name in a specific folder
    path = dir;
    if (dir.back() != '/') path += '/';
    path += filename;

    outfile = std::ofstream(path);
    if (!outfile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return false;
    }

    // Read the contents of the file and copy them into outfile
    do
    {
        send(sd, " ", 1, 0);     // send blank message to undo block from client
        n = recv(sd, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';

        std::cout << "CLIENT: " << buffer << std::endl;

        if (n == -1)
        {
            std::cerr << "recv() failed!\n";
            return false;
        }
        else if (n == 0)
        {
            std::cout << "CLIENT: File read complete.\n";
        }
        else
        {
            outfile << buffer;
        }
    } while (n > 0);

    outfile.close();
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
    
    if (!download_file(sd, "./test_client_repo/"))
    {
        std::cerr << "Failed to send file.\n";
        return EXIT_FAILURE;
    }
    
    close(sd);
    freeaddrinfo(client_info);

    return EXIT_SUCCESS;
}