#pragma once

#include "vfs/logging.hpp"
#include "file_flags.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    inline DWORD win_file_access(file_access access)
    {
        switch (access)
        {
        case file_access::read_only:    return GENERIC_READ;
        case file_access::write_only:   return GENERIC_WRITE;
        case file_access::read_write:   return GENERIC_READ | GENERIC_WRITE;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline DWORD win_file_share_mode(file_share_mode shareMode)
    {
        auto mode = DWORD{ 0 };

        if (uint32_t(shareMode) & uint32_t(file_share_mode::can_delete))
            mode |= FILE_SHARE_DELETE;

        if (uint32_t(shareMode) & uint32_t(file_share_mode::read))
            mode |= FILE_SHARE_READ;

        if (uint32_t(shareMode) & uint32_t(file_share_mode::write))
            mode |= FILE_SHARE_WRITE;

        return mode;
    }

    //----------------------------------------------------------------------------------------------
    inline DWORD win_file_creation_options(file_creation_options creationOption)
    {
        switch (creationOption)
        {
        case file_creation_options::create_if_nonexisting:  return CREATE_NEW;
        case file_creation_options::create_or_overwrite:    return CREATE_ALWAYS;
        case file_creation_options::open_if_existing:       return OPEN_EXISTING;
        case file_creation_options::open_or_create:         return OPEN_ALWAYS;
        case file_creation_options::truncate_existing:      return TRUNCATE_EXISTING;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline DWORD win_file_flags(file_flags flags)
    {
        auto f = DWORD{ 0 };

        if (uint32_t(flags) & uint32_t(file_flags::delete_on_close))
            f |= FILE_FLAG_DELETE_ON_CLOSE;

        if (uint32_t(flags) & uint32_t(file_flags::sequential_scan))
            f |= FILE_FLAG_SEQUENTIAL_SCAN;

        if (uint32_t(flags) & uint32_t(file_flags::write_through))
            f |= FILE_FLAG_WRITE_THROUGH;

        return f;
    }

    //----------------------------------------------------------------------------------------------
    inline DWORD win_file_attributes(file_attributes attributes)
    {
        auto attr = DWORD{ 0 };

        if (uint32_t(attributes) & uint32_t(file_attributes::normal))
            attr |= FILE_ATTRIBUTE_NORMAL;

        if (uint32_t(attributes) & uint32_t(file_attributes::temporary))
            attr |= FILE_ATTRIBUTE_TEMPORARY;

        return attr;
    }

    //----------------------------------------------------------------------------------------------
    inline DWORD win_pipe_access(pipe_access access)
    {
        switch (access)
        {
        case pipe_access::inbound:  return PIPE_ACCESS_INBOUND;
        case pipe_access::outbound: return PIPE_ACCESS_OUTBOUND;
        case pipe_access::duplex:   return PIPE_ACCESS_DUPLEX;
        }

        vfs_check(false);
        return 0;
    }

} /*vfs*/
