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
bool download_file(int sd, std::string username)
{
    std::string filename, dir, path;
    std::ofstream outfile;
    char buffer[BUFFER_SIZE];
    int n;

    // Notify the client the command 'UPLOAD' has been received.
    send(sd, "UPLOAD found", strlen("UPLOAD found"), 0);

    /* Get the file name and directory to determine which file will be
       sent to the client. */
    n = recv(sd, buffer, BUFFER_SIZE - 1, 0);
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
    buffer[n] = '\0';
    send(sd, " ", 1, 0);     // send blank message to undo block from client
    filename = std::string(buffer);
    
    n = recv(sd, buffer, BUFFER_SIZE - 1, 0);
    if (n == -1)
    {
        std::cerr << "recv() failed\n";
        return false;
    }
    else if (n == 0)
    {
        std::cerr << "missing directory\n";
        return false;
    }
    buffer[n] = '\0';
    send(sd, " ", 1, 0);
    dir = std::string(buffer);

    path = dir;
    if (dir.back() != '/') path += '/';
    path += filename;
    
    //to home directory
    if(path[0] != '/') path = "/" + path;
    path = "/home/"+username+path;
    //std::cout << path << std::endl;
    if(strstr(path.c_str(), "..")){
      std::cerr << "Error: Invalid server path! Cannot use \"..\"!" << std::endl;
      return false;
    }
    
    //filename = buffer;
    std::cout << "SERVER: Downloading file " << filename << std::endl;

    outfile = std::ofstream(path);
    if (!outfile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return false;
    }

    do
    {
        n = recv(sd, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';

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
            std::cout << "SERVER: " << buffer;
            outfile << buffer;
        }

        send(sd, " ", 1, 0);
    } while (n > 0);

    outfile.close();
    return true;
}

// Sends a file with the given filename from the directory specified in dir to the server
void upload_file(int sd, std::string username) {
    std::string word, filename, dir, path;
    std::ifstream infile;
    char buffer[BUFFER_SIZE];
    int n;

    // Notify the client the command 'DOWNLOAD' has been received.
    send(sd, "DOWNLOAD found", strlen("DOWNLOAD found"), 0);
    
    /* Get the file name and directory to determine which file will be
       sent to the server. */
    n = recv(sd, buffer, BUFFER_SIZE - 1, 0);
    if (n == -1)
    {
        std::cerr << "recv() failed\n";
        return;
    }
    else if (n == 0)
    {
        std::cerr << "missing file name\n";
        return;
    }
    buffer[n] = '\0';
    send(sd, " ", 1, 0);

    filename = std::string(buffer);
    
    n = recv(sd, buffer, BUFFER_SIZE - 1, 0);
    if (n == -1)
    {
        std::cerr << "recv() failed\n";
        return;
    }
    else if (n == 0)
    {
        std::cerr << "missing directory\n";
        return;
    }
    buffer[n] = '\0';

    dir = std::string(buffer);

    path = dir;
    if (dir.back() != '/') path += '/';
    path += filename;

    //to home directory
    if(path[0] != '/') path = "/" + path;
    path = "/home/"+username+path;

    if(strstr(path.c_str(), "..")){
      std::cerr << "Error: Invalid server path! Cannot use \"..\"!" << std::endl;
      return;
    }
    
    std::cout << "SERVER: Opening " << path << std::endl;

    infile = std::ifstream(path);
    if (!infile.good())
    {
        std::cerr << "Can't open " << filename << " to read.\n";
        return;
    }

    // Send the contents of the file into the client socket
    
    while (infile.getline(buffer, BUFFER_SIZE - 1))
    {
        if (send(sd, buffer, strlen(buffer), MSG_MORE) < 0)
        {
            std::cerr << "send() failed\n";
            return;
        }
        // send a new line until getline() reaches end of file.
        if (!infile.eof()) send(sd, "\n", strlen("\n"), 0);

	if(!recv(sd, buffer, 1, 0) > 0){
	  break;
	}
    }
    infile.close();
    std::cout << "File successfully sent to server.\n";
    close(sd); // close socket to prevent both the sever and client from blocking
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
	      if(!print_dir(path))
		 return false;
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
	      if(!upload_repo(sd, path))
		return false;
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
bool validate_credentials(std::string username, std::string password);
		
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
        // Socket for client
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
                buffer[11] = '\0';
		printf("%s\n", buffer);
		if(strcmp(buffer, "CREDENTIALS") != 0)
		  break;
		
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
		

		printf( "SERVER: Rcvd message from %s: %s\n",
                            inet_ntoa( (struct in_addr)client.sin_addr ),
                            buffer );
                if (strcmp(buffer, "UPLOAD") == 0)
                {
		  download_file(newsd, username);
                    break;
                }
                if (strcmp(buffer, "DOWNLOAD") == 0)
                {
		  upload_file(newsd, username);
                    break;
                }
            }
        }
        while ( n > 0 );

        close( newsd );
    }

    return EXIT_SUCCESS;
}
