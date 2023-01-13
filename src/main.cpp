// --- Internal Includes ---
#include "app.hpp"

// --- STL Includes ---
#include <iostream>


int main() {
    try {
        Application().run();
    } catch (const std::exception& r_exception) {
        std::cerr << r_exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
