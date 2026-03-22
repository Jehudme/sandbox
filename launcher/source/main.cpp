#include <sandbox/sandbox.h>
#include <iostream>

#include "sandbox/core/logger.h"

int main()
{
    sandbox::logger logger("SandboxLogger", sandbox::logger::level::debug, true);
    logger.debug("Hello, world! {}", "This is a test log message.");

    sandbox::properties properties = sandbox::properties();

    properties.set({"window", "width"}, 800);
    properties.set({"window", "height"}, 600);

    std::cout << properties.show() << std::endl;
    return 0;
}
