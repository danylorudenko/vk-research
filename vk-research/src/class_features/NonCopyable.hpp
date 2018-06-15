#pragma once

class NonCopyable
{
public:
    NonCopyable(){}
    NonCopyable(NonCopyable const&) = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
};