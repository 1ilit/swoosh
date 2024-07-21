#pragma once

#include "HTMLTokenizer.h"
#include <string>

class HTMLParser
{
public:
    void parse();
    HTMLParser(std::string input);

private:
    HTMLTokenizer m_tokenizer;
};