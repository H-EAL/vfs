#pragma once

#include "vfs/platform.hpp"
#include "vfs/string_utils.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    class path
    {
    public:
        //------------------------------------------------------------------------------------------
        // If the system is set to Unicode use wide char otherwise use regular char.
        using converter_type = string_converter<string>;

    public:
        //------------------------------------------------------------------------------------------
        path() = default;
        //------------------------------------------------------------------------------------------
        path(const std::string &p)
            : pathStr_(converter_type::to_native(p))
        {
            sanitize();
        }
        //------------------------------------------------------------------------------------------
        path(const std::wstring &p)
            : pathStr_(converter_type::to_native(p))
        {
            sanitize();
        }
        //------------------------------------------------------------------------------------------
        path(const char *p)
            : path(std::string{ p })
        {}
        //------------------------------------------------------------------------------------------
        path(const wchar_t *p)
            : path(std::wstring{ p })
        {}

    public:
        //------------------------------------------------------------------------------------------
        static auto separator()
        {
            static const auto path_separator = converter_type::to_native
            (
            #if VFS_PLATFORM_WIN
                "\\"
            #else
                "/"
            #endif
            );
            return path_separator;
        }
        //------------------------------------------------------------------------------------------
        static auto anti_separator()
        {
            static const auto path_separator = converter_type::to_native
            (
            #if VFS_PLATFORM_WIN
                "/"
            #else
                "\\"
            #endif
            );
            return path_separator;
        }
        //------------------------------------------------------------------------------------------
        static auto separators()
        {
            static const auto path_separators = converter_type::to_native("\\/");
            return path_separators;
        }

    public:
        //------------------------------------------------------------------------------------------
        // Automatic conversion for string types.
        operator std::string()  const { return converter_type::to_string(pathStr_);    }
        operator std::wstring() const { return converter_type::to_wstring(pathStr_);   }

        //------------------------------------------------------------------------------------------
        // Conversion to native string.
        const string& str() const { return pathStr_; }
        //------------------------------------------------------------------------------------------
        // Conversion to C string.
        const auto c_str() const { return str().c_str(); }

    public:
        //------------------------------------------------------------------------------------------
        // Sanitize path separators.
        void sanitize()
        {
            std::replace(pathStr_.begin(), pathStr_.end(), anti_separator()[0], separator()[0]);
        }

        //------------------------------------------------------------------------------------------
        // Combines an undetermined amount of paths using the system native separator.
        template<typename... _Paths>
        static path combine(const path &p0, _Paths &&...paths)
        {
            return combine_internal(p0, combine(std::forward<_Paths>(paths)...));
        }

    private:
        //------------------------------------------------------------------------------------------
        // End of recursion for the variadic combine function.
        static path combine(const path &p)
        {
            return p;
        }
        //------------------------------------------------------------------------------------------
        // Combines 2 paths together using the system native separator.
        static path combine_internal(const path &lhs, const path &rhs)
        {
            if (!lhs.str().empty())
            {
                const auto lastChar = lhs.pathStr_[lhs.str().length() - 1];
                if (lastChar == '\\' || lastChar == '/')
                {
                    // Left hand side path already ends with a path separator.
                    return lhs.str() + rhs.str();
                }
                // Left hand side path doesn't end with a path separator.
                return lhs.str() + separator() + rhs.str();
            }
            // Empty left hand side path.
            return rhs;
        }

    private:
        //------------------------------------------------------------------------------------------
        // String representation of the actual file or directory path.
        string pathStr_;
    };

} /*vfs*/
