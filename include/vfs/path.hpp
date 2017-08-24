#pragma once

#include "vfs/string_utils.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    class path
    {
    public:
        //------------------------------------------------------------------------------------------
        // If the system is set to Unicode use wide char otherwise use regular char.
        using string_type       = std::conditional<VFS_USE_UNICODE, std::wstring, std::string>::type;
        using converter_type    = string_converter<string_type>;

    public:
        //------------------------------------------------------------------------------------------
        path() = default;
        //------------------------------------------------------------------------------------------
        path(const std::string &p)
            : pathStr_(converter_type::to_native(p))
        {}
        //------------------------------------------------------------------------------------------
        path(const std::wstring &p)
            : pathStr_(converter_type::to_native(p))
        {}
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
            static const auto path_separator = converter_type::to_native("\\");
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
        const string_type& str() const { return pathStr_; }
        //------------------------------------------------------------------------------------------
        // Conversion to C string.
        const auto c_str() const { return str().c_str(); }

    public:
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
        string_type pathStr_;
    };


    //----------------------------------------------------------------------------------------------
    // Moves a file or a directory from src path to dst path.
    // If src path is a directory, src and dst paths must be on the same drive.
    inline bool move(const path &src, const path &dst, bool overwrite = false, int32_t maxAttempts = 1)
    {
        // When moving a file, the destination can be on a different file system or volume.
        // If the destination is on another drive, you must set the MOVEFILE_COPY_ALLOWED flag in dwFlags.
        auto flags = MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH;
        if (overwrite)
        {
            flags |= MOVEFILE_REPLACE_EXISTING;
        }

        auto attempts = 0;
        while (MoveFileEx(src.c_str(), dst.c_str(), flags) == FALSE)
        {
            if (++attempts == maxAttempts)
            {
                const auto errorCode = GetLastError();
                vfs_errorf("MoveFileEx(%ws, %ws, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) failed after %d attempts, returned error: %s", src.c_str(), dst.c_str(), attempts, get_last_error_as_string(errorCode).c_str());
                return false;
            }
        }

        return true;
    }

} /*vfs*/
