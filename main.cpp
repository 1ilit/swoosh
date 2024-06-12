#include <iostream>
#include <fstream>
#include <string>

int main()
{
    std::ifstream in("test.html");
    std::string line;
    while (std::getline(in, line))
    {
        std::cout << line << '\n';
    }
    return 0;
}