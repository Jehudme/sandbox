# How-To: Define FlatBuffer Schemas

To pass complex data across the ABI boundary, you must define a `.fbs` schema.

## 1. Create the Schema

Create `player_events.fbs`:

```fbs
namespace MyGame.Events;

table PlayerJump {
  force: float;
  is_double_jump: bool;
}

root_type PlayerJump;
```

## 2. Compile the Schema

Use `flatc` to generate the C++ headers.

```bash
flatc --cpp player_events.fbs
```

This generates `player_events_generated.h` which you can include in your plugin to serialize and deserialize the payload data.
