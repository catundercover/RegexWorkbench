// Starts the desktop or WebAssembly application lifecycle.
#include "app/Application.hpp"

// Initializes, runs, and shuts down the application process.
int main()
{
    app::Application application;
    if (!application.initialize())
    {
        return 1;
    }
    application.run();
    application.shutdown();

    return 0;
}
