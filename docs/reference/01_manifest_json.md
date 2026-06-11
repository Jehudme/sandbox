# Reference: Manifest.json

The `manifest.json` file is required for the bootstrapper to locate and load modules.

## Structure

```json
{
  "modules": [
    {
      "id": "my_module",
      "version": "1.0.0",
      "entry_point": "my_module_shared_lib",
      "requirements": [
        {
          "id": "core_logger",
          "kind": "import",
          "strictness": "required"
        }
      ]
    }
  ]
}
```

## Fields

- `id`: The unique identifier for the module.
- `version`: Semantic version string.
- `entry_point`: The name of the shared library file (without the `.dll` or `.so` extension). The engine will automatically append the correct OS-specific extension.
- `requirements`: An array of dependencies.
  - `id`: The target module ID.
  - `kind`: `import` (must run before this module) or `export` (runs after this module).
  - `strictness`: `required` (fail if missing) or `optional` (continue if missing).
