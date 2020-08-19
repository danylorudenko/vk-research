#pragma once 

#include <class_features\NonCopyable.hpp>

class Application final
    : public NonCopyable
{
public:
    class ApplicationDelegate
        : public NonCopyable
    {
    public:
        virtual void start() = 0;
        virtual void update() = 0;
        virtual void shutdown() = 0;

        virtual ~ApplicationDelegate() {}
    };


public:
    Application(ApplicationDelegate* delegate);

    Application(Application&& rhs);
    Application& operator=(Application&& rhs);

    ~Application();

    void run();

private:
    ApplicationDelegate* delegate_;
};