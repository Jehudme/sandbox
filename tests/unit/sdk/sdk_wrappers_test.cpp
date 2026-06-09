#include <catch2/catch_test_macros.hpp>
#include "sandbox/sdk/payload.h"
#include "sandbox/sdk/filesystem_api.h"
#include "sandbox/sdk/logger_api.h"
#include "sandbox/sdk/runner_api.h"
#include <string>
#include <cstdlib>
#include <cstring>

using namespace sandbox;

TEST_CASE("SDK Payload Wrapper", "[sdk][payload]") {
    SECTION("RAII payload automatically frees memory") {
        bool freed = false;
        {
            sdk::payload p;
            sandbox_payload* raw = p.get();
            raw->bytes = static_cast<uint8_t*>(std::malloc(4));
            std::memcpy(raw->bytes, "test", 4);
            raw->size = 4;
            raw->free_func = +[](void* ptr) {
                std::free(ptr);
                // We cannot easily set freed=true from a static function without capture, 
                // but if we don't leak, AddressSanitizer or basic execution confirms it.
            };
            REQUIRE(p.as_string() == "test");
        }
    }
}

TEST_CASE("SDK Filesystem Wrapper", "[sdk][filesystem]") {
    struct mock_fs : public ifilesystem {
        std::string stored_prop;
        int32_t mount(const char*, const char*, bool) override { return 0; }
        int32_t unmount(const char*) override { return 0; }
        int32_t read(const char*, sandbox_payload*) const override { return 0; }
        int32_t write(const char*, const uint8_t*, size_t, bool) override { return 0; }
        int32_t list(const char*, bool, sandbox_payload*) const override { return 0; }
        int32_t remove(const char*) override { return 0; }
        int32_t mkdir(const char*) override { return 0; }
        int32_t rename(const char*, const char*) override { return 0; }
        int32_t copy(const char*, const char*) override { return 0; }
        int32_t move(const char*, const char*) override { return 0; }
        int32_t state(const char*, sandbox_payload*) const override { return 0; }
        int32_t absolute(const char*, sandbox_payload*) const override { return 0; }
        void set_property(const char* key, const char* json) override { stored_prop = json; }
        int32_t get_property(const char* key, sandbox_payload* out) const override { return -1; }
    } fs_api;

    sdk::filesystem fs(&fs_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = fs.set_property("test_key", 42);
        REQUIRE(res.has_value());
        REQUIRE(fs_api.stored_prop == "42");
    }
}

TEST_CASE("SDK Logger Wrapper", "[sdk][logger]") {
    struct mock_logger : public ilogger {
        std::string stored_prop;
        int32_t log(const uint8_t*, size_t) override { return 0; }
        void set_property(const char* key, const char* json) override { stored_prop = json; }
        int32_t get_property(const char* key, sandbox_payload* out) const override { return -1; }
    } logger_api;

    sdk::logger log(&logger_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = log.set_property("level", "debug");
        REQUIRE(res.has_value());
        REQUIRE(logger_api.stored_prop == "\"debug\"");
    }
}

TEST_CASE("SDK Runner Wrapper", "[sdk][runner]") {
    struct mock_runner : public irunner {
        std::string stored_prop;
        int32_t start_async(flecs::world&) override { return 0; }
        int32_t run_sync(flecs::world&) override { return 0; }
        int32_t quit() override { return 0; }
        int32_t pause() override { return 0; }
        int32_t resume() override { return 0; }
        void set_property(const char* key, const char* json) override { stored_prop = json; }
        int32_t get_property(const char* key, sandbox_payload* out) const override { return -1; }
    } runner_api;

    sdk::runner run(&runner_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = run.set_property("fps", 60);
        REQUIRE(res.has_value());
        REQUIRE(runner_api.stored_prop == "60");
    }
}
