# Reference: Core Services API

The engine provides core C-ABI services via the `execute_command` pattern. You interact with them via SDK wrappers.

## Logger (`sandbox::sdk::logger`)
- `log(int level, const std::string& msg)`
- `set_property(const std::string& key, const T& value)`

## Filesystem (`sandbox::sdk::filesystem`)
- `read_text(const std::string& path)` -> `std::expected<std::string>`
- `read_binary(const std::string& path)` -> `std::expected<std::vector<std::byte>>`
- `write(const std::string& path, const std::string& data, bool append)`
- `list(const std::string& path)` -> `std::expected<std::vector<std::string>>`
- `state(const std::string& path)` -> `file_state { size, is_directory, modified_time }`
- `mount(const std::string& physical, const std::string& virtual_prefix)`

## Runner (`sandbox::sdk::runner`)
- `set_property("fps_limit", float)`
