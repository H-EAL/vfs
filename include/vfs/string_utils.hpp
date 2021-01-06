#pragma once

#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>


namespace vfs {

    //--------------------------------------------------------------------------------------------------
    using string = std::conditional_t<VFS_USE_UNICODE, std::wstring, std::string>;

    //--------------------------------------------------------------------------------------------------
    inline std::string wstring_to_string(const std::wstring &toConvert)
    {
        using convert_type = std::codecvt_utf8<wchar_t>;
        static std::wstring_convert<convert_type, wchar_t> converter;
        return converter.to_bytes(toConvert);
    }

    //--------------------------------------------------------------------------------------------------
    inline std::wstring string_to_wstring(const std::string &toConvert)
    {
        using convert_type = std::codecvt_utf8<wchar_t>;
        static std::wstring_convert<convert_type, wchar_t> converter;
        return converter.from_bytes(toConvert);
    }
    
    //----------------------------------------------------------------------------------------------
    template<typename _NativeStringType>
    struct string_converter;
    //----------------------------------------------------------------------------------------------
    template<>
    struct string_converter<std::string>
    {
        static std::string  to_string(const std::string &str)    { return str; }
        static std::string  to_string(const std::wstring &str)   { return wstring_to_string(str); }
        static std::wstring to_wstring(const std::wstring &str)  { return str; }
        static std::wstring to_wstring(const std::string &str)   { return string_to_wstring(str); }

        template<typename _StringType>
        static std::string  to_native(const _StringType &str)    { return to_string(str); }
    };
    //----------------------------------------------------------------------------------------------
    template<>
    struct string_converter<std::wstring>
    {
        static std::string  to_string(const std::string &str)    { return str; }
        static std::string  to_string(const std::wstring &str)   { return wstring_to_string(str); }
        static std::wstring to_wstring(const std::wstring &str)  { return str; }
        static std::wstring to_wstring(const std::string &str)   { return string_to_wstring(str); }

        template<typename _StringType>
        static std::wstring to_native(const _StringType &str)    { return to_wstring(str); }
    };

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
