#pragma once

#include "sandbox/abi/handle.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Specifies the serialization format for properties.
 */
typedef enum {
    SANDBOX_FORMAT_JSON = 0,
    SANDBOX_FORMAT_BEVE,
    SANDBOX_FORMAT_TOML,
    SANDBOX_FORMAT_YAML
} sandbox_properties_format_t;

/**
 * @brief Defines the properties handle.
 */
SANDBOX_DEFINE_HANDLE(sandbox_properties_handle_t);

/**
 * @brief Creates a new properties handle.
 * @return The new properties handle.
 */
sandbox_properties_handle_t sandbox_properties_create(void);

/**
 * @brief Destroys a properties handle.
 * @param props Pointer to the handle to destroy.
 */
void sandbox_properties_destroy(sandbox_properties_handle_t* props);


/**
 * @brief Loads properties from a string buffer.
 * @param props The properties handle.
 * @param data The string buffer.
 * @param data_length The length of the string buffer.
 * @param format The format of the data.
 * @return True on success, false on failure.
 */
bool sandbox_properties_load(sandbox_properties_handle_t props, const char* data, size_t data_length, sandbox_properties_format_t format);


/**
 * @brief Dumps properties to a string.
 * @param props The properties handle.
 * @param format The output format.
 * @return A dynamically allocated string representing the properties. Must be freed.
 */
char* sandbox_properties_dump(sandbox_properties_handle_t props, sandbox_properties_format_t format);

/**
 * @brief Frees a string returned by sandbox_properties_dump.
 * @param str The string to free.
 */
void  sandbox_properties_free_string(char* str);



/**
 * @brief Clears the property at the specified path.
 * @param props The properties handle.
 * @param path_str The path to clear.
 */
void sandbox_properties_clear(sandbox_properties_handle_t props, const char* path_str);

/**
 * @brief Checks if a property exists at the specified path.
 * @param props The properties handle.
 * @param path_str The path to check.
 * @return True if the property exists, false otherwise.
 */
bool sandbox_properties_has(sandbox_properties_handle_t props, const char* path_str);


/**
 * @brief Iterates over the keys of an object property.
 * @param props The properties handle.
 * @param path_str The path to the object.
 * @param callback The callback invoked for each key.
 * @param ctx User data passed to the callback.
 */
void sandbox_properties_keys(sandbox_properties_handle_t props, const char* path_str, void (*callback)(const char* key, void* ctx), void* ctx);


/**
 * @brief Merges another properties object into the specified path.
 * @param props The destination properties handle.
 * @param path_str The path to merge into.
 * @param other The properties to merge from.
 */
void sandbox_properties_merge(sandbox_properties_handle_t props, const char* path_str, sandbox_properties_handle_t other);


/**
 * @brief Retrieves a sub-tree as a new properties handle.
 * @param props The parent properties handle.
 * @param path_str The path to the sub-tree.
 * @return A new properties handle representing the sub-tree.
 */
sandbox_properties_handle_t sandbox_properties_sub(sandbox_properties_handle_t props, const char* path_str);



/**
 * @brief Gets an int64 value.
 * @param props The properties handle.
 * @param path_str The path to the value.
 * @param out_val Pointer to store the value.
 * @return True on success.
 */
bool sandbox_properties_get_int64(sandbox_properties_handle_t props, const char* path_str, int64_t* out_val);

/**
 * @brief Gets a uint64 value.
 * @param props The properties handle.
 * @param path_str The path to the value.
 * @param out_val Pointer to store the value.
 * @return True on success.
 */
bool sandbox_properties_get_uint64(sandbox_properties_handle_t props, const char* path_str, uint64_t* out_val);

/**
 * @brief Gets a double value.
 * @param props The properties handle.
 * @param path_str The path to the value.
 * @param out_val Pointer to store the value.
 * @return True on success.
 */
bool sandbox_properties_get_double(sandbox_properties_handle_t props, const char* path_str, double* out_val);

/**
 * @brief Gets a bool value.
 * @param props The properties handle.
 * @param path_str The path to the value.
 * @param out_val Pointer to store the value.
 * @return True on success.
 */
