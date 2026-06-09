#include <catch2/catch_test_macros.hpp>
#include "sandbox/core/abi_types.h"
#include <vector>
#include <cstring>
#include <memory>

static int s_allocations = 0;
static int s_deallocations = 0;

void test_free_func(void* ptr) {
    uint8_t* p = static_cast<uint8_t*>(ptr);
    delete[] p;
    s_deallocations++;
}

TEST_CASE("ABI Payload Memory Safety", "[core][abi]") {
    s_allocations = 0;
    s_deallocations = 0;

    SECTION("Allocate, set, and correctly free using free_func") {
        const char* test_data = "Hello FlatBuffers";
        size_t len = std::strlen(test_data);
        
        uint8_t* mem = new uint8_t[len];
        std::memcpy(mem, test_data, len);
        s_allocations++;

        sandbox_payload payload;
        payload.bytes = mem;
        payload.size = len;
        payload.free_func = test_free_func;

        REQUIRE(payload.size == len);
        REQUIRE(std::memcmp(payload.bytes, test_data, len) == 0);

        // Simulate free
        if (payload.free_func) {
            payload.free_func(payload.bytes);
        }

        REQUIRE(s_allocations == 1);
        REQUIRE(s_deallocations == 1);
    }
}
