#pragma once

#include "../class_features/NonCopyable.hpp"

class ByteBuffer;

class IOManager
    : public NonCopyable
{
public:
    IOManager();
    IOManager(IOManager&& rhs);
    IOManager& operator=(IOManager&& rhs);

    std::uint64_t ReadFileToBuffer(char const* path, ByteBuffer& buffer);

    ~IOManager();

private:

};