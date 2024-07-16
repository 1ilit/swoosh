#include "HTMLToken.h"

std::string if_empty_say_empty(const std::string &str)
{
    return str.length() == 0 ? "EMPTY" : str;
}

std::string HTMLToken::to_string()
{
    std::string str = "";
    switch (m_type)
    {
    case Type::DOCTYPE:
        return "DOCTYPE "
               "{\n\tname: " +
               if_empty_say_empty(m_doctype_token.name) +
               ",\n\tpublic identifier: " + if_empty_say_empty(m_doctype_token.public_identifier) +
               ",\n\tsystem identifer: " + if_empty_say_empty(m_doctype_token.system_identifier) +
               ",\n\tforce quirks: " + (m_doctype_token.force_quirks ? "on" : "off") + "\n}";
    case Type::COMMENT:
        return "COMMENT "
               "{\n\tdata: " +
               if_empty_say_empty(m_comment.data) +
               "\n}";
    case Type::CHARACTER:
        return "CHARACTER "
               "{\n\tdata: " +
               if_empty_say_empty(m_character.data) +
               "\n}";
    case Type::START_TAG:
        str = "START TAG ";
        str.append("{\n\tname: ");
        str.append(m_start_tag.name);
        str.append(",\n\tself closing: ");
        str.append(m_start_tag.self_closing ? "yes" : "no");
        str.append(",\n\tattributes: [\n");

        for (const Attribute &a : m_start_tag.attributes)
        {
            str.append("\t\t" + a.name + ": " + a.value + '\n');
        }

        str.append("\t]\n}");
        return str;
    case Type::END_TAG:
        str = "END TAG ";
        str.append("{\n\tname: ");
        str.append(m_end_tag.name);
        str.append(",\n\tself closing: ");
        str.append(m_end_tag.self_closing ? "yes" : "no");
        str.append(",\n\tattributes: [\n");

        for (const Attribute &a : m_end_tag.attributes)
        {
            str.append("\t\t" + a.name + ": " + a.value + '\n');
        }

        str.append("\t]\n}");
        return str;
    case Type::END_OF_FILE:
        return "END OF FILE";
    default:
        return "";
    }
}