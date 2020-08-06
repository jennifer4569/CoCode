#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_USERS 10
//install with sudo apt-get install sqlite3 libsqlite3-dev
//compile with -l sqlite3
  
struct sql_data{
  bool receivedData;
  std::vector<const char*> data;
};

static int callback(void* data, int argc, char** argv, char** azColName){
  sql_data* retData = (sql_data*)data;
  retData->data.clear();
  if(argc == 0)
    retData->receivedData = false;
  else{
    retData->receivedData = true;
    for(int i = 0; i < argc; i++){
      //sprintf(tmp, "%s = %s\n", azColName[i], argv[i]? argv[i]: "NULL");
      const char* tmp;
      if(argv[i]){
	tmp = argv[i];
      }
      else{
	std::string str = "NULL";
	tmp = str.c_str();
      }
      retData->data.push_back(tmp);
    }
  }
  return 0;
}

bool valid(std::string str){
  if(str.length() >= 30) return false;
  for(int i = 0; i < str.length(); i++){
    if(!isalnum(str[i])){
      //maybe allow certain special characters here but idk
      return false;
    }
  }
  return true;
}
//from https://stackoverflow.com/questions/47068948/best-practice-to-use-execvp-in-c
template <std::size_t N>
int execvp(const char* file, const char* const (&argv)[N])
{
  //assert((N > 0) && (argv[N - 1] == nullptr));

  return execvp(file, const_cast<char* const*>(argv));
}

bool validate_credentials(std::string username, std::string password){
  sqlite3* db;
  if(sqlite3_open("login.db", &db)){
    std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
    return false;
  }
  if(!valid(username) || !valid(password))
    return false;
  
  std::string command;
  char* messageError;
  sql_data* retData = (sql_data*) calloc(1, sizeof(sql_data));

  command  = "SELECT COUNT(*) FROM USERS WHERE ";
  command += "username='" + username + "' AND ";
  command += "password='" + password + "';";

  if(sqlite3_exec(db, command.c_str(), callback, retData, &messageError) != SQLITE_OK){
    std::cerr << "Error: " << messageError << std::endl;
    sqlite3_free(messageError);
  }
  
  if(retData->data.size() > 0 && atoi(retData->data[0]) > 0){
    free(retData);
    return true;
  }
  free(retData);
  return false;
}
