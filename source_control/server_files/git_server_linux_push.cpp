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

bool download_file(int sd, const std::string & filename)
{
    // open a new file with the same name in a specific folder
  std::ofstream outfile(filename);//"./server_test_files/" + filename);
    if (!outfile.good())
    {
        std::cerr << "Can't open " << filename << " to write.\n";
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

bool validate_credentials(std::string username, std::string password);
int main()
{
    const std::string port = "8123";

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

    /* Create the listener socket as TCP socket (SOCK_STREAM) */
    /* here, the sd is a socket descriptor (part of the fd table) */
    int sd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if ( sd == -1 )
    {
        std::perror( "socket() failed" );
        return EXIT_FAILURE;
    }

    /* attempt to bind (or associate) port 8123 with the socket */
    if ( bind(sd, server_info->ai_addr, server_info->ai_addrlen) == -1 )
    {
        std::perror( "bind() failed" );
        return EXIT_FAILURE;
    }

    /* identify this port as a TCP listener port */
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
                buffer[11] = '\0';
                assert(strcmp(buffer, "CREDENTIALS") == 0);
		int credentials_size = 0;
		if(buffer[13] != ' ')
		  credentials_size = (buffer[13]-'0')*10;
		credentials_size += (buffer[14]-'0');

		send(newsd, "CREDENTIALS found", strlen("CREDENTIALS found"), 0);
                n = recv(newsd, buffer, credentials_size, 0);
		buffer[n] = '\0';

		//cstring to string
		std::string username = "";
		std::string password = "";
		bool is_user = true;

		for(int i = 0; i < n; i++){
		  if(buffer[i] == ','){
		    is_user = false;
		    continue;
		  }
		  if(is_user) username += buffer[i];
		  else password += buffer[i];
		}
		
		//validates
		if(!validate_credentials(username, password)){
		  send(newsd, "failed", strlen("failed"), 0);
		  continue;
		}
		send(newsd, "succes", strlen("succes"), 0);
		n = recv(newsd, buffer, BUFFER_SIZE - 1, 0);
		buffer[n] = '\0';
		
		std::cout << buffer << std::endl;
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
			filename = "/home/"+username+"/"+filename;
                        download_file(newsd, filename);
                    }
                }
                

                // send ACK message back to client

                n = send( newsd, "ACK\n", 4, 0 );

                if ( n != 4 )
                {
                    perror( "send() failed" );
                    return EXIT_FAILURE;
                }
            }
        }
        while ( n > 0 );

        close( newsd );
    }

    return EXIT_SUCCESS;
}
