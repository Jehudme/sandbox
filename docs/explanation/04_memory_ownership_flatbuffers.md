# Explanation: Memory Ownership & FlatBuffers

Because of the C-ABI Firewall, memory ownership must be strictly managed when passing data.

## The Payload Struct
When a plugin requests data (e.g., reading a file), the engine returns a `sandbox_payload`.

```cpp
struct sandbox_payload {
    uint8_t* bytes;
    size_t size;
    void (*free_func)(void*);
};
```

**Rule 1: The Allocator Provides the Free Function**
Whichever DLL allocates the `bytes` must provide the `free_func` pointer. When the receiving DLL is finished with the data, it invokes `free_func(bytes)`. This ensures that memory is always freed by the allocator that created it, avoiding cross-heap corruption.

## FlatBuffers for IPC
To pass structured data, the engine uses **FlatBuffers**. 

FlatBuffers are zero-copy memory-mapped data structures. When an event is published via the Event Bus, the `FlatBufferBuilder` constructs a contiguous byte array. This raw array is passed across the C-ABI wall. The receiving plugin casts the raw byte pointer using `flatbuffers::GetRoot<T>()` and instantly reads the structured data without any deserialization overhead.
