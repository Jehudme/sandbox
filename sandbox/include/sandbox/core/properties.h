#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque pointer representing sandbox::core::properties_t */
typedef struct sandbox_properties sandbox_properties_t;

/* Mirrors sandbox::core::properties_t::Format */
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

/* Returns an array of dynamically allocated strings. Must be freed with sandbox_properties_free_keys */
char** sandbox_properties_keys(const sandbox_properties_t* props, const char* path_str, size_t* out_count);
void   sandbox_properties_free_keys(char** keys, size_t count);

void sandbox_properties_merge(sandbox_properties_t* props, const char* path_str, const sandbox_properties_t* other);

/* Returns a newly allocated properties object containing the sub-tree. Must be destroyed. */
sandbox_properties_t* sandbox_properties_sub(const sandbox_properties_t* props, const char* path_str);

/* ========================================================================== */
/* GETTERS (Returns true if key exists and type matches)                      */
/* ========================================================================== */

bool sandbox_properties_get_int64(const sandbox_properties_t* props, const char* path_str, int64_t* out_val);
bool sandbox_properties_get_double(const sandbox_properties_t* props, const char* path_str, double* out_val);
bool sandbox_properties_get_bool(const sandbox_properties_t* props, const char* path_str, bool* out_val);

/* String pointer is guaranteed valid until the next get_string call on this thread (Thread-local scratchpad) */
const char* sandbox_properties_get_string(const sandbox_properties_t* props, const char* path_str);

/* ========================================================================== */
/* SETTERS                                                                    */
/* ========================================================================== */

void sandbox_properties_set_int64(sandbox_properties_t* props, const char* path_str, int64_t val);
void sandbox_properties_set_double(sandbox_properties_t* props, const char* path_str, double val);
void sandbox_properties_set_bool(sandbox_properties_t* props, const char* path_str, bool val);
void sandbox_properties_set_string(sandbox_properties_t* props, const char* path_str, const char* val);

#ifdef __cplusplus
}
#endif