#pragma once

#include <sys/mman.h>

#include "vfs/logging.hpp"
#include "file_flags.hpp"


namespace vfs {

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_file_access(file_access access)
    {
        switch (access)
        {
            case file_access::read_only:    return O_RDONLY;
            case file_access::write_only:   return O_WRONLY;
            case file_access::read_write:   return O_RDWR;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_memory_mapping_protection(file_access access)
    {
        switch (access)
        {
            case file_access::read_only:    return PROT_READ;
            case file_access::write_only:   return PROT_WRITE;
            case file_access::read_write:   return PROT_READ | PROT_WRITE;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_file_share_mode(file_share_mode shareMode)
    {
        switch (shareMode)
        {
        case file_share_mode::exclusive:	return 0;
        // For a user to be able to delete a file, they must have write permission for the directory and own the file they want to delete.
        case file_share_mode::can_delete:	return S_IWUSR;
        case file_share_mode::read:			return S_IRUSR;
        case file_share_mode::write:		return S_IWUSR;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_file_creation_options(file_creation_options creationOption)
    {
        switch (creationOption)
        {
            case file_creation_options::create_if_nonexisting:  return O_CREAT | O_EXCL;
            case file_creation_options::create_or_overwrite:    return O_CREAT | O_TRUNC;
            // There is no equivalent to OPEN_IF_EXISTING in posix.
            // Check if file already exists, if so then simply open it with file_creation_options::open_or_create.
            case file_creation_options::open_if_existing:       return 0;
            case file_creation_options::open_or_create:         return O_CREAT;
            case file_creation_options::truncate_existing:      return O_TRUNC;
        }

        vfs_check(false);
        return 0;
    }

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_file_flags(file_flags flags)
    {
        auto f = uint64_t{ 0 };

        if (uint32_t(flags) & uint32_t(file_flags::delete_on_close))
            // Note that if we want to use O_TMPFILE, which creates an unnamed temporary regular file, the pathname
            // provided to open() needs to specify a directory. If it specifies a file, the resulting file will not be
            // deleted on close.
            f |= O_TMPFILE;

        if (uint32_t(flags) & uint32_t(file_flags::sequential_scan))
            // f |= ;

        if (uint32_t(flags) & uint32_t(file_flags::write_through))
            f |= O_DIRECT;

        return f;
    }

    //----------------------------------------------------------------------------------------------
    inline uint64_t posix_file_attributes(file_attributes attributes)
    {
        auto attr = uint64_t{ 0 };

        //if (uint32_t(attributes) & uint32_t(file_attributes::normal))

        //if (uint32_t(attributes) & uint32_t(file_attributes::temporary))
            // attr |= ;

        return attr;
    }

} /*vfs*/
