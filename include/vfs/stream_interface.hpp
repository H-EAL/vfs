#pragma once

#include <string>
#include <vector>

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
        int64_t read(T &toRead)
        {
            const int64_t sizeInBytes = sizeof(T);
            return _StreamImpl::read((uint8_t*)&toRead, sizeInBytes);
        }

        // Specialization for string
        int64_t read(std::string &toRead)
        {
            return read(toRead.data(), toRead.size());
        }

        // Specialization for vector
        template<typename T>
        int64_t read(std::vector<T> &toRead)
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
        int64_t write(const T &toWrite)
        {
            const int64_t sizeInBytes = sizeof(T);
            return _StreamImpl::write((const uint8_t*)&toWrite, sizeInBytes);
        }

        // Specialization for string
        int64_t write(const std::string &toWrite)
        {
            return write(toWrite.data(), toWrite.size());
        }

        // Specialization for vector
        template<typename T>
        int64_t write(const std::vector<T> &toWrite)
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
        int64_t write(const void *pToWrite, int64_t sizeInBytes)
        {
            return _StreamImpl::write((const uint8_t*)pToWrite, sizeInBytes);
        }

        int64_t read(void *pToRead, int64_t sizeInBytes)
        {
            return _StreamImpl::read((uint8_t*)pToRead, sizeInBytes);
        }

        // Dynamic array
        template<typename T>
        int64_t read(T *pToRead, int64_t count)
        {
            const int64_t sizeInBytes = count * sizeof(T);
            return _StreamImpl::read((uint8_t*)pToRead, sizeInBytes);
        }

        template<typename T>
        int64_t write(const T *pToWrite, int64_t count)
        {
            const int64_t sizeInBytes = count * sizeof(T);
            return _StreamImpl::write((const uint8_t*)pToWrite, sizeInBytes);
        }

        // Static array
        template<typename T, int N>
        int64_t read(T(&toRead)[N])
        {
            const int64_t sizeInBytes = N * sizeof(T);
            return _StreamImpl::read((uint8_t*)toRead, sizeInBytes);
        }

        template<typename T, int N>
        stream_interface<_StreamImpl>& operator >> (T(&toRead)[N])
        {
            read(toRead);
            return (*this);
        }

        template<typename T, int N>
        int64_t write(const T(&toWrite)[N])
        {
            const int64_t sizeInBytes = N * sizeof(T);
            return _StreamImpl::write((const uint8_t*)toWrite, sizeInBytes);
        }

        template<typename T, int N>
        stream_interface<_StreamImpl>& operator <<(const T(&toWrite)[N])
        {
            write(toWrite);
            return (*this);
        }
    };

} /*vfs*/
