#include <iostream>
#include <fstream>
#include <filesystem>
#include "../../../launcher/source/cli/cli_parser.h"

int main() {
    std::filesystem::path p1 = "dummy_project_1.json";
    std::ofstream out1(p1);
    out1 << R"({
        "engine": {
            "libraries": ["dummy_plugin.so"],
            "sandbox": ["test::sys-Renderer@1.0.0"]
        }
    })";
    out1.close();

    std::string config_arg = p1.string();
    std::vector<const char*> args = {"sandbox_launcher", "--config", config_arg.c_str()};
    
    sandbox_properties_t* props = sandbox::launcher::parse_cli(args.size(), const_cast<char**>(args.data()));
    if (!props) { std::cout << "null props\n"; return 1; }

    char* dump = sandbox_properties_dump(props, SANDBOX_FORMAT_JSON);
    std::cout << "Dump: " << (dump ? dump : "null") << "\n";
    if (dump) free(dump);
    return 0;
}
