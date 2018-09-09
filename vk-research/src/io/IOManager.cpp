#include "IOManager.hpp"

#include <fstream>
#include <iostream>

IOManager::IOManager() {}

IOManager::IOManager(IOManager&& rhs) {}

IOManager& IOManager::operator=(IOManager&& rhs)
{
    return *this;
}

IOManager::~IOManager() {}

ByteBuffer IOManager::ReadFileToBuffer(char const* path)
{
    std::ifstream istream{ path };
    if (!istream) {
        std::cerr << "Error opening file in path: " << path << std::endl;
        return ByteBuffer{};
    }

    std::uint64_t constexpr defaultInputStep = 2048;

    ByteBuffer memoryBuffer{ defaultInputStep };
    while (!istream.eof()) {
        istream.read(memoryBuffer.As<char>(), defaultInputStep);
    }
}