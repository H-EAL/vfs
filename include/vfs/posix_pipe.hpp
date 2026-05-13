#pragma once
// Posix pipe is implemented with a Unix domain socket which allows for bidirectional inter-process communication.
// As these sockets are bidirectional at the kernel level, the read/write access params passed to ctor are ignored.
// Note that availableBytesToRead is not queryable with sockets, so length of message should be passed first.

#include <chrono>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "vfs/platform.hpp"
#include "vfs/posix_file_flags.hpp"
#include "vfs/path.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    using pipe_impl = class posix_pipe;


    //----------------------------------------------------------------------------------------------
    class posix_pipe
    {
    protected:
        //------------------------------------------------------------------------------------------
        using native_handle = int32_t;

    protected:
        //------------------------------------------------------------------------------------------
        native_handle nativeHandle() const
        {
            return clientFd_;
        }

        //------------------------------------------------------------------------------------------
        const path &fileName() const
        {
            return pipeName_;
        }

        //------------------------------------------------------------------------------------------
        file_access fileAccess() const
        {
            return file_access::read_write;
        }

    protected:
        //------------------------------------------------------------------------------------------
        posix_pipe
        (
            const path &name,
            file_access     access,
            file_flags      flags,
            file_attributes attributes
        )
            : pipeName_(name)
            , serverFd_(-1)
            , clientFd_(-1)
        {
            // (Client socket)

            // Create socket (client endpoint)
            clientFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
            if (clientFd_ == -1)
            {
                vfs_errorf("[pipe: '%s'] socket() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                return;
            }

            auto serverAddr = sockaddr_un{};
            serverAddr.sun_family = AF_UNIX;
            strncpy(serverAddr.sun_path, pipeName_.c_str(), sizeof(serverAddr.sun_path) - 1);

            // Connect client socket to server using the server address
            if (connect(clientFd_, (sockaddr *)&serverAddr, sizeof(sockaddr_un)) == -1)
            {
                vfs_errorf("[pipe: '%s'] connect() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                close();
            }
        }

        //------------------------------------------------------------------------------------------
        posix_pipe
        (
            const path &name,
            pipe_access pipeAccess
        )
            : pipeName_(name)
            , serverFd_(-1)
            , clientFd_(-1)
        {
            // (Server socket)

            // Create socket (server endpoint)
            serverFd_ = socket(AF_UNIX, SOCK_STREAM, 0);
            if (serverFd_ == -1)
            {
                vfs_errorf("[pipe: '%s'] socket() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                return;
            }

            // Bind the socket to an address (the address is the pipe name)
            auto myAddr = sockaddr_un{};
            myAddr.sun_family = AF_UNIX;
            strncpy(myAddr.sun_path, pipeName_.c_str(), sizeof(myAddr.sun_path) - 1);
            if (bind(serverFd_, (sockaddr *)&myAddr, sizeof(sockaddr_un)) == -1)
            {
                vfs_errorf("[pipe: '%s'] bind() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                close();
            }
        }

        //------------------------------------------------------------------------------------------
        ~posix_pipe()
        {
            close();
        }

    protected:
        //------------------------------------------------------------------------------------------
        bool isValid() const
        {
            return clientFd_ != -1 || serverFd_ != -1;
        }

        //------------------------------------------------------------------------------------------
        void close()
        {
            if (clientFd_ != -1)
            {
                ::close(clientFd_);
                clientFd_ = -1;
            }
            if (serverFd_ != -1)
            {
                ::close(serverFd_);
                unlink(pipeName_.c_str());
                serverFd_ = -1;
            }
        }

        //------------------------------------------------------------------------------------------
        bool waitForConnection()
        {
            // (Server socket)

            // Listen for 1 connection
            constexpr auto listenBacklog = 1;
            if (listen(serverFd_, listenBacklog) == -1)
            {
                vfs_errorf("[pipe: '%s'] listen() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }

            auto clientAddrSize = socklen_t(sizeof(sockaddr));
            auto clientAddr = sockaddr{};
            // Accept incoming connections. Block until client connects 
            clientFd_ = accept(serverFd_, &clientAddr, &clientAddrSize);

            if (clientFd_ == -1)
            {
                vfs_errorf("[pipe: '%s'] accept() failed with error: %s",
                    pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        int64_t availableBytesToRead() const
        {
            vfs_errorf("[pipe: '%s'] availableBytesToRead() is not supported on POSIX: "
                "Unix sockets do not expose pending byte count."
                "Send message length ahead of the payload instead.", pipeName_.c_str());

            return -1;
        }

        //------------------------------------------------------------------------------------------
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(clientFd_ != -1);

            auto totalBytesRead = int64_t(0);
            while (totalBytesRead < sizeInBytes)
            {
                // Read blocks until data is available, the connection closes or an error occurs
                auto bytesRead = ::read(clientFd_, dst + totalBytesRead, sizeInBytes - totalBytesRead);
                if (bytesRead == 0)
                {
                    // Connection closed by peer
                    close();
                    return totalBytesRead;
                }
                if (bytesRead == -1)
                {
                    // Error occurred
                    vfs_errorf("[pipe: '%s'] read() failed with error: %s",
                        pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                    close();
                    return 0;
                }

                totalBytesRead += bytesRead;
            }

            return totalBytesRead;
        }

        //------------------------------------------------------------------------------------------
        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(clientFd_ != -1);

            auto totalBytesWritten = int64_t(0);
            while (totalBytesWritten < sizeInBytes)
            {
                auto bytesWritten = ::write(clientFd_, src + totalBytesWritten, sizeInBytes - totalBytesWritten);
                if (bytesWritten == -1)
                {
                    vfs_errorf("[pipe: '%s'] write() failed with error: %s",
                        pipeName_.c_str(), get_last_error_as_string(errno).c_str());
                    close();
                    break;
                }

                totalBytesWritten += bytesWritten;
            }

            return totalBytesWritten;
        }

    private:
        //------------------------------------------------------------------------------------------
        path    pipeName_;
        int32_t serverFd_;
        int32_t clientFd_;
    };

} /*vfs*/
