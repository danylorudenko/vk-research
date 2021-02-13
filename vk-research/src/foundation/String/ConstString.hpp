#pragma once

#include <foundation\Common.hpp>

DRE_BEGIN_NAMESPACE

class ConstString
{
public:
    ConstString()
        : m_String  { nullptr }
        , m_Size    { 0 }
    {
    }

    template<U32 STR_SIZE>
    ConstString(char const (&str)[STR_SIZE])
        : m_String  { str }
        , m_Size    { STR_SIZE - 1 }
    {
    }

    ConstString(ConstString const& rhs)
        : m_String  { rhs.m_String }
        , m_Size    { rhs.m_Size }
    { 
    }

    ConstString(ConstString&& rhs)
        : m_String  { rhs.m_String }
        , m_Size    { rhs.m_Size }
    {
    }

    ConstString& operator=(ConstString const& rhs)
    {
        m_String    = rhs.m_String;
        m_Size      = rhs.m_Size;
        return *this;
    }

    ConstString& operator=(ConstString&& rhs)
    {
        m_String    = rhs.m_String;
        m_Size      = rhs.m_Size;

        return *this;
    }

    ~ConstString() { }

    char const* Data() const
    {
        return m_String;
    }

    U32 Size() const
    {
        return m_Size;
    }

    bool operator==(char const* rhs) const
    {
        return std::strcmp(m_String, rhs) == 0;
    }

    operator char const*() const
    {
        return m_String;
    }

private:
    char const* m_String;
    U32         m_Size;
};

DRE_END_NAMESPACE

