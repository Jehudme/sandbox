#include <iostream>

#include "../../sandbox/include/sandbox/utilities/properties.h"


int main()
{
    sandbox::properties props;
    props.set({"database", "host"}, "localhost");
    props.set({"database", "port"}, 5432);

    std::cout << props.to_json_string() << std::endl;
}