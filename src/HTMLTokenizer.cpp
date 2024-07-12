#include "HTMLTokenizer.h"
#include <iostream>

HTMLTokenizer::HTMLTokenizer(std::string input)
{
    m_input = input;
}

HTMLTokenizer::~HTMLTokenizer() {}

bool HTMLTokenizer::tokenize()
{
    std::cout << "Tokenizing...\n";
    std::cout << m_input;
    return true;
}

void HTMLTokenizer::set_input(std::string input)
{
    m_input = input;
}

void HTMLTokenizer::shift_cursor(unsigned int offset)
{
    if (m_cursor >= m_input.length())
    {
        return;
    }

    m_cursor += offset;
}
