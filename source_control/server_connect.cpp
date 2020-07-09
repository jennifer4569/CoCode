#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <unistd.h>

#include <cassert>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#define SERVER_LOGIN_FILE "server_login.txt"

//from https://stackoverflow.com/questions/47068948/best-practice-to-use-execvp-in-c
template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N])
{
  assert((N > 0) && (argv[N - 1] == nullptr));

  return execvp(file, const_cast<char* const*>(argv));
}


void get_login_info(std::string& ip, std::string& user, std::string& pass){
  std::ifstream f;
  f.open(SERVER_LOGIN_FILE);
  if(!f.good()){
    std::cerr << SERVER_LOGIN_FILE << " not found!" << std::endl;
    exit(1);
  }

  //assumes the input file is valid format
  f >> ip;
  f >> user;
  f >> pass;

  f.close();
}

int main(){
  std::string ip, user, pass;
  get_login_info(ip, user, pass);

  //writing to the write end of the pipe
  int p[2];
  int rc = pipe(p);
  write(p[1], pass.c_str(), pass.length());
  close(p[1]);

  int f1 = fork();
  
  //child process execs rsync command
  if(f1 == 0){
    dup2(p[0], 0); //redirect read end of pipe to stdin
    const std::string c = user + "@" + ip + ":/home/" + user;
    const char* const a[] = {"rsync", "-avz", "tmp.txt", c.c_str(), nullptr};
    execvp(a[0], a);
    perror("exec failed");
  }
  else{
    wait(NULL);
    std::cout << "done" << std::endl;
  }

    
}
