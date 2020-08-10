// git_client_version
#include <iostream>
#include <fstream>
#include <string>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/stat.h>

// Linux Headers
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 1024

int get_version_number(const std::string &file){
  std::string filepath = file.substr(0,file.find_last_of("/\\")+1);
  std::string cocodepath = filepath+".cocode/";
  errno = 0;
  //mkdir was successful, so the directory didn't exist before
  if(mkdir(cocodepath.c_str(), 0755) == 0){
    return -1;
  }
  if(errno == EEXIST){
    std::ifstream infile = std::ifstream(cocodepath+"version.txt");
    if(!infile.good()){
      return -1;
    }
    int version_number = -1;
    infile >> version_number;
    if(version_number <= 0) version_number = -1;
    return version_number;
  }
  //unexpected error
  return -1;
}

//from https://stackoverflow.com/questions/47068948/best-practice-to-use-execvp-in-c
template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N])
{
  //assert((N > 0) && (argv[N - 1] == nullptr));

  return execvp(file, const_cast<char* const*>(argv));
}
// Sends a file with the given filename from the directory specified in dir to the server
bool version_check(int sd, const std::string & filename, const std::string &username, const std::string & password,
		   const std::string server_dir, const std::string dir = "./") {
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
	close(sd);
        return false;
    }
    infile.close();
    
    // Notify the server a file is being uploaded.
    send(sd, "VERSION CHECK", strlen("VERSION CHECK"), 0);

    // Wait for the server to read before sending more data.
    n = recv(sd, buffer, strlen("VERSION CHECK found"), 0);
    buffer[n] = '\0';

    // Send the name of the file and the server directory
    std::string file = filename.substr(filename.find_last_of("/\\")+1);
    send(sd, file.c_str(), file.size(), 0);
    recv(sd, buffer, 1, 0);

    if (server_dir.back() != '/') strcpy(buffer, std::string(server_dir + '/').c_str());
    else strcpy(buffer, server_dir.c_str());
    assert(strlen(buffer) > 0);
    send(sd, buffer, strlen(buffer), 0);
    recv(sd, buffer, 1, 0);

    std::string version_num = std::to_string(get_version_number(filename));
    send(sd, version_num.c_str(), version_num.length(), 0);
    n = recv(sd, buffer, strlen("succes"), 0);
    buffer[n] = '\0';
    std::string filepath = filename.substr(0,filename.find_last_of("/\\")+1);
    std::string fname = filename.substr(filename.find_last_of("/\\")+1);
    std::string cocodepath = filepath+".cocode/";

    if(strcmp(buffer, "succes") == 0){
      send(sd, " ", 1, 0);
      //server sends their version number
      n = recv(sd, buffer, BUFFER_SIZE, 0);
      buffer[n] = '\0';
      close(sd);

      std::ofstream outfile = std::ofstream(cocodepath + "version.txt");
      outfile << buffer << std::endl;
      outfile.close();
      
      return true;
    }
    std::cout << "CLIENT: Merge conflict detected." << std::endl;
    send(sd, " ", 1, 0);
    //server sends their version number
    n = recv(sd, buffer, BUFFER_SIZE, 0);
    buffer[n] = '\0';
    close(sd);
    
    int f = fork();
    if(f == 0){
      std::string server_version = cocodepath + buffer + "/";
      mkdir(server_version.c_str(), 0755);
      server_version+= fname;
      const char* const a[] = {"./pull.exe", server_version.c_str(), username.c_str(), password.c_str(), server_dir.c_str(), ".", "is_version", nullptr};
      execvp(a[0], a);
      return EXIT_FAILURE;
    }
    else{
      waitpid(f, NULL, 0);
      f = fork();
      if(f == 0){
	std::string client_version = cocodepath +version_num + "/";
	mkdir(client_version.c_str(), 0755);
	client_version += fname;
	const char* const a[] = {"cp", filename.c_str(), client_version.c_str(), nullptr};
	execvp(a[0], a);
	return EXIT_FAILURE;
      }
      else{
	waitpid(f, NULL, 0);
	//update version number to server's version number after done
	std::ofstream outfile = std::ofstream(cocodepath + "version.txt");
	outfile << buffer << std::endl;
	outfile.close();
	return false;
      }
    }
}

bool version_main(int argc, char * argv[])
{
    std::string username = argv[2];
    std::string password = argv[3];

    const std::string port = "8123";
    const std::string name = "161.35.48.64";
    struct addrinfo hints;
    struct addrinfo *client_info;
    struct sockaddr_in server;
    int sd;

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
        return false;
    }

    //verify user credentials before uploading the file
    std::string credentials = username + "," + password;
    std::string credentials_size = std::to_string(credentials.length());
    if(credentials_size.length() < 2){
      credentials_size = " " + credentials_size;
    }
    std::string send_credentials = "CREDENTIALS " + credentials_size;
    send(sd, send_credentials.c_str(), send_credentials.length(), 0);

    char buffer[BUFFER_SIZE];
    int n = recv(sd, buffer, strlen("CREDENTIALS found"), 0);
    send(sd, credentials.c_str(), credentials.length(), 0);
    n = recv(sd, buffer, strlen("succes"), 0);

    //failed
    buffer[n] = '\0';
    if(strcmp(buffer, "failed") == 0){
      std::cerr << "Error: Incorrect login credentials!" <<std::endl;
      return false;
    }
    std::cout << "Successfully logged in" <<std::endl;

    
    if (argc == 5 && !version_check(sd, argv[1], username, password, argv[4])){
      return false;
    }
    else if(argc == 6 && !version_check(sd, argv[1], username, password, argv[4], argv[5]))
    {
      return false;
    }
    
    freeaddrinfo(client_info);

    return true;
}