bool sandbox_properties_get_bool(sandbox_properties_handle_t props, const char* path_str, bool* out_val);


/**
 * @brief Reads a string value via callback.
 * @param props The properties handle.
 * @param path_str The path to the value.
 * @param callback The callback invoked with the string value.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_string(sandbox_properties_handle_t props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data);


/**
 * @brief Reads an int64 array via callback.
 * @param props The properties handle.
 * @param path_str The path to the array.
 * @param callback The callback invoked for each element.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_int64_array(sandbox_properties_handle_t props, const char* path_str, void (*callback)(int64_t value, void* user_data), void* user_data);

/**
 * @brief Reads a uint64 array via callback.
 * @param props The properties handle.
 * @param path_str The path to the array.
 * @param callback The callback invoked for each element.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_uint64_array(sandbox_properties_handle_t props, const char* path_str, void (*callback)(uint64_t value, void* user_data), void* user_data);

/**
 * @brief Reads a double array via callback.
 * @param props The properties handle.
 * @param path_str The path to the array.
 * @param callback The callback invoked for each element.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_double_array(sandbox_properties_handle_t props, const char* path_str, void (*callback)(double value, void* user_data), void* user_data);

/**
 * @brief Reads a bool array via callback.
 * @param props The properties handle.
 * @param path_str The path to the array.
 * @param callback The callback invoked for each element.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_bool_array(sandbox_properties_handle_t props, const char* path_str, void (*callback)(bool value, void* user_data), void* user_data);

/**
 * @brief Reads a string array via callback.
 * @param props The properties handle.
 * @param path_str The path to the array.
 * @param callback The callback invoked for each element.
 * @param user_data User data passed to the callback.
 */
void sandbox_properties_read_string_array(sandbox_properties_handle_t props, const char* path_str, void (*callback)(const char* value, void* user_data), void* user_data);



/**
 * @brief Sets an int64 value.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param val The value to set.
 */
void sandbox_properties_set_int64(sandbox_properties_handle_t props, const char* path_str, int64_t val);

/**
 * @brief Sets a uint64 value.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param val The value to set.
 */
void sandbox_properties_set_uint64(sandbox_properties_handle_t props, const char* path_str, uint64_t val);

/**
 * @brief Sets a double value.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param val The value to set.
 */
void sandbox_properties_set_double(sandbox_properties_handle_t props, const char* path_str, double val);

/**
 * @brief Sets a bool value.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param val The value to set.
 */
void sandbox_properties_set_bool(sandbox_properties_handle_t props, const char* path_str, bool val);

/**
 * @brief Sets a string value.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param val The value to set.
 */
void sandbox_properties_set_string(sandbox_properties_handle_t props, const char* path_str, const char* val);


/**
 * @brief Sets an int64 array.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param values The array values.
 * @param count The number of elements.
 */
void sandbox_properties_set_int64_array(sandbox_properties_handle_t props, const char* path_str, const int64_t* values, size_t count);

/**
 * @brief Sets a uint64 array.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param values The array values.
 * @param count The number of elements.
 */
void sandbox_properties_set_uint64_array(sandbox_properties_handle_t props, const char* path_str, const uint64_t* values, size_t count);

/**
 * @brief Sets a double array.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param values The array values.
 * @param count The number of elements.
 */
void sandbox_properties_set_double_array(sandbox_properties_handle_t props, const char* path_str, const double* values, size_t count);

/**
 * @brief Sets a bool array.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param values The array values.
 * @param count The number of elements.
 */
void sandbox_properties_set_bool_array(sandbox_properties_handle_t props, const char* path_str, const bool* values, size_t count);

/**
 * @brief Sets a string array.
 * @param props The properties handle.
 * @param path_str The path to set.
 * @param values The array values.
 * @param count The number of elements.
 */
void sandbox_properties_set_string_array(sandbox_properties_handle_t props, const char* path_str, const char** values, size_t count);


#ifdef __cplusplus
}
#endif
