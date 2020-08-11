#include <iostream>
#include <fstream>

int main() {
    std::ofstream out("test.txt");
    out << "test" << std::endl;
    out.close();
}