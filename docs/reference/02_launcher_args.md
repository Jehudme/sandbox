# Reference: Launcher CLI Arguments

The Sandbox Engine launcher accepts several command-line arguments.

```text
Usage: ./bin/sandbox [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -m,--mount TEXT:PATH        (REQUIRED) Path to the application archive or directory.
                              Can be a folder or an archive (like .zip).
  -d,--dev                    Enable developer mode (verbose logging and tracing).
  -r,--run                    Run the engine main loop immediately after boot.
  -p,--prop [TEXT,TEXT] ...   Custom module properties in Key=Value format.
```

## Examples

Run a zipped game archive:
```bash
./sandbox --mount my_game.zip --run
```

Run in developer mode with custom properties:
```bash
./sandbox --mount ./game_data/ --dev -p fps_limit=60 -p logger_level=1
```
