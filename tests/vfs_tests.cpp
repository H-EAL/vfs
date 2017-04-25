#include <iostream>
#include "vfs.hpp"


int main()
{
    auto fv = vfs::open_read_write_view(L"toto.txt", vfs::file_creation_options::open_or_create);

    (*fv) << 42;

    return EXIT_SUCCESS;
}