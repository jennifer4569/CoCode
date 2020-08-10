# this makefile assumes that it's linux
# apt-get install make

compile_client: clean
	@echo "Compiling Client Code..."
	@g++ git_client_push_linux.cpp git_client_version.cpp -o push.exe
	@g++ git_client_pull_linux.cpp git_client_version.cpp -o pull.exe

clean:
	@echo "Cleaning directories..."
	@find . -type f -name "*~" -delete
	@find . -type f -name "*.exe" -delete
	@find . -type f -name "*.out" -delete
