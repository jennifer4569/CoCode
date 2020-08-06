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

int main(int argc, char** argv){
  sqlite3* db;
  if(sqlite3_open("login.db", &db)){
    std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
    return EXIT_FAILURE;
  }
  
  //creates table if it does not exist
  std::string command;
  char* messageError;
  sql_data* retData = (sql_data*) calloc(1, sizeof(sql_data));
  command = "SELECT name FROM sqlite_master WHERE type='table' AND name='USERS';";
  if(sqlite3_exec(db, command.c_str(), callback, retData, &messageError) != SQLITE_OK){
    std::cerr << "Error: " << messageError << std::endl;
    sqlite3_free(messageError);
  }
  if(!retData->receivedData){
    command  = "  CREATE TABLE USERS(";
    command += "    username VARCHAR(255) PRIMARY KEY NOT NULL,"; //string of maxlength 255 chars
    command += "    password VARCHAR(255) NOT NULL";
    command += "  );";
    //must use c styled string
    if(sqlite3_exec(db, command.c_str(), NULL, 0, &messageError) != SQLITE_OK){
      std::cerr << "Error creating table: " << messageError << std::endl;
      sqlite3_free(messageError);
    }
  }

  //attempts to add user
  std::string username = std::string(argv[1]);
  std::string password = std::string(argv[2]);
  if(!valid(username) || !valid(password)){
    std::cerr << "Error: Invalid characters!" << std::endl;
    return EXIT_FAILURE;
  }

  command = "SELECT COUNT(*) FROM USERS;";
  if(sqlite3_exec(db, command.c_str(), callback, retData, &messageError) != SQLITE_OK){
    std::cerr << "Error: " << messageError << std::endl;
    sqlite3_free(messageError);
  }

  if(retData->data.size() > 0 && atoi(retData->data[0]) >= MAX_USERS){
    std::cout << atoi(retData->data[0]) << " >= " << MAX_USERS << std::endl;
    
    std::cerr << "Error: Maximum users exceeded!" << std::endl;
    return EXIT_FAILURE;
  }
  
  command  = "SELECT username FROM USERS WHERE username=";
  command += "'" + username + "';";
  if(sqlite3_exec(db, command.c_str(), callback, retData, &messageError) != SQLITE_OK){
    std::cerr << "Error: " << messageError << std::endl;
    sqlite3_free(messageError);
  }
  if(retData->receivedData && retData->data.size() > 0 && strcmp(retData->data[0], username.c_str()) == 0){
    std::cerr << "Error: Username already in use!" << std::endl;
    for(int i = 0; i < retData->data.size(); i++){
      std::cout << retData->data[i] << std::endl;
    }
    return EXIT_FAILURE;
  }  

  
  command = "INSERT INTO USERS VALUES (";
  command += "'" + username + "','"+ password + "');";

  if(sqlite3_exec(db, command.c_str(), NULL, 0, &messageError) != SQLITE_OK){
    std::cerr << "Error inserting into table: " << messageError << std::endl;
    sqlite3_free(messageError);
  }
  else{
    int f = fork();
    if(f == 0){
      const char* const a[] = {"bash", "./register_user.sh", username.c_str(), password.c_str(), nullptr};
      execvp(a[0], a);
      std::cerr << "Error: Couldn't add user " << username << std::endl;
      command = "DELETE FROM USERS WHERE username='"+username+"';";
      if(sqlite3_exec(db, command.c_str(), NULL, 0, &messageError) != SQLITE_OK){
	std::cerr << "Error: " << messageError << std::endl;
	sqlite3_free(messageError);
      }
      return EXIT_FAILURE;
    }
    else{
      int status;
      waitpid(f, &status, 0);
      if(status ==  EXIT_SUCCESS){
	std::cout << "Success: Added user " << username << std::endl;
      }
    }
  }
  
  free(retData);
  sqlite3_close(db);
  return EXIT_SUCCESS;
}
