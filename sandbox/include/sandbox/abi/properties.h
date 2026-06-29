#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque pointer representing sandbox::sdk::properties_t */
typedef struct sandbox_properties sandbox_properties_t;

/* Mirrors sandbox::sdk::properties_t::Format */
typedef enum {
    SANDBOX_FORMAT_JSON = 0,
    SANDBOX_FORMAT_BEVE,
    SANDBOX_FORMAT_TOML,
    SANDBOX_FORMAT_YAML
} sandbox_properties_format_t;

/* ========================================================================== */
/* LIFECYCLE & PARSING                                                        */
/* ========================================================================== */

sandbox_properties_t* sandbox_properties_create(void);
void                  sandbox_properties_destroy(sandbox_properties_t* props);

/* Returns true on success, false if parsing fails (exception caught internally) */
bool sandbox_properties_load(sandbox_properties_t* props, const char* data, size_t data_length, sandbox_properties_format_t format);

/* Returns a dynamically allocated string. Must be freed with sandbox_properties_free_string */
char* sandbox_properties_dump(const sandbox_properties_t* props, sandbox_properties_format_t format);
void  sandbox_properties_free_string(char* str);

/* ========================================================================== */
/* TREE MANIPULATION                                                          */
/* ========================================================================== */

void sandbox_properties_clear(sandbox_properties_t* props, const char* path_str);
bool sandbox_properties_has(const sandbox_properties_t* props, const char* path_str);

/* Iterates over all keys at the given path. Calls the callback for each key. */
void sandbox_properties_keys(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* key, void* ctx), void* ctx);

void sandbox_properties_merge(sandbox_properties_t* props, const char* path_str, const sandbox_properties_t* other);

/* Returns a newly allocated properties object containing the sub-tree. Must be destroyed. */
sandbox_properties_t* sandbox_properties_sub(const sandbox_properties_t* props, const char* path_str);

/* ========================================================================== */
/* GETTERS (Returns true if key exists and type matches)                      */
/* ========================================================================== */

bool sandbox_properties_get_int64(const sandbox_properties_t* props, const char* path_str, int64_t* out_val);
bool sandbox_properties_get_uint64(const sandbox_properties_t* props, const char* path_str, uint64_t* out_val);
bool sandbox_properties_get_double(const sandbox_properties_t* props, const char* path_str, double* out_val);
bool sandbox_properties_get_bool(const sandbox_properties_t* props, const char* path_str, bool* out_val);

/* Invokes the callback with the string pointer. If not found or type mismatch, invokes with nullptr. Pointer is only valid during the callback. */
void sandbox_properties_read_string(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data);

/* Array readers */
void sandbox_properties_read_int64_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(int64_t value, void* user_data), void* user_data);
void sandbox_properties_read_uint64_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(uint64_t value, void* user_data), void* user_data);
void sandbox_properties_read_double_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(double value, void* user_data), void* user_data);
void sandbox_properties_read_bool_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(bool value, void* user_data), void* user_data);
void sandbox_properties_read_string_array(const sandbox_properties_t* props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data);

/* ========================================================================== */
/* SETTERS                                                                    */
/* ========================================================================== */

void sandbox_properties_set_int64(sandbox_properties_t* props, const char* path_str, int64_t val);
void sandbox_properties_set_uint64(sandbox_properties_t* props, const char* path_str, uint64_t val);
void sandbox_properties_set_double(sandbox_properties_t* props, const char* path_str, double val);
void sandbox_properties_set_bool(sandbox_properties_t* props, const char* path_str, bool val);
void sandbox_properties_set_string(sandbox_properties_t* props, const char* path_str, const char* val);

void sandbox_properties_set_int64_array(sandbox_properties_t* props, const char* path_str, const int64_t* values, size_t count);
void sandbox_properties_set_uint64_array(sandbox_properties_t* props, const char* path_str, const uint64_t* values, size_t count);
void sandbox_properties_set_double_array(sandbox_properties_t* props, const char* path_str, const double* values, size_t count);
void sandbox_properties_set_bool_array(sandbox_properties_t* props, const char* path_str, const bool* values, size_t count);
void sandbox_properties_set_string_array(sandbox_properties_t* props, const char* path_str, const char** values, size_t count);

#ifdef __cplusplus
}
#endif