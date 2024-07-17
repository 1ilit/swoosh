#include "HTMLTokenizer.h"
#include <iostream>
#include <cctype>
#include "HTMLToken.h"
#include "HTMLParseError.h"

HTMLTokenizer::HTMLTokenizer(std::string input)
{
    m_input = input;
}

HTMLTokenizer::~HTMLTokenizer() {}

bool HTMLTokenizer::tokenize()
{
    char current_input_character;

    switch (m_state)
    {
    Data:
    case State::DATA:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t' || current_input_character == '\n')
        {
            m_state = State::DATA;
            goto Data;
        }

        if (current_input_character == '&')
        {
            m_return_state = State::DATA;
            m_state = State::CHARACTER_REFERENCE;
            goto CharacterReference;
        }

        if (current_input_character == '<')
        {
            m_state = State::TAG_OPEN;
            goto TagOpen;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        if (!current_input_character)
        {
            HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_NULL_CHARACTER).emit_error();
        }

        m_current_token.m_type = HTMLToken::Type::CHARACTER;
        m_current_token.m_character.data += current_input_character;
        emit_current_token();
        goto Data;

        break;

    CharacterReference:
    case State::CHARACTER_REFERENCE:

        break;

    TagOpen:
    case State::TAG_OPEN:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == '?')
        {
            HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_QUESTION_MARK_INSTEAD_OF_TAG_NAME).emit_error();
        }

        if (current_input_character == '!')
        {
            m_state = State::MARKUP_DECLARATION_OPEN;
            goto MarkupDeclarationOpen;
        }

        if (current_input_character == '/')
        {
            m_state = State::END_TAG_OPEN;
            goto EndTagOpen;
        }

        if (isalpha(current_input_character))
        {
            m_current_token.m_type = HTMLToken::Type::START_TAG;
            m_current_token.m_start_tag.name = current_input_character;
            m_state = State::TAG_NAME;
            goto TagName;
        }

        break;

    TagName:
    case State::TAG_NAME:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            m_state = State::BEFORE_ATTRIBUTE_NAME;
            goto BeforeAttributeName;
        }

        if (current_input_character == '/')
        {
            m_state = State::SELF_CLOSING_START_TAG;
            goto SelfClosingStartTag;
        }

        if (current_input_character == '>')
        {
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        if (isupper(current_input_character))
        {
            if (m_current_token.m_type == HTMLToken::Type::START_TAG)
            {
                m_current_token.m_start_tag.name += (current_input_character - 32);
            }

            if (m_current_token.m_type == HTMLToken::Type::END_TAG)
            {
                m_current_token.m_end_tag.name += (current_input_character - 32);
            }
            m_state = State::TAG_NAME;
            goto TagName;
        }

        if (m_current_token.m_type == HTMLToken::Type::START_TAG)
        {
            m_current_token.m_start_tag.name += current_input_character;
        }

        if (m_current_token.m_type == HTMLToken::Type::END_TAG)
        {
            m_current_token.m_end_tag.name += current_input_character;
        }
        m_state = State::TAG_NAME;
        goto TagName;

        break;

    SelfClosingStartTag:
    case State::SELF_CLOSING_START_TAG:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == '>')
        {
            m_current_token.m_start_tag.self_closing = true;
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        // error
        shift_cursor(-1);
        m_state = State::BEFORE_ATTRIBUTE_NAME;
        goto BeforeAttributeName;

        break;

    BeforeAttributeName:
    case State::BEFORE_ATTRIBUTE_NAME:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            m_state = State::BEFORE_ATTRIBUTE_NAME;
            goto BeforeAttributeName;
        }

        if (current_input_character == '/' || current_input_character == '>' || current_input_character == '\0')
        {
            shift_cursor(-1);
            m_state = State::AFTER_ATTRIBUTE_NAME;
            goto AfterAttributeName;
        }

        // if (current_input_character == '=')
        // {
        //     std::cout << "error\n";
        // }

        m_current_token.m_start_tag.attributes.push_back({.name = "", .value = ""});
        m_state = State::ATTRIBUTE_NAME;
        shift_cursor(-1);
        goto AttributeName;

        break;

    AfterAttributeName:
    case State::AFTER_ATTRIBUTE_NAME:
        break;

    AttributeName:
    case State::ATTRIBUTE_NAME:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' ||
            current_input_character == '\t' || current_input_character == '/' || current_input_character == '>')
        {
            shift_cursor(-1);
            m_state = State::AFTER_ATTRIBUTE_NAME;
            goto AfterAttributeName;
        }

        if (current_input_character == '=')
        {
            m_state = State::BEFORE_ATTRIBUTE_VALUE;
            goto BeforeAttributeValue;
        }

        if (isupper(current_input_character))
        {
            m_current_token.m_start_tag.attributes.back().name += current_input_character - 32;
            goto AttributeName;
        }

        m_current_token.m_start_tag.attributes.back().name += current_input_character;
        goto AttributeName;

        break;

    BeforeAttributeValue:
    case State::BEFORE_ATTRIBUTE_VALUE:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            goto BeforeAttributeValue;
        }

        if (current_input_character == '\'')
        {
            m_state = State::ATTRIBUTE_VALUE_SINGLE_QUOTED;
            goto AttributeValueSingleQuoted;
        }

        if (current_input_character == '"')
        {
            m_state = State::ATTRIBUTE_VALUE_DOUBLE_QUOTED;
            goto AttributeValueDoubleQuoted;
        }

        if (current_input_character == '>')
        {
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        shift_cursor(-1);
        m_state = State::ATTRIBUTE_VALUE_UNQUOTED;
        goto AttributeValueUnquoted;

        break;

    AttributeValueSingleQuoted:
    case State::ATTRIBUTE_VALUE_SINGLE_QUOTED:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == '\'')
        {
            m_state = State::AFTER_ATTRIBUTE_VALUE_QUOTED;
            goto AfterAttributeValueQuoted;
        }

        if (current_input_character == '&')
        {
            m_return_state = State::ATTRIBUTE_VALUE_SINGLE_QUOTED;
            m_state = State::CHARACTER_REFERENCE;
            goto CharacterReference;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        m_current_token.m_start_tag.attributes.back().value += current_input_character;
        goto AttributeValueSingleQuoted;
        break;

    AttributeValueDoubleQuoted:
    case State::ATTRIBUTE_VALUE_DOUBLE_QUOTED:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == '"')
        {
            m_state = State::AFTER_ATTRIBUTE_VALUE_QUOTED;
            goto AfterAttributeValueQuoted;
        }

        if (current_input_character == '&')
        {
            m_return_state = State::ATTRIBUTE_VALUE_DOUBLE_QUOTED;
            m_state = State::CHARACTER_REFERENCE;
            goto CharacterReference;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        m_current_token.m_start_tag.attributes.back().value += current_input_character;
        goto AttributeValueDoubleQuoted;
        break;

    AttributeValueUnquoted:
    case State::ATTRIBUTE_VALUE_UNQUOTED:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            goto AttributeValueUnquoted;
        }

        if (current_input_character == '&')
        {
            m_return_state = State::ATTRIBUTE_VALUE_UNQUOTED;
            m_state = State::CHARACTER_REFERENCE;
            goto CharacterReference;
        }

        if (current_input_character == '>')
        {
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        if (current_input_character == '=' || current_input_character == '\'' || current_input_character || '"' || current_input_character == '<' || current_input_character == '`')
        {
            // error
        }

        m_current_token.m_start_tag.attributes.back().value += current_input_character;

        break;

    AfterAttributeValueQuoted:
    case State::AFTER_ATTRIBUTE_VALUE_QUOTED:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            goto AttributeValueUnquoted;
        }

        if (current_input_character == '/')
        {
            m_state = State::SELF_CLOSING_START_TAG;
            goto SelfClosingStartTag;
        }

        if (current_input_character == '>')
        {
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        if (current_input_character == '\0')
        {
            m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
            emit_current_token();
            break;
        }

        // error
        shift_cursor(-1);
        m_state = State::BEFORE_ATTRIBUTE_NAME;
        goto BeforeAttributeName;

        break;

    MarkupDeclarationOpen:
    case State::MARKUP_DECLARATION_OPEN:
        if (next_few_characters_are(2, "--"))
        {
            shift_cursor(2);
            //
        }

        if (next_few_characters_are(7, "DOCTYPE"))
        {
            shift_cursor(7);
            m_state = State::DOCTYPE;
            goto Doctype;
        }
        break;

    Doctype:
    case State::DOCTYPE:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            m_state = State::BEFORE_DOCTYPE_NAME;
            goto BeforeDoctypeName;
        }

        break;

    BeforeDoctypeName:
    case State::BEFORE_DOCTYPE_NAME:
        current_input_character = m_input[m_cursor];
        shift_cursor();
        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            goto BeforeDoctypeName;
        }

        else
        {
            m_current_token.m_type = HTMLToken::Type::DOCTYPE;
            m_current_token.m_doctype_token.name = current_input_character;
            m_state = State::DOCTYPE_NAME;
            goto DoctypeName;
        }

        break;

    DoctypeName:
    case State::DOCTYPE_NAME:
        current_input_character = m_input[m_cursor];
        shift_cursor();
        if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
        {
            m_state = State::AFTER_DOCTYPE_NAME;
            goto AfterDoctypeName;
        }

        if (current_input_character == '>')
        {
            emit_current_token();
            m_state = State::DATA;
            goto Data;
        }

        else
        {
            m_current_token.m_doctype_token.name = m_current_token.m_doctype_token.name + current_input_character;
            goto DoctypeName;
        }
        break;

    AfterDoctypeName:
    case State::AFTER_DOCTYPE_NAME:
        break;

    EndTagOpen:
    case State::END_TAG_OPEN:
        current_input_character = m_input[m_cursor];
        shift_cursor();

        if (isalpha(current_input_character))
        {
            m_current_token.m_type = HTMLToken::Type::END_TAG;
            m_current_token.m_end_tag.name = current_input_character;
            m_state = State::TAG_NAME;
            goto TagName;
        }

        break;

    default:
        break;
    }

    return true;
}

void HTMLTokenizer::set_input(std::string input)
{
    m_input = input;
}

void HTMLTokenizer::shift_cursor(int offset)
{
    if (m_cursor >= m_input.length())
    {
        return;
    }

    m_cursor += offset;
}

void HTMLTokenizer::emit_current_token()
{
    std::cout << m_current_token.to_string() << std::endl;
    m_current_token = {};
}

bool HTMLTokenizer::next_few_characters_are(size_t number, std::string chars)
{
    for (unsigned int i = m_cursor, j = 0; i < m_cursor + number, j < number; i++, j++)
    {
        if (m_input[i] != chars[j])
        {
            return false;
        }
    }

    return true;
}
