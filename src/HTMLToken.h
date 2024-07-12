#pragma once
#include <string>
#include <vector>

class HTMLToken
{
    friend class HTMLTokenizer;

public:
    enum class Type
    {
        DOCTYPE,
        START_TAG,
        END_TAG,
        COMMENT,
        CHARACTER,
        END_OF_FILE
    };

    struct Attribute
    {
        std::string name;
        std::string value;
    };

    struct DoctypeToken
    {
        std::string name;
        std::string public_identifier;
        std::string system_identifier;
        bool force_quirks;
    };

    struct TagToken
    {
        std::string name;
        bool self_closing;
        std::vector<Attribute> attributes;
    };

    struct CommentOrCharacterToken
    {
        std::string data;
    };

    std::string to_string();

private:
    Type m_type;

    DoctypeToken m_doctype_token;
    TagToken m_start_tag;
    TagToken m_end_tag;
    CommentOrCharacterToken m_comment;
    CommentOrCharacterToken m_character;

    unsigned int m_offset;
};