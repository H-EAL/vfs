#pragma once

// TODO. posix_pipe is a work in progress. There is no direct equivalent to a windows pipe.
// The options in posix are fifo pipes (uni-directional) and sockets (bi-directional). This uses sockets presently, as windows pipes are bi-directional.

// ISSUE WITH PORTABILITY (in the case of unix-sockets).

//There is no good function to get the available bytes to read for unix sockets.

// If a socket is blocking andwe read past the amount that there is, the read() call will block.
// Therefore to complete a read it may be optimal to have the read be nonblocking so that we don't have to guess the number of bytes there is to read.
// In this case read() will return if there is nothing to read rather than block.

// However, in win_pipe we specify PIPE_WAIT to CreateNamedPipe(...) which enables blocking mode.This means that ReadFile does in fact block until there is something to read or a client is connected.
// Issues:

// 1. We want to specify non blocking mode to our socket to perform the read(since availableBytesToRead() does not function for posix_pipe), so there is no good way to block this read.
// Solution could be to pass an argument bool block to pipe::read() to specify if want it to block or not, but this could also alter functionality of win_pipe(although we could make block default to true).

// 2. With the way unix sockets work, we cannot do a read() until client is connected since our clientFd_ that we will be providing to the read() will be - 1.
// If we are the server, we MUST call waitForConnection() before read().

#include <chrono>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "vfs/platform.hpp"
#include "vfs/posix_file_flags.hpp"
#include "vfs/path.hpp"

