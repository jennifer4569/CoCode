#include <iostream>
#include <sqlite3.h>
#include <vector>
#include <string.h>
//install with sudo apt-get install sqlite3 libsqlite3-dev
//compile with -l sqlite3
  
struct sql_data{
  bool receivedData;
  std::vector<const char*> data;
};

static int callback(void* data, int argc, char** argv, char** azColName){
  sql_data* retData = (sql_data*)data;
  if(argc == 0)
    retData->receivedData = false;
  else{
    retData->receivedData = true;
    retData->data.clear();
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
  for(int i = 0; i < str.length(); i++){
    if(!isalnum(str[i])){
      //maybe allow certain special characters here but idk
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv){
  sqlite3* db;
  if(sqlite3_open("login.db", &db)){
    std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
    return EXIT_FAILURE;
  }
  std::string command;
  char* messageError;
  //command =  "IF (EXISTS (SELECT * FROM INFORMATION_SCHEMA.TABLES";
  //command += "          WHERE TABLE_NAME = 'USERS'))";

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

  std::string username = std::string(argv[1]);
  std::string password = std::string(argv[2]);
  if(!valid(username) || !valid(password)){
    std::cerr << "Error: Invalid characters!" << std::endl;
    return EXIT_FAILURE;
  }

  //command = "SELECT * FROM USERS;";
  command  = "SELECT username FROM USERS WHERE username=";
  command += "'" + username + "';";
  if(sqlite3_exec(db, command.c_str(), callback, retData, &messageError) != SQLITE_OK){
    std::cerr << "Error: " << messageError << std::endl;
    sqlite3_free(messageError);
  }
  if(retData->receivedData && retData->data.size() > 0 && strcmp(retData->data[0], "VARCHAR(255)") != 0){
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
    std::cout << "Success: Added user " << username << std::endl;
  }
  free(retData);
  sqlite3_close(db);
  return EXIT_SUCCESS;
}
