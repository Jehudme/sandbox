#include <sandbox/sandbox.h>
#include <iostream>


int main()
{
    SANDBOX_LOG_INFO("Sandbox application started.");
    sandbox::logger logger("SandboxLogger", sandbox::logger::level::debug, true);


    sandbox::properties properties = sandbox::properties();

    properties.set({"window", "width"}, 800);
    properties.set({"window", "height"}, 600);

    std::cout << properties.show() << std::endl;
    return 0;
}
