#pragma once

#include <string>
#include <optional>
#include "HTMLToken.h"

class HTMLTokenizer
{
public:
    enum class State
    {
        DATA,
        RCDATA,
        RAWTEXT,
        SCRIPT_DATA,
        PLAINTEXT,
        TAG_OPEN,
        END_TAG_OPEN,
        TAG_NAME,
        RCDATA_LESS_THAN_SIGN,
        RCDATA_END_TAG_OPEN_STATE,
        RCDATA_END_TAG_NAME,
        RAWTEXT_LESS_THAN_SIGN,
        RAWTEXT_END_TAG_OPEN,
        RAWTEXT_END_TAG_NAME,
        SCRIPT_DATA_LESS_THAN_SIGN,
        SCRIPT_DATA_END_TAG_OPEN,
        SCRIPT_DATA_END_TAG_NAME,
        SCRIPT_DATA_ESCAPE_START,
        SCRIPT_DATA_ESCAPE_START_DASH,
        SCRIPT_DATA_ESCAPED,
        SCRIPT_DATA_ESCAPED_DASH,
        SCRIPT_DATA_ESCAPED_DASH_DASH,
        SCRIPT_DATA_ESCAPED_LESS_THAN_SIGN,
        SCRIPT_DATA_ESCAPED_END_TAG_OPEN,
        SCRIPT_DATA_ESCAPED_END_TAG_NAME,
        SCRIPT_DATA_DOUBLE_ESCAPE_START,
        SCRIPT_DATA_DOUBLE_ESCAPED,
        SCRIPT_DATA_DOUBLE_ESCAPED_DASH,
        SCRIPT_DATA_DOUBLE_ESCAPED_DASH_DASH,
        SCRIPT_DATA_DOUBLE_ESCAPED_LESS_THAN_SIGN,
        SCRIPT_DATA_DOUBLE_ESCAPE_END,
        BEFORE_ATTRIBUTE_NAME,
        ATTRIBUTE_NAME,
        AFTER_ATTRIBUTE_NAME,
        BEFORE_ATTRIBUTE_VALUE,
        ATTRIBUTE_VALUE_DOUBLE_QUOTED,
        ATTRIBUTE_VALUE_SINGLE_QUOTED,
        ATTRIBUTE_VALUE_UNQUOTED,
        AFTER_ATTRIBUTE_VALUE_QUOTED,
        SELF_CLOSING_START_TAG,
        BOGUS_COMMENT,
        MARKUP_DECLARATION_OPEN,
        COMMENT_START,
        COMMENT_START_DASH,
        COMMENT,
        COMMENT_LESS_THAN_SIGN,
        COMMENT_LESS_THAN_SIGN_BANG,
        COMMENT_LESS_THAN_SIGN_BANG_DASH,
        COMMENT_LESS_THAN_SIGN_BANG_DASH_DASH,
        COMMENT_END_DASH,
        COMMENT_END,
        COMMENT_END_BANG,
        DOCTYPE,
        BEFORE_DOCTYPE_NAME,
        DOCTYPE_NAME,
        AFTER_DOCTYPE_NAME,
        AFTER_DOCTYPE_PUBLIC_KEYWORD,
        BEFORE_DOCTYPE_PUBLIC_IDENTIFIER,
        DOCTYPE_PUBLIC_IDENTIFIER_DOUBLE_QUOTED,
        DOCTYPE_PUBLIC_IDENTIFIER_SINGLE_QUOTED,
        AFTER_DOCTYPE_PUBLIC_IDENTIFIER,
        BETWEEN_DOCTYPE_PUBLIC_AND_SYSTEM_IDENTIFIERS,
        AFTER_DOCTYPE_SYSTEM_KEYWORD,
        BEFORE_DOCTYPE_SYSTEM_IDENTIFIER,
        DOCTYPE_SYSTEM_IDENTIFIER_DOUBLE_QUOTED,
        DOCTYPE_SYSTEM_IDENTIFIER_SINGLE_QUOTED,
        AFTER_DOCTYPE_SYSTEM_IDENTIFIER,
        BOGUS_DOCTYPE,
        CDATA_SECTION,
        CDATA_SECTION_BRACKET,
        CDATA_SECTION_END,
        CHARACTER_REFERENCE,
        NAMED_CHARACTER_REFERENCE,
        AMBIGUOUS_AMPERSAND,
        NUMERIC_CHATACTER_REFERENCE,
        HEXADECIMAL_CHATACTER_REFERENCE_START,
        DECIMAL_CHATACTER_REFERENCE_START,
        HEXADECIMAL_CHATACTER_REFERENCE,
        DECIMAL_CHATACTER_REFERENCE,
        NUMERIC_CHATACTER_REFERENCE_END,
    };

    std::optional<HTMLToken> next_token();
    void set_input(std::string input);
    void shift_cursor(int offset = 1);
    bool next_few_characters_are(size_t number, std::string chars);
    void emit_current_token();

    HTMLTokenizer(std::string input);
    ~HTMLTokenizer();

private:
    State m_state{State::DATA};
    State m_return_state{State::DATA};

    std::string m_input;
    unsigned int m_cursor = 0;
    HTMLToken m_current_token = {};
};