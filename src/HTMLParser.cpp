#include "HTMLParser.h"
#include <iostream>
#include <optional>

HTMLParser::HTMLParser(std::string input) : m_tokenizer(input)
{
}

void HTMLParser::parse()
{
    std::optional<HTMLToken> next = {};
    for (;;)
    {
        next = m_tokenizer.next_token();
        if (next)
        {
            std::cout << (*next).to_string() << '\n';
        }
        else
        {
            break;
        }
    }
}
