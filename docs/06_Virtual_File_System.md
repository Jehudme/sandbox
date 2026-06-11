# 6.  The Virtual File System (VFS) Security

The Sandbox Engine uses PhysicsFS (PhysFS) as its backend to implement a highly secure Virtual File System (VFS).

## Jailed Execution
By design, plugins running inside the Sandbox Engine are **jailed**. They cannot arbitrarily browse the host user's hard drive or read sensitive files (e.g., `/etc/passwd` or `C:\Windows\System32`).

*   Any call to `sandbox::sdk::filesystem` APIs using physical paths will be immediately rejected.
*   The VFS restricts all file operations to predefined virtual alias paths.

## Mounting & Aliases
The engine explicitly mounts three specific locations at boot:

1.  **`mount://bin/`**: The absolute physical directory where the `sandbox` executable resides. It is read-only and used to locate engine-native dynamic libraries.
2.  **`mount://app/`**: The application directory or archive specified by the `--mount` launcher argument. It can be a `.zip`, `.tar`, or folder. It is read-only to prevent mutating distributed application files.
3.  **`mount://cache/`**: The strictly isolated writable directory provided by the OS (e.g., `~/.local/share/sandbox/` on Linux). This is where save games, cached modules, and database files must reside.

### Path Traversal Defenses
The VFS prevents directory climbing attacks (`../../`) inside the writable cache. The internal `resolve_physical_write_path` function validates the requested virtual path by iterating and comparing the resulting real path against the locked root cache directory, ensuring the resolved path physically stays inside the sandbox environment.
