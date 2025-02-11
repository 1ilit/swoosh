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

std::optional<HTMLToken> HTMLTokenizer::next_token()
{
    char current_input_character;
    m_current_token = {};

    for (;;)
    {
        if (m_cursor >= m_input.length())
        {
            break;
        }

        switch (m_state)
        {
        case State::DATA:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t' || current_input_character == '\n')
            {
                m_state = State::DATA;
                continue;
            }

            if (current_input_character == '&')
            {
                m_return_state = State::DATA;
                m_state = State::CHARACTER_REFERENCE;
                continue;
            }

            if (current_input_character == '<')
            {
                m_state = State::TAG_OPEN;
                continue;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            if (!current_input_character)
            {
                HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_NULL_CHARACTER).emit_error();
            }

            m_current_token.m_type = HTMLToken::Type::CHARACTER;
            m_current_token.m_character.data += current_input_character;
            return m_current_token;

            break;

        case State::RCDATA:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '&')
            {
                m_return_state = State::RCDATA;
                m_state = State::CHARACTER_REFERENCE;
                continue;
            }

            if (current_input_character == '<')
            {
                m_state = State::RCDATA_LESS_THAN_SIGN;
                continue;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            if (!current_input_character)
            {
                HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_NULL_CHARACTER).emit_error();
            }

            m_current_token.m_type = HTMLToken::Type::CHARACTER;
            m_current_token.m_character.data += current_input_character;
            return m_current_token;

            break;

        case State::RAWTEXT:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '<')
            {
                m_state = State::RAWTEXT_LESS_THAN_SIGN;
                continue;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            if (!current_input_character)
            {
                HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_NULL_CHARACTER).emit_error();
            }

            m_current_token.m_type = HTMLToken::Type::CHARACTER;
            m_current_token.m_character.data += current_input_character;
            return m_current_token;

            break;

        case State::RCDATA_LESS_THAN_SIGN:
            break;

        case State::CHARACTER_REFERENCE:
            break;

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
                continue;
            }

            if (current_input_character == '/')
            {
                m_state = State::END_TAG_OPEN;
                continue;
            }

            if (isalpha(current_input_character))
            {
                m_current_token.m_type = HTMLToken::Type::START_TAG;
                m_current_token.m_start_tag.name = current_input_character;
                m_state = State::TAG_NAME;
                continue;
            }

            break;

        case State::TAG_NAME:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                m_state = State::BEFORE_ATTRIBUTE_NAME;
                continue;
            }

            if (current_input_character == '/')
            {
                m_state = State::SELF_CLOSING_START_TAG;
                continue;
            }

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
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
                continue;
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
            continue;

            break;

        case State::SELF_CLOSING_START_TAG:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '>')
            {
                m_current_token.m_start_tag.self_closing = true;
                m_state = State::DATA;
                return m_current_token;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            // error
            shift_cursor(-1);
            m_state = State::BEFORE_ATTRIBUTE_NAME;
            continue;

            break;

        case State::BEFORE_ATTRIBUTE_NAME:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                m_state = State::BEFORE_ATTRIBUTE_NAME;
                continue;
            }

            if (current_input_character == '/' || current_input_character == '>' || current_input_character == '\0')
            {
                shift_cursor(-1);
                m_state = State::AFTER_ATTRIBUTE_NAME;
                continue;
            }

            // if (current_input_character == '=')
            // {
            //     std::cout << "error\n";
            // }

            m_current_token.m_start_tag.attributes.push_back({.name = "", .value = ""});
            m_state = State::ATTRIBUTE_NAME;
            shift_cursor(-1);
            continue;

            break;

        case State::AFTER_ATTRIBUTE_NAME:
            break;

        case State::ATTRIBUTE_NAME:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' ||
                current_input_character == '\t' || current_input_character == '/' || current_input_character == '>')
            {
                shift_cursor(-1);
                m_state = State::AFTER_ATTRIBUTE_NAME;
                continue;
            }

            if (current_input_character == '=')
            {
                m_state = State::BEFORE_ATTRIBUTE_VALUE;
                continue;
            }

            if (isupper(current_input_character))
            {
                m_current_token.m_start_tag.attributes.back().name += current_input_character - 32;
                continue;
            }

            m_current_token.m_start_tag.attributes.back().name += current_input_character;
            continue;

            break;

        case State::BEFORE_ATTRIBUTE_VALUE:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                continue;
            }

            if (current_input_character == '\'')
            {
                m_state = State::ATTRIBUTE_VALUE_SINGLE_QUOTED;
                continue;
            }

            if (current_input_character == '"')
            {
                m_state = State::ATTRIBUTE_VALUE_DOUBLE_QUOTED;
                continue;
            }

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
            }

            shift_cursor(-1);
            m_state = State::ATTRIBUTE_VALUE_UNQUOTED;
            continue;

            break;

        case State::ATTRIBUTE_VALUE_SINGLE_QUOTED:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '\'')
            {
                m_state = State::AFTER_ATTRIBUTE_VALUE_QUOTED;
                continue;
            }

            if (current_input_character == '&')
            {
                m_return_state = State::ATTRIBUTE_VALUE_SINGLE_QUOTED;
                m_state = State::CHARACTER_REFERENCE;
                continue;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            m_current_token.m_start_tag.attributes.back().value += current_input_character;
            continue;
            break;

        case State::ATTRIBUTE_VALUE_DOUBLE_QUOTED:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '"')
            {
                m_state = State::AFTER_ATTRIBUTE_VALUE_QUOTED;
                continue;
            }

            if (current_input_character == '&')
            {
                m_return_state = State::ATTRIBUTE_VALUE_DOUBLE_QUOTED;
                m_state = State::CHARACTER_REFERENCE;
                continue;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            m_current_token.m_start_tag.attributes.back().value += current_input_character;
            continue;
            break;

        case State::ATTRIBUTE_VALUE_UNQUOTED:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                continue;
            }

            if (current_input_character == '&')
            {
                m_return_state = State::ATTRIBUTE_VALUE_UNQUOTED;
                m_state = State::CHARACTER_REFERENCE;
                continue;
            }

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            if (current_input_character == '=' || current_input_character == '\'' || current_input_character || '"' || current_input_character == '<' || current_input_character == '`')
            {
                // error
            }

            m_current_token.m_start_tag.attributes.back().value += current_input_character;
            continue;

            break;

        case State::AFTER_ATTRIBUTE_VALUE_QUOTED:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                continue;
            }

            if (current_input_character == '/')
            {
                m_state = State::SELF_CLOSING_START_TAG;
                continue;
            }

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
            }

            if (current_input_character == '\0')
            {
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            // error
            shift_cursor(-1);
            m_state = State::BEFORE_ATTRIBUTE_NAME;
            continue;

            break;

        case State::MARKUP_DECLARATION_OPEN:
            if (next_few_characters_are(2, "--"))
            {
                shift_cursor(2);
                m_current_token.m_type = HTMLToken::Type::COMMENT;
                m_current_token.m_comment.data = "";
                m_state = State::COMMENT_START;
                continue;
            }

            if (next_few_characters_are(7, "DOCTYPE"))
            {
                shift_cursor(7);
                m_state = State::DOCTYPE;
                continue;
            }
            break;

        case State::COMMENT_START:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '-')
            {
                m_state = State::COMMENT_START_DASH;
                continue;
            }

            if (current_input_character == '>')
            {
                HTMLParseError(HTMLParseError::ErrorCode::ABRUPT_CLOSING_OF_EMPTY_COMMENT).emit_error();
                m_state = State::DATA;
                continue;
            }

            shift_cursor(-1);
            m_state = State::COMMENT;
            continue;

            break;

        case State::COMMENT:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '<')
            {
                m_current_token.m_comment.data += current_input_character;
                m_state = State::COMMENT_LESS_THAN_SIGN;
                continue;
            }

            if (current_input_character == '-')
            {
                m_state = State::COMMENT_END_DASH;
                continue;
            }

            if (current_input_character == '\0')
            {
                HTMLParseError(HTMLParseError::ErrorCode::EOF_IN_COMMENT).emit_error();
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            if (!current_input_character)
            {
                HTMLParseError(HTMLParseError::ErrorCode::UNEXPECTED_NULL_CHARACTER).emit_error();
            }

            m_current_token.m_comment.data += current_input_character;
            continue;

            break;

        case State::COMMENT_START_DASH:
            current_input_character = m_input[m_cursor];
            shift_cursor();
            if (current_input_character == '-')
            {
                m_state = State::COMMENT_END;
                continue;
            }

            if (current_input_character == '\0')
            {
                HTMLParseError(HTMLParseError::ErrorCode::EOF_IN_COMMENT).emit_error();
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            m_current_token.m_comment.data += '-';
            m_state = State::COMMENT;
            shift_cursor(-1);

            continue;

            break;

        case State::COMMENT_END_DASH:
            current_input_character = m_input[m_cursor];
            shift_cursor();
            if (current_input_character == '-')
            {
                m_state = State::COMMENT_END;
                continue;
            }

            if (current_input_character == '\0')
            {
                HTMLParseError(HTMLParseError::ErrorCode::EOF_IN_COMMENT).emit_error();
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            m_current_token.m_comment.data += '-';
            m_state = State::COMMENT;
            shift_cursor(-1);

            continue;

            break;

        case State::COMMENT_END:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
            }

            if (current_input_character == '!')
            {
                m_state = State::COMMENT_END_BANG;
                continue;
            }

            if (current_input_character == '-')
            {
                m_current_token.m_comment.data += '-';
                continue;
            }

            if (current_input_character == '\0')
            {
                HTMLParseError(HTMLParseError::ErrorCode::EOF_IN_COMMENT).emit_error();
                m_current_token.m_type = HTMLToken::Type::END_OF_FILE;
                return m_current_token;
            }

            m_current_token.m_comment.data.append("--");
            m_state = State::COMMENT;
            shift_cursor(-1);
            continue;

            break;

        case State::COMMENT_END_BANG:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == '-')
            {
                m_current_token.m_comment.data.append("--!");
                m_state = State::COMMENT_END_DASH;
                continue;
            }

            if (current_input_character == '>')
            {
                HTMLParseError(HTMLParseError::ErrorCode::INCORRECTLY_CLOSED_COMMENT).emit_error();
                m_state = State::DATA;
                return m_current_token;
            }

            // NOTE: This returned both an EOF and COMMENT. MAYBE return eof if for is broken next time??
            if (current_input_character == '\0')
            {
                HTMLParseError(HTMLParseError::ErrorCode::EOF_IN_COMMENT).emit_error();

                return m_current_token;
            }

            m_current_token.m_comment.data.append("--!");
            m_state = State::COMMENT;
            continue;

            break;

        case State::DOCTYPE:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                m_state = State::BEFORE_DOCTYPE_NAME;
                continue;
            }

            break;

        case State::BEFORE_DOCTYPE_NAME:
            current_input_character = m_input[m_cursor];
            shift_cursor();
            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                continue;
            }

            else
            {
                m_current_token.m_type = HTMLToken::Type::DOCTYPE;
                m_current_token.m_doctype_token.name = current_input_character;
                m_state = State::DOCTYPE_NAME;
                continue;
            }

            break;

        case State::DOCTYPE_NAME:
            current_input_character = m_input[m_cursor];
            shift_cursor();
            if (current_input_character == ' ' || current_input_character == '\a' || current_input_character == '\f' || current_input_character == '\t')
            {
                m_state = State::AFTER_DOCTYPE_NAME;
                continue;
            }

            if (current_input_character == '>')
            {
                m_state = State::DATA;
                return m_current_token;
            }

            else
            {
                m_current_token.m_doctype_token.name = m_current_token.m_doctype_token.name + current_input_character;
                continue;
            }
            break;

        case State::AFTER_DOCTYPE_NAME:
            break;

        case State::END_TAG_OPEN:
            current_input_character = m_input[m_cursor];
            shift_cursor();

            if (isalpha(current_input_character))
            {
                m_current_token.m_type = HTMLToken::Type::END_TAG;
                m_current_token.m_end_tag.name = current_input_character;
                m_state = State::TAG_NAME;
                continue;
            }

            break;

        default:
            break;
        }
        break;
    }

    return std::nullopt;
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
