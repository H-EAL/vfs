#pragma once

#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>

#include "vfs/string_converter.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    inline std::string get_extension(const std::string &fileName)
    {
        const auto pos = fileName.find_last_of('.');
        return (pos != std::string::npos) ? fileName.substr(pos + 1) : "";
    }

    //----------------------------------------------------------------------------------------------
    inline std::string extract_file_name(const std::string &fileName)
    {
        const auto pos = fileName.find_last_of("/\\");
        return (pos != std::string::npos) ? fileName.substr(pos + 1) : fileName;
    }

    //----------------------------------------------------------------------------------------------
    inline std::string remove_extension(const std::string &fileName)
    {
        const auto pos = fileName.find_last_of('.');
        return (pos != std::string::npos) ? fileName.substr(0, pos) : "";
    }

    //----------------------------------------------------------------------------------------------
    inline std::string to_lower_case(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](char c) { return char(::tolower(c)); });
        return str;
    }

    //----------------------------------------------------------------------------------------------
    template<typename _CharType>
    inline auto split_string(const std::basic_string<_CharType> &str, const std::basic_string<_CharType> &delimiters = " ")
    {
        using _StringType = std::basic_string<_CharType>;
        std::vector<_StringType> tokens;

        // Skip delimiters at beginning.
        auto lastPos = str.find_first_not_of(delimiters, 0);
        // Find first "non-delimiter".
        auto pos = str.find_first_of(delimiters, lastPos);

        while (_StringType::npos != pos || _StringType::npos != lastPos)
        {
            // Found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            // Skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);
            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }

        return tokens;
    }

    //----------------------------------------------------------------------------------------------
    template<typename _StringType>
    inline auto trimmed(const _StringType &str)
    {
        // Find first and last space.
        const auto spaces   = string_converter<_StringType>::to_native(" \t\r\n");
        auto firstChar      = str.find_first_not_of(spaces);
        auto lastChar       = str.find_last_not_of(spaces);

        // The string doesn't start with a space.
        if (firstChar == _StringType::npos)
        {
            firstChar = 0;
        }

        const auto trimmedStringLength = lastChar == _StringType::npos ? _StringType::npos : (lastChar - firstChar + 1);
        return str.substr(firstChar, trimmedStringLength);
    }

} /*vfs*/
