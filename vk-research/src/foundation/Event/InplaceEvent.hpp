#pragma once

#include <foundation\Container\InplaceVector.hpp>

DRE_BEGIN_NAMESPACE

template<U32 MAX_EVENT_HANDLERS, typename... EventArgT>
class InplaceEvent
{
public:
    InplaceEvent()
        : m_EventHandlerStorage{}
    {}

    InplaceEvent(InplaceEvent<MAX_EVENT_HANDLERS, EventArgT...>&& rhs)
    {
        operator=(DRE_MOVE(rhs));
    }
    
    InplaceEvent& operator=(InplaceEvent<MAX_EVENT_HANDLERS, EventArgT...>&& rhs)
    {
        DRE_ASSERT(rhs.m_EventHandlerStorage.Size() == 0, "Can't move into non-empty InplaceEvent");
    
        m_EventHandlerStorage = DRE_MOVE(rhs.m_EventHandlerStorage);
    }

    template<typename T, void(T::*TMethod)(EventArgT...)>
    static void CallDataMember(void* data, EventArgT... eventArgs)
    {
        T* dataTyped = (T*)data;
        (dataTyped->*TMethod)(std::forward<EventArgT>(eventArgs)...);
    }

    template<typename T, void(T::*TMethod)(EventArgT...)>
    void Subscribe(T* object)
    {
        m_EventHandlerStorage.EmplaceBack(CallDataMember<T, TMethod>, object);
    }

    void Unsubscribe(void* object)
    {
        U32 const count = m_EventHandlerStorage.Size();
        for (U32 i = 0; i < count; ++i)
        {
            if (m_EventHandlerStorage[i].m_Data == object)
            {
                m_EventHandlerStorage.Remove(object);
                return;
            }
        }
    }

    void Trigger(EventArgT... arg)
    {
        U32 const count = m_EventHandlerStorage.Size();
        for (U32 i = 0; i < count; ++i)
        {
            EventHandlerContainer& container = m_EventHandlerStorage[i];
            container.m_Callback(container.m_Data, std::forward<EventArgT>(arg)...);
        }
    }


private:
    struct EventHandlerContainer
    {
        using EventCallback = void(*)(void*, EventArgT...);

        EventHandlerContainer(EventCallback callback, void* data)
            : m_Callback{ callback }
            , m_Data{ data }
        {}

        EventCallback m_Callback;
        void* m_Data;
    };

private:
    InplaceVector<EventHandlerContainer, MAX_EVENT_HANDLERS> m_EventHandlerStorage;
};

DRE_END_NAMESPACE

