// git_client_pull_windows
#include <iostream>
#include <fstream>
#include <string>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Windows Headers
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>

#define BUFFER_SIZE 1024

// The following two functions initializes and cleans up Winsock 2.2 respectively
int sockInit()
{
	#ifdef _WIN32
		WSADATA wsa_data;
		return WSAStartup(MAKEWORD(2,2), &wsa_data);
	#else
		return 0;
	#endif
}

int sockQuit()
{
	#ifdef _WIN32
		return WSACleanup();
	#else
		return 0;
	#endif
}

bool valid(const std::string & str){
    if(str.length() >= 30) return false;
        for(int i = 0; i < str.length(); i++){
            if(!isalnum(str[i])){
                //maybe allow certain special characters here but idk
                return false;
            }
        }
    return true;
}

/* Sends a file with the given filename from the directory specified in dir to the server
   Searches through current directory by default. */
bool upload_file(const SOCKET sd, const std::string & filename, const std::string server_dir,
                    const std::string dir = "./") {
    std::string word, path = dir;
    std::ifstream infile;
    char buffer[BUFFER_SIZE];
    int n;

    if (dir.back() != '/') path += '/';
    path += filename;

    infile = std::ifstream(path);
    if (!infile.good())
    {
        std::cerr << path << " does not exist or is empty\n";
        return false;
    }

    // Notify the server a file is being uploaded.
    send(sd, "UPLOAD", strlen("UPLOAD"), 0);

    // Wait for the server to read before sending more data.
    n = recv(sd, buffer, strlen("UPLOAD found"), 0);
    buffer[n] = '\0';

    // Send the name of the file and the server directory
    send(sd, filename.c_str(), filename.size(), 0);
    recv(sd, buffer, 1, 0);

    if (server_dir.back() != '/') strcpy(buffer, std::string(server_dir + '/').c_str());
    else strcpy(buffer, server_dir.c_str());
    assert(strlen(buffer) > 0);
    send(sd, buffer, strlen(buffer), 0);
    recv(sd, buffer, 1, 0);

    // Send the contents of the file into the client socket
    while (infile.getline(buffer, BUFFER_SIZE))
    {
        if (send(sd, buffer, strlen(buffer), 0) < 0)
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

int main(int argc, char * argv[])
{
    // Check command line arguments
    if (argc != 5 && argc != 6)
    {
        std::cerr << "Usage: " << argv[0] << " <file> <username> <password> <server directory>\n"
                    << "or " << argv[0] << " <file> <username> <password> <server directory> <client directory>\n";
                    return EXIT_FAILURE;
    }

    const std::string username = argv[2];
    const std::string password = argv[3];
    const std::string port = "8123";
    const std::string name = "161.35.48.64";   // Digital Ocean Server
    int winsock = sockInit();
    struct addrinfo hints;
    struct addrinfo *client_info;
    struct sockaddr_in server;
    SOCKET sd;
    char buffer[BUFFER_SIZE];

    if(!valid(username) || !valid(password)) {
        std::cerr << "Error: Invalid username/password!" << std::endl;
        return EXIT_FAILURE;
    }

    if(strstr(argv[4], "..")) {
        std::cerr << "Error: Invalid server path! Cannot use \"..\"!" << std::endl;
        return EXIT_FAILURE;
    }

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    // get ready to connect
    if (getaddrinfo(name.c_str(), port.c_str(), &hints, &client_info) != 0)
    {
        std::cerr << "getaddrinfo() failed!\n";
    }

    /* create TCP client socket (endpoint) */
    sd = socket(client_info->ai_family, client_info->ai_socktype, client_info->ai_protocol);
    if (sd == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    server = *(sockaddr_in *)client_info->ai_addr;

    std::cout << "connecting to server.....\n";
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        std::perror("connect() failed\n");
        return EXIT_FAILURE;
    }


    //verify user credentials before uploading the file
    std::string credentials = username + "," + password;
    std::string credentials_size = std::to_string(credentials.length());
    if(credentials_size.length() < 2){
        credentials_size = " " + credentials_size;
    }
    std::string send_credentials = "CREDENTIALS " + credentials_size;
    send(sd, send_credentials.c_str(), send_credentials.length(), 0);

    int n = recv(sd, buffer, strlen("CREDENTIALS found"), 0);
    send(sd, credentials.c_str(), credentials.length(), 0);
    n = recv(sd, buffer, strlen("succes"), 0);

    //failed
    buffer[n] = '\0';
    if(strcmp(buffer, "failed") == 0){
        std::cerr << "Error: Incorrect login credentials!" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Successfully logged in" <<std::endl;

    if (argc == 5 && !upload_file(sd, argv[1], argv[4]))
    {
        std::cerr << "Failed to send file.\n";
        return EXIT_FAILURE;
    }
    else if (argc == 6 && !upload_file(sd, argv[1], argv[4], argv[5]))
    {
        std::cerr << "Failed to send file.\n";
        return EXIT_FAILURE;
    }
    
    closesocket(sd);
    freeaddrinfo(client_info);
	
	sockQuit();
    return EXIT_SUCCESS;
}