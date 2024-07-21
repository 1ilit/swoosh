#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "HTMLTokenizer.h"
#include "HTMLParser.h"
#include "HTTPClient.h"

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
    // HTMLTokenizer tokenizer(res);
    // tokenizer.tokenize();

    HTMLParser parser(res);
    parser.parse();

    // HTTPClient client;
    // client.get("raw.githubusercontent.com", "/drawdb-io/drawdb/main/src/i18n/i18n.js");
    // client.get("raw.githubusercontent.com", "/drawdb-io/drawdb/main/src/main.jsx");

    return 0;
}