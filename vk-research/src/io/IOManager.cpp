#include "IOManager.hpp"
#include "../memory/ByteBuffer.hpp"

#include <fstream>
#include <iostream>

IOManager::IOManager() {}

IOManager::IOManager(IOManager&& rhs) {}

IOManager& IOManager::operator=(IOManager&& rhs)
{
    return *this;
}

IOManager::~IOManager() {}

std::uint64_t IOManager::ReadFileToBuffer(char const* path, ByteBuffer& buffer)
{
    std::ifstream istream{ path, std::ios_base::binary | std::ios_base::beg };
    if (!istream) {
        std::cerr << "Error opening file in path: " << path << std::endl;
        return 0;
    }

    auto const fileSize = istream.seekg(0, std::ios_base::end).tellg();
    if (buffer.Size() < static_cast<std::uint64_t>(fileSize)) {
        buffer.Resize(fileSize);
    }

    istream.read(buffer.As<char*>(), fileSize);

    return fileSize;
}