# this makefile assumes that it's linux
# apt-get install make

compile_server: clean
	@echo "Compiling Server Code..."
	@g++ git_server_linux.cpp sql_authenticate.cpp -l sqlite3 -o source_control_server.exe
	@g++ sql_login.cpp -l sqlite3 -o sql_register.exe

# NOTE: This is designed for the CoCode server, make sure you test the ./wait_for_register.sh first
#       before running this command, otherwise it'll spam your terminal. Run make clean to resolve.
init_server: compile_server
	@echo "Initializing Server..."
	@./wait_for_register.sh &
	@./source_control_server.exe &

clean:
	@echo "Cleaning directories..."
	@find . -type f -name "*~" -delete
	@find . -type f -name "*.exe" -delete
	@find . -type f -name "*.out" -delete
	@killall wait_for_register.sh
	@killall inotifywait
	@killall source_control_server.exe
