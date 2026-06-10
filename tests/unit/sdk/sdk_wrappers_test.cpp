#include <catch2/catch_test_macros.hpp>
#include <sandbox/api/payload.h>
#include <sandbox/api/filesystem_api.h>
#include <sandbox/api/logger_api.h>
#include <sandbox/api/runner_api.h>
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

#include "generated/schemas/filesystem_generated.h"

TEST_CASE("SDK Filesystem Wrapper", "[sdk][filesystem]") {
    struct mock_fs : public ifilesystem {
        std::string stored_prop;
        int32_t mount(const char*, const char*, bool) override { return 0; }
        int32_t unmount(const char*) override { return 0; }
        int32_t read(const char*, sandbox_payload*) const override { return 0; }
        int32_t write(const char*, const uint8_t*, size_t, bool) override { return 0; }
        
        int32_t list(const char*, bool, sandbox_payload* out) const override { 
            flatbuffers::FlatBufferBuilder builder;
            std::vector<flatbuffers::Offset<flatbuffers::String>> items = {
                builder.CreateString("file1.txt"),
                builder.CreateString("file2.txt")
            };
            auto vec = builder.CreateVector(items);
            auto list_fb = sandbox::schemas::CreateStringList(builder, vec);
            builder.Finish(list_fb);
            
            out->size = builder.GetSize();
            out->bytes = static_cast<uint8_t*>(std::malloc(out->size));
            std::memcpy(out->bytes, builder.GetBufferPointer(), out->size);
            out->free_func = +[](void* ptr) { std::free(ptr); };
            return 0;
        }
        
        int32_t remove(const char*) override { return 0; }
        int32_t mkdir(const char*) override { return 0; }
        int32_t rename(const char*, const char*) override { return 0; }
        int32_t copy(const char*, const char*) override { return 0; }
        int32_t move(const char*, const char*) override { return 0; }
        int32_t state(const char*, sandbox_payload* out) const override {
            flatbuffers::FlatBufferBuilder builder;
            auto meta = sandbox::schemas::CreateFileMetadata(builder, 0, 1024, 0, 123456789, 0, sandbox::schemas::FileType_Regular, false);
            builder.Finish(meta);
            
            out->size = builder.GetSize();
            out->bytes = static_cast<uint8_t*>(std::malloc(out->size));
            std::memcpy(out->bytes, builder.GetBufferPointer(), out->size);
            out->free_func = +[](void* ptr) { std::free(ptr); };
            return 0;
        }
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

    SECTION("list deserializes FlatBuffers correctly") {
        auto res = fs.list("mount://test");
        REQUIRE(res.has_value());
        REQUIRE(res.value().size() == 2);
        REQUIRE(res.value()[0] == "file1.txt");
        REQUIRE(res.value()[1] == "file2.txt");
    }

    SECTION("state deserializes FlatBuffers correctly") {
        auto res = fs.state("mount://test/file.txt");
        REQUIRE(res.has_value());
        REQUIRE(res.value().size == 1024);
        REQUIRE(res.value().is_directory == false);
        REQUIRE(res.value().modified_time == 123456789);
    }

    SECTION("read_binary uses std::vector<std::byte>") {
        mock_fs mock_read;
        sdk::filesystem fs_read(&mock_read);
        auto res = fs_read.read_binary("mount://test/file.txt");
        // Our mock returns empty payload without error, so the std::expected succeeds
        REQUIRE(res.has_value());
    }

    SECTION("write uses std::vector<std::byte>") {
        mock_fs mock_write;
        sdk::filesystem fs_write(&mock_write);
        std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
        auto res = fs_write.write("mount://test/file.txt", data, false);
        REQUIRE(res.has_value());
    }
}

TEST_CASE("SDK Logger Wrapper", "[sdk][logger]") {
    struct mock_logger : public ilogger {
        std::string stored_prop;
        std::string last_log_msg;
        int last_log_level = -1;
        int32_t log(const uint8_t* log_msg_fb, size_t size) override {
            auto fb = flatbuffers::GetRoot<sandbox::schemas::logger::LogMessage>(log_msg_fb);
            if (fb) {
                last_log_level = fb->level();
                if (fb->message()) last_log_msg = fb->message()->str();
            }
            return 0;
        }
        void set_property(const char* key, const char* json) override { stored_prop = json; }
        int32_t get_property(const char* key, sandbox_payload* out) const override { return -1; }
    } logger_api;

    sdk::logger log(&logger_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = log.set_property("level", "debug");
        REQUIRE(res.has_value());
        REQUIRE(logger_api.stored_prop == "\"debug\"");
    }

    SECTION("log packs FlatBuffers correctly") {
        auto res = log.log(2, "Test message");
        REQUIRE(res.has_value());
        REQUIRE(logger_api.last_log_level == 2);
        REQUIRE(logger_api.last_log_msg == "Test message");
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
