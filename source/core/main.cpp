#include "app.hpp"

#include <iostream>

int main(){
    Application::App app{};
    try{
        app.run();
    }
    catch(const std::exception &exception){
        std::cerr << exception.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

    return 0;
}