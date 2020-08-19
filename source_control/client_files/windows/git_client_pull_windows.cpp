// git_client_pull_windows

// Windows Headers
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>

#include <iostream>
#include <fstream>
#include <string>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define BUFFER_SIZE 1024

// The following two functions initializes and cleans up Winsock 2.2 respectively
int sockInit(void)
{
	#ifdef _WIN32
		WSADATA wsa_data;
		return WSAStartup(MAKEWORD(2,2), &wsa_data);
	#else
		return -1;
	#endif
}

int sockQuit(void)
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

/* Download a file from the server and saves it in client_dir.
   Uses current directory by default.*/
bool download_file(SOCKET sd, const std::string & filename, const std::string & dir,
                    const std::string & client_dir = "./")
{
    std::string path;
    std::ofstream outfile;
    char buffer[BUFFER_SIZE];
    int n;

    send(sd, "DOWNLOAD", strlen("DOWNLOAD"), 0);

    recv(sd, buffer, strlen("DOWNLOAD found"), 0);

    // Send the name of the file and the server directory
    std::string file = filename.substr(filename.find_last_of("/\\")+1);
    std::cout << file << std::endl;
    send(sd, file.c_str(), file.size(), 0);
    recv(sd, buffer, 1, 0);

    if (dir.back() != '/') strcpy(buffer, std::string(dir + '/').c_str());
    else strcpy(buffer, dir.c_str());
    assert(strlen(buffer) > 0);
    send(sd, buffer, strlen(buffer), 0);

    std::cout << "CLIENT: Downloading file " << filename << std::endl;

    // open a new file with the same name in a specific folder
    path = client_dir;
    if (client_dir.back() != '/') path += '/';
    path += filename;

    outfile = std::ofstream(path);
    if (!outfile.good())
    {
        std::cerr << "Can't open " << filename << " to write.\n";
        return false;
    }

    // Read the contents of the file and copy them into outfile
    do
    {
        n = recv(sd, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';

        if (n == -1)
        {
            std::cerr << "File does not exist in server or recv() failed.\n";
            return false;
        }
        else if (n == 0)
        {
            std::cout << "CLIENT: File write complete.\n";
        }
        else
        {
            std::cout << "CLIENT: " << buffer;
            outfile << buffer;
        }

        send(sd, " ", 1, 0);
    } while (n > 0);

    outfile.close();
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
    struct addrinfo hints;
    struct addrinfo *client_info;
    struct sockaddr_in server;
    SOCKET sd;
    char buffer[BUFFER_SIZE];
    int winsock = sockInit();

	if (winsock != 0)
	{
		std::cerr << "Program is not running on Windows!\n";
		return EXIT_FAILURE;
	}

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
    if (sd == INVALID_SOCKET)
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

    if (argc == 5 && !download_file(sd, argv[1], argv[4]))
    {
        std::cerr << "Failed to download file.\n";
        return EXIT_FAILURE;
    }
    else if (argc == 6 && !download_file(sd, argv[1], argv[4], argv[5]))
    {
        std::cerr << "Failed to download file.\n";
        return EXIT_FAILURE;
    }
    
    closesocket(sd);
    freeaddrinfo(client_info);

	sockQuit();
    return EXIT_SUCCESS;
}