#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "HTMLTokenizer.h"

std::string read(const std::string &filePath)
{
    std::ifstream in(filePath);

    if (!in.is_open())
    {
        throw std::runtime_error("Failed to open " + filePath);
    }

    std::stringstream ss;
    ss << in.rdbuf();
    in.close();

    return ss.str();
}

int main()
{
    std::string res = read("./examples/simple.html");
    HTMLTokenizer tokenizer(res);

    tokenizer.tokenize();

    return 0;
}