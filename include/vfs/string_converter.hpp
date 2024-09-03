#pragma once

#include <string>

#include "vfs/platform.hpp"


namespace vfs {

#if VFS_PLATFORM_WIN
    //--------------------------------------------------------------------------------------------------
    inline std::string wstring_to_string(const std::wstring &toConvert)
    {
        if (toConvert.empty())
        {
            return std::string{};
        }

        int size_needed = WideCharToMultiByte(CP_UTF8, 0, toConvert.c_str(), (int) toConvert.size(), NULL, 0, NULL, NULL);
        std::string result(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, toConvert.c_str(), (int) toConvert.size(), &result[0], size_needed, NULL, NULL);
        return result;
    }

    //--------------------------------------------------------------------------------------------------
    inline std::wstring string_to_wstring(const std::string &toConvert)
    {
        if (toConvert.empty()) {
            return std::wstring();
        }

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, toConvert.c_str(), (int) toConvert.size(), NULL, 0);
        std::wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, toConvert.c_str(), (int) toConvert.size(), &result[0], size_needed);

        return result;
    }
#elif VFS_PLATFORM_POSIX
    //--------------------------------------------------------------------------------------------------
    inline std::string wstring_to_string(const std::wstring &toConvert)
    {
        if (toConvert.empty()) {
            return std::string();
        }

        std::setlocale(LC_ALL, "en_US.UTF-8");  // Set locale to handle UTF-8
        size_t size_needed = std::wcstombs(nullptr, toConvert.c_str(), 0) + 1;
        std::string result(size_needed, 0);
        std::wcstombs(&result[0], toConvert.c_str(), size_needed);

        return result;
    }

    //--------------------------------------------------------------------------------------------------
    inline std::wstring string_to_wstring(const std::string &toConvert)
    {
        if (toConvert.empty()) {
            return std::wstring();
        }

        std::setlocale(LC_ALL, "en_US.UTF-8");  // Set locale to handle UTF-8
        size_t size_needed = std::mbstowcs(nullptr, toConvert.c_str(), 0) + 1;
        std::wstring result(size_needed, 0);
        std::mbstowcs(&result[0], toConvert.c_str(), size_needed);

        return result;
    }
#else
#	error No string_converter implementation defined for the current platform
#endif
    
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

} /*vfs*/
