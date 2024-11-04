#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace vfs {

    //
    // This is an interface to access any stream of bytes.
    // It contains several functions to read and write generic types.
    //
    template<typename _StreamImpl>
    class stream_interface
        : public _StreamImpl
    {
    public:
        // Forward all construction parameters to the implementation
        template<typename... _Args>
        stream_interface(_Args &&...args)
            : _StreamImpl(std::forward<_Args>(args)...)
        {}

        // Single value
        template<typename T>
        uint64_t read(T &toRead)
        {
            const uint64_t sizeInBytes = sizeof(T);
            return _StreamImpl::read((uint8_t*)&toRead, sizeInBytes);
        }

        // Specialization for string
        uint64_t read(std::string &toRead)
        {
            return read(toRead.data(), toRead.size());
        }

        // Specialization for string view
        uint64_t read(std::string_view &toRead)
        {
            return read(toRead.data(), toRead.size());
        }

        // Specialization for vector
        template<typename T>
        uint64_t read(std::vector<T> &toRead)
        {
            return read(toRead.data(), toRead.size());
        }

        // Overload for the shift operator
        template<typename T>
        stream_interface<_StreamImpl>& operator >> (T &toRead)
        {
            read(toRead);
            return (*this);
        }

        // Write
        template<typename T>
        uint64_t write(const T &toWrite)
        {
            const uint64_t sizeInBytes = sizeof(T);
            return _StreamImpl::write((const uint8_t*)&toWrite, sizeInBytes);
        }

        // Specialization for string
        uint64_t write(const std::string &toWrite)
        {
            return write(toWrite.data(), toWrite.size());
        }

        // Specialization for string view
        uint64_t write(const std::string_view &toWrite)
        {
            return write(toWrite.data(), toWrite.size());
        }

        // Specialization for vector
        template<typename T>
        uint64_t write(const std::vector<T> &toWrite)
        {
            return write(toWrite.data(), toWrite.size());
        }

        // Overload for the shift operator
        template<typename T>
        stream_interface<_StreamImpl>& operator <<(const T &toWrite)
        {
            write(toWrite);
            return (*this);
        }

        // Raw Byte array
        uint64_t write(const void *pToWrite, uint64_t sizeInBytes)
        {
            return _StreamImpl::write((const uint8_t*)pToWrite, sizeInBytes);
        }

        uint64_t read(void *pToRead, uint64_t sizeInBytes)
        {
            return _StreamImpl::read((uint8_t*)pToRead, sizeInBytes);
        }

        // Dynamic array
        template<typename T>
        uint64_t read(T *pToRead, uint64_t count)
        {
            const uint64_t sizeInBytes = count * sizeof(T);
            return _StreamImpl::read((uint8_t*)pToRead, sizeInBytes);
        }

        template<typename T>
        uint64_t write(const T *pToWrite, uint64_t count)
        {
            const uint64_t sizeInBytes = count * sizeof(T);
            return _StreamImpl::write((const uint8_t*)pToWrite, sizeInBytes);
        }

        // Static array
        template<typename T, int N>
        uint64_t read(T(&toRead)[N])
        {
            const uint64_t sizeInBytes = N * sizeof(T);
            return _StreamImpl::read((uint8_t*)toRead, sizeInBytes);
        }

        template<typename T, int N>
        stream_interface<_StreamImpl>& operator >> (T(&toRead)[N])
        {
            read(toRead);
            return (*this);
        }

        template<typename T, int N>
        uint64_t write(const T(&toWrite)[N])
        {
            const uint64_t sizeInBytes = N * sizeof(T);
            return _StreamImpl::write((const uint8_t*)toWrite, sizeInBytes);
        }

        template<int N>
        uint64_t write(const char(&toWrite)[N], bool discardNullChar = false)
        {
            const uint64_t sizeInBytes = N * sizeof(char) - (discardNullChar ? 1 : 0);
            return _StreamImpl::write((const uint8_t *)toWrite, sizeInBytes);
        }

        template<typename T, int N>
        stream_interface<_StreamImpl>& operator <<(const T(&toWrite)[N])
        {
            write(toWrite);
            return (*this);
        }
    };

} /*vfs*/
