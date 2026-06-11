# 12. VFS Jailed Execution Security

*This explanation explores how the Virtual File System secures the engine against rogue plugins.*

## Jailed Execution
If you allow third-party plugins in your game engine, a malicious (or simply poorly-written) plugin might execute:
`fs.write("C:/Windows/System32/config.sys", "corrupted");` or `fs.read("/etc/passwd")`.

The Sandbox Engine explicitly **jails** all plugins. The C++ SDK wrappers (`sandbox::sdk::filesystem`) and the underlying C-ABI structs *only* accept "virtual" paths, not physical OS paths.

## Virtual Aliases
When the engine boots, it mounts predefined physical OS locations to Virtual Aliases using PhysicsFS.
Plugins interact entirely with these aliases.

1.  **`mount://app/`**: Maps to the folder or archive (e.g., `test-app.zip`) passed in the `--mount` launcher CLI argument. It is explicitly mounted as **read-only**. A plugin cannot overwrite the core application files.
2.  **`mount://bin/`**: Maps to the executable's directory. Read-only.
3.  **`mount://cache/`**: Maps to a secure, isolated AppData directory provided by the host OS (e.g., `~/.local/share/sandbox/`). This is the **only** writable location in the entire engine.

## Defeating Directory Traversal
What happens if a malicious plugin tries to use relative climbing paths to escape the jail?
`fs.write("mount://cache/../../../../../etc/passwd", "corrupt");`

The engine intercepts this at the C++ implementation level in `src/subsystems/filesystem/filesystem.cpp`. 
Inside the `resolve_physical_write_path` function, the engine takes the incoming string, resolves all `..` tokens mathematically, and produces a final absolute OS path.

It then executes an iterator-based `std::mismatch` check against the locked physical root of the cache directory. If the requested resolved path does not mathematically begin with the exact string of the locked root, the operation is flagged as a traversal attack and rejected instantly, preventing any escape from the sandbox.
