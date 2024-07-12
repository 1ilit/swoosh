#pragma once

#include <string>

class HTMLTokenizer
{
public:
    bool tokenize();
    void set_input(std::string input);
    void shift_cursor(unsigned int offset = 1);

    HTMLTokenizer(std::string input);
    ~HTMLTokenizer();

private:
    std::string m_input;
    unsigned int m_cursor;
};