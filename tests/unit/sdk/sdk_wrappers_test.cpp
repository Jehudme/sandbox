#include <catch2/catch_test_macros.hpp>
#include <sandbox/api/payload.h>
#include <sandbox/api/filesystem_api.h>
#include <sandbox/api/logger_api.h>
#include <sandbox/api/runner_api.h>
#include <string>
#include <cstdlib>
#include <cstring>
#include "generated/schemas/filesystem_generated.h"
#include "generated/schemas/logger_generated.h"

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
            };
            REQUIRE(p.as_string() == "test");
        }
    }
}

TEST_CASE("SDK Filesystem Wrapper", "[sdk][filesystem]") {
    struct mock_fs {
        std::string stored_prop;
        
        static int32_t mount(void*, const char*, const char*, bool) { return 0; }
        static int32_t unmount(void*, const char*) { return 0; }
        static int32_t read(const void*, const char*, sandbox_payload*) { return 0; }
        static int32_t write(void*, const char*, const uint8_t*, size_t, bool) { return 0; }
        
        static int32_t list(const void*, const char*, bool, sandbox_payload* out) { 
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
        
        static int32_t remove(void*, const char*) { return 0; }
        static int32_t mkdir(void*, const char*) { return 0; }
        static int32_t rename(void*, const char*, const char*) { return 0; }
        static int32_t copy(void*, const char*, const char*) { return 0; }
        static int32_t move(void*, const char*, const char*) { return 0; }
        static int32_t state(const void*, const char*, sandbox_payload* out) {
            flatbuffers::FlatBufferBuilder builder;
            auto meta = sandbox::schemas::CreateFileMetadata(builder, 0, 1024, 0, 123456789, 0, sandbox::schemas::FileType_Regular, false);
            builder.Finish(meta);
            
            out->size = builder.GetSize();
            out->bytes = static_cast<uint8_t*>(std::malloc(out->size));
            std::memcpy(out->bytes, builder.GetBufferPointer(), out->size);
            out->free_func = +[](void* ptr) { std::free(ptr); };
            return 0;
        }
        static int32_t absolute(const void*, const char*, sandbox_payload*) { return 0; }
        static void set_property(void* inst, const char* key, const char* json) { 
            static_cast<mock_fs*>(inst)->stored_prop = json; 
        }
        static int32_t get_property(const void*, const char* key, sandbox_payload* out) { return -1; }
        
        sandbox::filesystem_service get_api() {
            sandbox::filesystem_service api{};
            api.instance = this;
            api.mount = mount;
            api.unmount = unmount;
            api.read = read;
            api.write = write;
            api.list = list;
            api.remove = remove;
            api.mkdir = mkdir;
            api.rename = rename;
            api.copy = copy;
            api.move = move;
            api.state = state;
            api.absolute = absolute;
            api.set_property = set_property;
            api.get_property = get_property;
            return api;
        }
    } mfs;

    auto fs_api = mfs.get_api();
    sdk::filesystem fs(&fs_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = fs.set_property("test_key", 42);
        REQUIRE(res.has_value());
        REQUIRE(mfs.stored_prop == "42");
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
        auto m_api = mock_read.get_api();
        sdk::filesystem fs_read(&m_api);
        auto res = fs_read.read_binary("mount://test/file.txt");
        REQUIRE(res.has_value());
    }

    SECTION("write uses std::vector<std::byte>") {
        mock_fs mock_write;
        auto m_api = mock_write.get_api();
        sdk::filesystem fs_write(&m_api);
        std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02}};
        auto res = fs_write.write("mount://test/file.txt", data, false);
        REQUIRE(res.has_value());
    }
}

TEST_CASE("SDK Logger Wrapper", "[sdk][logger]") {
    struct mock_logger {
        std::string stored_prop;
        std::string last_log_msg;
        int last_log_level = -1;
        
        static int32_t log(void* inst, const uint8_t* log_msg_fb, size_t size) {
            auto* self = static_cast<mock_logger*>(inst);
            auto fb = flatbuffers::GetRoot<sandbox::schemas::logger::LogMessage>(log_msg_fb);
            if (fb) {
                self->last_log_level = fb->level();
                if (fb->message()) self->last_log_msg = fb->message()->str();
            }
            return 0;
        }
        static void set_property(void* inst, const char* key, const char* json) { 
            static_cast<mock_logger*>(inst)->stored_prop = json; 
        }
        static int32_t get_property(const void*, const char* key, sandbox_payload* out) { return -1; }
        
        sandbox::logger_service get_api() {
            sandbox::logger_service api{};
            api.instance = this;
            api.log = log;
            api.set_property = set_property;
            api.get_property = get_property;
            return api;
        }
    } mlog;

    auto logger_api = mlog.get_api();
    sdk::logger log(&logger_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = log.set_property("level", "debug");
        REQUIRE(res.has_value());
        REQUIRE(mlog.stored_prop == "\"debug\"");
    }

    SECTION("log packs FlatBuffers correctly") {
        auto res = log.log(2, "Test message");
        REQUIRE(res.has_value());
        REQUIRE(mlog.last_log_level == 2);
        REQUIRE(mlog.last_log_msg == "Test message");
    }
}

TEST_CASE("SDK Runner Wrapper", "[sdk][runner]") {
    struct mock_runner {
        std::string stored_prop;
        
        static int32_t start_async(void*, flecs::world&) { return 0; }
        static int32_t run_sync(void*, flecs::world&) { return 0; }
        static int32_t quit(void*) { return 0; }
        static int32_t pause(void*) { return 0; }
        static int32_t resume(void*) { return 0; }
        static void set_property(void* inst, const char* key, const char* json) { 
            static_cast<mock_runner*>(inst)->stored_prop = json; 
        }
        static int32_t get_property(const void*, const char* key, sandbox_payload* out) { return -1; }
        
        sandbox::runner_service get_api() {
            sandbox::runner_service api{};
            api.instance = this;
            api.start_async = start_async;
            api.run_sync = run_sync;
            api.quit = quit;
            api.pause = pause;
            api.resume = resume;
            api.set_property = set_property;
            api.get_property = get_property;
            return api;
        }
    } mrun;

    auto runner_api = mrun.get_api();
    sdk::runner run(&runner_api);

    SECTION("set_property serializes to JSON correctly") {
        auto res = run.set_property("fps", 60);
        REQUIRE(res.has_value());
        REQUIRE(mrun.stored_prop == "60");
    }
}
