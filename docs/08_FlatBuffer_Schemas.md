# 8. FlatBuffer Schemas

*Reference list of the standard FlatBuffer payloads used by the engine's internal services.*

## `logger.fbs`
Used by the `logger_service` to transfer log requests across the ABI boundary.
```flatbuffers
namespace sandbox::schemas::logger;

enum LogLevel : byte { Trace = 0, Debug, Info, Warn, Error, Fatal }

table LogMessage {
  level: LogLevel = Info;
  message: string (required);
  source_file: string;
  source_line: int;
  throw_on_error: bool = false;
}

root_type LogMessage;
```

## `filesystem.fbs`
Used by the `filesystem_service` to return complex data structures (arrays, metadata) back to the caller safely.
```flatbuffers
namespace sandbox::schemas;

table StringList {
  items: [string];
}

enum FileType : byte { File = 0, Directory, Symlink, Other }

table FileMetadata {
  size: uint64;
  type: FileType;
  modification_time: uint64;
}
```
