#include <iostream>
#include <dylib.hpp>

int main() {
    try {
        dylib::library lib("./build/bin/configuration.so");
        std::cout << "Loaded!" << std::endl;
    } catch(std::exception& e) {
        std::cerr << "Failed: " << e.what() << std::endl;
    }
}
