#pragma once 

class Application final
{
public:
    class ApplicationDelegate
    {
        virtual void start() = 0;
        virtual void update() = 0;
        virtual void shutdown() = 0;

        virtual ~ApplicationDelegate() {}
    };


public:
    Application(ApplicationDelegate* delegate);

    Application(Application&& rhs);
    Application(Application const& rhs) = delete;

    Application& operator=(Application&& rhs);
    Application& operator=(Application const& rhs) = delete;

    ~Application();

private:
    ApplicationDelegate* delegate_;
};