// LISTEN_BACKLOG corresponds to the maximum length to which the queue of pending conections for socketFd_ may grow.
#define LISTEN_BACKLOG 50 


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
            // TODO. return socketFd_ or clientFd_ ?
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
            return fileAccess_;
        }

    protected:
        //------------------------------------------------------------------------------------------
        posix_pipe
        (
            const path &name,
            file_access             access,
            file_flags              flags,
            file_attributes         attributes
        )
            : pipeName_(name)
            , fileAccess_(access)
            , socketFd_(-1)
            , clientFd_(-1)
        {
            // CLIENT. 

            // TO DO. Do something appropriate with access.
            // https://stackoverflow.com/questions/2588213/is-there-a-way-to-close-a-unix-socket-for-only-reading-or-writing
            // Don't think there is a good way to make a socket read only or write only. Maybe I can check the permissions before the call to read() and write() ?

            // Create a socket with the socket() system call.
            clientFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

            // Not sure if shutdown works with read() and write() operations however.
            //if (access == file_access::read_only)
            //{
            //    // Disables further send operations.
            //    shutdown(clientFd_, SHUT_WR);
            //}
            //else if (access == file_access::write_only)
            //{
            //    // Disables further receive operations.
            //    shutdown(clientFd_, SHUT_RD);
            //}

            auto serverAddr = sockaddr_un{};
            // Clear structure
            memset(&serverAddr, 0, sizeof(sockaddr_un));
            serverAddr.sun_family = AF_UNIX;
            strncpy(serverAddr.sun_path, pipeName_.c_str(), sizeof(serverAddr.sun_path) - 1);

            // Connect the socket to the address of the server using the connect() system call.
            if (connect(clientFd_, (sockaddr *)&serverAddr, sizeof(sockaddr_un)) == -1)
            {
                vfs_errorf("connect() failed with error: %s", get_last_error_as_string(errno).c_str());
            }
        }

        //------------------------------------------------------------------------------------------
        posix_pipe
        (
            const path &name,
            pipe_access pipeAccess
        )
            : pipeName_(name)
            , socketFd_(-1)
            , clientFd_(-1)
        {
            // SERVER.
            
            // Create a socket with the socket() system call.
            // Posix pipe access should be SOCK_STREAM. 
            // 0 might have to be changed to appropriate protocol. 
            socketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

            if (socketFd_ == -1)
            {
                vfs_errorf("socket() failed with error: %s", get_last_error_as_string(errno).c_str());
                return;
            }

            // Bind the socket to an address using the bind() system call.
            auto myAddr = sockaddr_un{};
            // Clear structure
            memset(&myAddr, 0, sizeof(sockaddr_un));
            myAddr.sun_family = AF_UNIX;
            strncpy(myAddr.sun_path, pipeName_.c_str(), sizeof(myAddr.sun_path) - 1);

            if (bind(socketFd_, (sockaddr *)&myAddr, sizeof(sockaddr_un)) == -1)
            {
                vfs_errorf("bind() failed with error: %s", get_last_error_as_string(errno).c_str());
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
            return clientFd_ != -1 || socketFd_ != -1;
        }

        //------------------------------------------------------------------------------------------
        void close()
        {
            if (socketFd_ != -1)
            {
                ::close(socketFd_);
            }
            if (clientFd_ != -1)
            {
                ::close(clientFd_);
            }
            if (isValid())
            {
                unlink(pipeName_.c_str());
                clientFd_ = -1;
                socketFd_ = -1;
            }
        }

        //------------------------------------------------------------------------------------------
        bool waitForConnection()
        {
            // Server.
            
            // Listen for connections with the listen() system call. 
            // listen() marks the socket referred to by socketFd_ as a passive socket, e.g. as a socket that will be used to accept incoming connection requests using accept().
            if (listen(socketFd_, LISTEN_BACKLOG) == -1)
            {
                vfs_errorf("listen() failed with error: %s", get_last_error_as_string(errno).c_str());
            }
            
            // Now we can accept incoming connections.
            auto peer_addr_size = (uint32_t)sizeof(sockaddr_un);
            auto peer_addr      = sockaddr_un{};
            // The accept() system call causes the process to block until a client connects to the server.
            // It returns a new file descriptor, and all communication on this connection should be done using the new file descriptor. 
            clientFd_ = accept(socketFd_, (sockaddr *)&peer_addr, (socklen_t *)&peer_addr_size);

            if (clientFd_ == -1)
            {
                vfs_errorf("accept() failed with error: %s", get_last_error_as_string(errno).c_str());
                return false;
            }

            return true;
        }

        //------------------------------------------------------------------------------------------
        int64_t availableBytesToRead() const
        {
            // There is no function for determining how many bytes are left to read on a unix domain socket.
            // To read a message whose length cannot be determined in advance, just do a non-blocking reads until one returns EAGAIN or EWOULDBLOCK.
            
            // Source: https://stackoverflow.com/questions/65204661/is-there-a-function-for-determining-how-many-bytes-are-left-to-read-on-a-unix-do

            return -1;
        }

        //------------------------------------------------------------------------------------------
        int64_t read(uint8_t *dst, int64_t sizeInBytes)
        {
            vfs_check(clientFd_ != -1);

            // Read() is nonblocking since we specified the flag O_NONBLOCK to clientFd_.
            // It will either read the total number of characters in the socket or 255, whichever is less, and return the number of characters read.
            auto numberOfBytesRead = ::read(clientFd_, dst, sizeInBytes);

            if (numberOfBytesRead == -1)
            {
                vfs_errorf("read() failed with error: %s", get_last_error_as_string(errno).c_str());
                close();
                return 0;
            }

            return numberOfBytesRead;
        }

        //------------------------------------------------------------------------------------------
        int64_t write(const uint8_t *src, int64_t sizeInBytes)
        {
            vfs_check(clientFd_ != -1);

            auto numberOfBytesWritten = ::write(clientFd_, src, sizeInBytes);
            if (numberOfBytesWritten == -1)
            {
                vfs_errorf("write() failed with error: %s", get_last_error_as_string(errno).c_str());
                close();
            }

            return numberOfBytesWritten;
        }

    private:
        //------------------------------------------------------------------------------------------
        path        pipeName_;
        int32_t     socketFd_;
        int32_t     clientFd_;
        file_access fileAccess_;
    };

} /*vfs*/
