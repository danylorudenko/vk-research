#pragma once

#include "../class_features/NonCopyable.hpp"
#include "../memory/ByteBuffer.hpp"

class IOManager
    : public NonCopyable
{
    IOManager();
    IOManager(IOManager&& rhs);
    IOManager& operator=(IOManager&& rhs);

    ByteBuffer ReadFileToBuffer(char const* path);

    ~IOManager();

private:

};