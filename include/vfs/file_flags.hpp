#pragma once

#include <cstdint>


namespace vfs {

    enum class file_creation_options
    {
        create_if_nonexistant,
        create_or_overwrite,
        open_if_existant,
        open_or_create,
        truncate_existing
    };

	enum class file_share_mode
	{
		exclusive,
		can_delete,
		read,
		write
	};

    enum class file_flags : uint32_t
    {
        none                = 0,
        sequential_scan     = 1 << 0,
        delete_on_close     = 1 << 1
    };

    enum class file_attributes : uint32_t
    {
        none                = 0,
        normal              = 1 << 0,
        temporary           = 1 << 1
    };

    enum class file_access : uint32_t
    {
        read_only,
        write_only,
        read_write,
    };

} /*vfs*/
