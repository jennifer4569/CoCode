// git_server_linux
#include <iostream>
#include <fstream>
#include <string>
//#include <filesystem>     // C++17

// C headers
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Linux Headers
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

#if 0
    struct hostent * hp = gethostbyname( "localhost" );  /* 127.0.0.1 */
    struct hostent * hp = gethostbyname( "127.0.0.1" );  // IPv4
    struct hostent * hp = gethostbyname( "::1" );  // IPv6
#endif

/* Download a file from a client socket into a directory in dir
Default directory is the current working directory */
bool download_file(int sd, const std::string & filename, const std::string & dir = "./")
{
    // open a new file with the same name in a specific folder
    std::string path = dir;
    if (dir.back() != '/') path += '/';
    path += filename;

    std::ofstream outfile(path);
    if (!outfile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return false;
    }

    std::cout << "SERVER: Downloading file " << filename << std::endl;

    send(sd, " ", strlen(" "), 0);

    int n;
    do
    {
        char buffer[BUFFER_SIZE];

        send(sd, " ", 1, 0);     // send blank message to undo block from client
        n = recv(sd, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';

        std::cout << "SERVER: " << buffer << std::endl;

        if (n == -1)
        {
            std::cerr << "recv() failed!\n";
            return false;
        }
        else if (n == 0)
        {
            std::cout << "SERVER: File read complete.\n";
        }
        else
        {
            outfile << buffer;
        }
    } while (n > 0);

    outfile.close();
    return true;
}

bool upload_file(int sd, const std::string & filename) {
    char buffer[BUFFER_SIZE];

    // Notify the server a file is being sent.
    send(sd, "FILE", std::strlen("FILE"), 0);

    // Wait for the server to read before sending more data.
    int n = recv(sd, buffer, strlen("FILE found"), 0);
    buffer[n] = '\0';

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
        if (send(sd, buffer, strlen(buffer), MSG_MORE) < 0)
        {
            std::cerr << "send() failed\n";
            return false;
        }
        // send a new line until the getline() reaches eof.
        if (!infile.eof()) send(sd, "\n", strlen("\n"), 0);
        assert(recv(sd, buffer, 1, 0) > 0);
    }
    infile.close();
    return true;
}

// Prints all folders and files within the given path name in pre-order DFS
bool print_dir(std::string name)
{
    DIR * pdir;
    struct dirent * dir;
    struct stat stat_buf;

    if((pdir = opendir(name.c_str())) == NULL) 
    {
        std::cerr << "Error: Unable to open directory\n";
        return false;
    }

    while((dir = readdir(pdir)) != NULL)
    {
        // path represents the path to the file
        std::string path = name;
        
        // Ensure pathname ends with '/' before adding a file/folder name
        if(path.back() != '/') path += '/';
        path += dir->d_name;

        if (!stat(path.c_str(), &stat_buf))
        {
            if (S_ISDIR(stat_buf.st_mode) && path.back() != '.')
            {
                

                // Print other folders excluding the cwd and parent directory
                assert(print_dir(path));
            }
            else if (S_ISREG(stat_buf.st_mode))
            {
                std::cout << "File Name: " << dir->d_name << std::endl;
                std::cout << "File Size: " << stat_buf.st_size << std::endl;
                std::cout << "Path: " << path << std::endl;
                std::cout << "Last Access: " << ctime(&stat_buf.st_atime) << std::endl;
            }
        }
        else
        {
            std::cerr << "Cannot open " << path << std::endl;
            return false;
        }
    }
    closedir(pdir);
    return true;
}

bool upload_repo(int sd, const std::string & name)
{
    DIR * pdir;
    struct dirent * dir;
    struct stat stat_buf;

    if((pdir = opendir(name.c_str())) == NULL) 
    {
        std::cerr << "Error: Unable to open directory\n";
        return false;
    }

    while((dir = readdir(pdir)) != NULL)
    {
        // path represents the path to the file
        std::string path = name;
        
        // Ensure pathname ends with '/' before adding a file/folder name
        if(path.back() != '/') path += '/';
        path += dir->d_name;

        if (!stat(path.c_str(), &stat_buf))
        {
            if (S_ISDIR(stat_buf.st_mode) && path.back() != '.')
            {
                // Create a folder

                // Print other folders excluding the cwd and parent directory
                assert(upload_repo(sd, path));
            }
            else if (S_ISREG(stat_buf.st_mode))
            {
                std::cout << "File Name: " << dir->d_name << "\n";
                std::cout << "File Size: " << stat_buf.st_size << "\n";
                std::cout << "Path: " << path << "\n";
                std::cout << "Last Access: " << ctime(&stat_buf.st_atime) << std::endl;
            }
        }
    }

    return true;
}

int main()
{
    const std::string port = "8123";
    const std::string ip4 = "161.35.48.64";

    // Address Structures
    struct addrinfo hint, *server_info;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;   // IPv4
    hint.ai_socktype = SOCK_STREAM;     // TCP
    hint.ai_flags = AI_PASSIVE;
    /* By using the AI_PASSIVE flag, I’m telling the program to
    bind to the IP of the host it’s running on.
    If you want to bind to a specific local IP address,
    drop the AI_PASSIVE and put an IP address
    in for the first argument to getaddrinfo() */

    if (getaddrinfo(NULL, port.c_str(), &hint, &server_info) != 0)
    {
        std::cerr << "getaddrinfo() failed!\n";
    }

    // Create the listener socket as TCP socket (SOCK_STREAM)
    // here, the sd is a socket descriptor (part of the fd table)
    int sd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if ( sd == -1 )
    {
        std::perror( "socket() failed" );
        return EXIT_FAILURE;
    }

    // attempt to bind (or associate) port 8123 with the socket
    if ( bind(sd, server_info->ai_addr, server_info->ai_addrlen) == -1 )
    {
        std::perror( "bind() failed" );
        return EXIT_FAILURE;
    }

    // identify this port as a TCP listener port
    if ( listen( sd, 5 ) == -1 )
    {
        std::perror( "listen() failed" );
        return EXIT_FAILURE;
    }

    std::cout << "SERVER: TCP listener socket bound to port " << port << std::endl;

    FILE * socket_ptr = fdopen(sd, "r+");
    setvbuf(socket_ptr, NULL, _IONBF, 0);

    struct sockaddr_in client;
    int fromlen = sizeof( client );

    int n;
    char buffer[ BUFFER_SIZE ];

    while ( 1 )
    {
        std::cout << "SERVER: Blocked on accept()\n";
        int newsd = accept( sd, (struct sockaddr *)&client, (socklen_t *)&fromlen );

        std::cout << "SERVER: Accepted new client connection on newsd " << newsd << std::endl;

        /* we have just established a TCP connection between server and a
            remote client; below implements the application-layer protocol */

        /* the sd variable is listener socket descriptor that we use to accept
            new incoming client connections (port 8123) */

        /* the newsd variable is the means of communicating via recv()/send()
            or read()/write() with the connected client */

        do
        {
            std::cout << "SERVER: Blocked on recv()\n";

            /* recv() call will block until we receive data (n > 0)
                or there's an error (n == -1)
                or the client closed its socket (n == 0) */
        
            n = recv( newsd, buffer, BUFFER_SIZE - 1, 0 );   // or read()

            if ( n == -1 )
            {
                perror( "recv() failed\n" );
                return EXIT_FAILURE;
            }
            else if ( n == 0 )
            {
                printf( "SERVER: Rcvd 0 from recv(); closing socket...\n" );
            }
            else // n > 0
            {
                buffer[n] = '\0';   // assume this is text data
                printf( "SERVER: Rcvd message from %s: %s\n",
                            inet_ntoa( (struct in_addr)client.sin_addr ),
                            buffer );
                if (strcmp(buffer, "FILE") == 0)
                {
                    send(newsd, "FILE found", strlen("FILE found"), 0);
                    // Next line will be the file name
                    n = recv(newsd, buffer, BUFFER_SIZE - 1, 0);
                    buffer[n] = '\0';
                    
                    if (n == -1)
                    {
                        std::cerr << "recv() failed\n";
                        return EXIT_FAILURE;
                    }
                    else if (n == 0)
                    {
                        std::cerr << "missing file name\n";
                        return EXIT_FAILURE;
                    }
                    else {
                        std::string filename(buffer);
                        download_file(newsd, filename, "./test_server_repo/");
                    }
                }
            }
        }
        while ( n > 0 );

        close( newsd );
    }

    return EXIT_SUCCESS;
}
