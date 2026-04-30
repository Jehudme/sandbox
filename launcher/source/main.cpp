#include <iostream>
#include <format>
#include <filesystem>
#include <atomic>

#include "sandbox/core/engine.h"
#include "sandbox/core/plugin.h"
#include "sandbox/core/dispatcher.h"
#include "sandbox/core/type_registry.h"
#include "sandbox/core/type_registration.h"
#include "sandbox/utils/properties.h"

// ============================================================
// Concrete test plugins
// ============================================================

static int g_alpha_init  = 0;
static int g_alpha_fini  = 0;
static int g_beta_init   = 0;
static bool g_beta_saw_alpha = false;

struct AlphaTag {};  // ECS tag created inside initialize()

class alpha_plugin : public sandbox::plugin
{
public:
    explicit alpha_plugin(sandbox::engine* eng) : sandbox::plugin(eng) {}

    static std::string class_name() { return "alpha_plugin"; }
    int init_count() const { return g_alpha_init; }

private:
    void initialize() override
    {
        ++g_alpha_init;
        context.ecs.entity("::alpha_marker");  // root-anchored: visible outside plugin scope
    }

    void finalize() override { ++g_alpha_fini; }
};

// beta_plugin depends on alpha_plugin — must appear after it in the manifest
class beta_plugin : public sandbox::plugin
{
public:
    explicit beta_plugin(sandbox::engine* eng) : sandbox::plugin(eng) {}

private:
    void initialize() override
    {
        ++g_beta_init;
        if (context.find_plugin<alpha_plugin>("alpha"))
            g_beta_saw_alpha = true;
    }

    void finalize() override {}
};

SANDBOX_REFLECTION
{
    rttr::registration::class_<alpha_plugin>("alpha_plugin")
        .constructor<sandbox::engine*>()(rttr::policy::ctor::as_raw_ptr)
        .method("init_count",  &alpha_plugin::init_count)
        .method("class_name",  &alpha_plugin::class_name);

    rttr::registration::class_<beta_plugin>("beta_plugin")
        .constructor<sandbox::engine*>()(rttr::policy::ctor::as_raw_ptr);
}

// ============================================================
// Minimal test harness
// ============================================================

static int g_pass = 0;
static int g_fail = 0;

static void check(bool ok, const char* label)
{
    if (ok) { std::cout << std::format("  {:<52}  PASS\n", label); ++g_pass; }
    else    { std::cout << std::format("  {:<52}  FAIL  <<<\n", label); ++g_fail; }
}

// ============================================================
// properties
// ============================================================

static void test_properties()
{
    std::cout << "\n[properties]\n";
    sandbox::properties p;

    p.set({"db", "host"}, std::string("localhost"));
    p.set({"db", "port"}, 5432);
    p.set({"db", "tls"},  true);

    check(p.get<std::string>({"db", "host"}) == "localhost",        "set/get string");
    check(p.get<int>({"db", "port"})         == 5432,               "set/get int");
    check(p.get<bool>({"db", "tls"})         == true,               "set/get bool");
    check(!p.get<std::string>({"db", "missing"}).has_value(),       "get missing → nullopt");

    check(p.contains({"db", "host"}),                               "contains (present)");
    check(!p.contains({"db", "nope"}),                              "contains (absent)");

    auto keys = p.list_keys({"db"});
    check(keys.size() == 3,                                         "list_keys count");

    check(!p.to_json_string({"db", "port"}).empty(),                "to_json_string non-empty");

    // merge
    sandbox::properties other;
    other.set({"db",    "name"}, std::string("mydb"));
    other.set({"cache", "ttl"},  60);
    p.merge(other);
    check(p.get<std::string>({"db",    "name"}) == "mydb",          "merge adds key");
    check(p.get<int>({"cache", "ttl"})          == 60,              "merge adds subtree");
    check(p.get<std::string>({"db", "host"}) == "localhost",        "merge preserves existing");

    // move
    p.move({"cache", "ttl"}, {"cache", "lifetime"});
    check(p.contains({"cache", "lifetime"}),                        "move: destination exists");
    check(!p.contains({"cache", "ttl"}),                            "move: source removed");

    // rename
    p.rename({"cache", "lifetime"}, "expiry");
    check(p.contains({"cache", "expiry"}),                          "rename: destination exists");
    check(!p.contains({"cache", "lifetime"}),                       "rename: source removed");

    // remove
    p.remove({"cache", "expiry"});
    check(!p.contains({"cache", "expiry"}),                         "remove works");

    // get_subtree
    auto sub = p.get_subtree({"db"});
    check(sub.contains({"host"}),                                   "get_subtree contains key");

    // traverse
    int nodes = 0;
    p.traverse([&](const sandbox::properties::key_path& path, const std::string&) {
        if (!path.empty()) ++nodes;
    });
    check(nodes > 0,                                                "traverse visits nodes");

    // save / load roundtrip
    const std::filesystem::path tmp = "/tmp/sandbox_props_test.json";
    p.save_to_file(tmp);
    sandbox::properties loaded;
    loaded.load_from_file(tmp);
    check(loaded.get<std::string>({"db", "host"}) == "localhost",   "save/load file roundtrip");

    // clear
    p.clear();
    check(p.list_keys({}).empty(),                                  "clear empties root");
}

// ============================================================
// type_registry (non-engine parts)
// ============================================================

static void test_type_registry()
{
    std::cout << "\n[type_registry]\n";

    check(sandbox::type_registry::has_type("alpha_plugin"),         "has_type (registered)");
    check(!sandbox::type_registry::has_type("no_such_plugin"),      "has_type (unknown)");
    check(sandbox::type_registry::get_type_metadata_name("alpha_plugin") == "alpha_plugin",
          "get_type_metadata_name");
}

// ============================================================
// engine + plugin lifecycle
// ============================================================

static void test_engine()
{
    std::cout << "\n[engine + plugin lifecycle]\n";

    sandbox::properties manifest;
    manifest.set({"plugins", "alpha", "type"}, std::string("alpha_plugin"));
    manifest.set({"plugins", "beta",  "type"}, std::string("beta_plugin"));

    sandbox::engine eng;
    eng.initialize(manifest);

    check(g_alpha_init == 1,                                        "initialize() called on alpha_plugin");
    check(g_beta_init  == 1,                                        "initialize() called on beta_plugin");
    check(g_beta_saw_alpha,                                         "beta_plugin resolved alpha_plugin dependency");

    // get_plugin
    sandbox::plugin* raw = eng.get_plugin("alpha");
    check(raw != nullptr,                                           "get_plugin returns non-null");

    // find_plugin<T>
    auto* typed = eng.find_plugin<alpha_plugin>("alpha");
    check(typed != nullptr,                                         "find_plugin<alpha_plugin> correct type");
    check(eng.find_plugin<beta_plugin>("alpha") == nullptr,         "find_plugin<wrong_type> → nullptr");

    // ECS entity created inside alpha_plugin::initialize()
    check(eng.ecs.lookup("::alpha_marker").is_alive(),              "ECS entity created inside initialize()");

    // call_method (instance method via RTTR)
    rttr::instance inst(*typed);
    auto result = sandbox::type_registry::call_method("init_count", inst);
    check(result.is_valid() && result.get_value<int>() == 1,        "call_method (init_count)");

    // call_static_method
    auto sr = sandbox::type_registry::call_static_method("alpha_plugin", "class_name");
    check(sr.is_valid() && sr.get_value<std::string>() == "alpha_plugin",
          "call_static_method (class_name)");

    // create_plugin programmatically
    eng.create_plugin("extra", "alpha_plugin");
    check(eng.get_plugin("extra") != nullptr,                       "create_plugin (programmatic)");
    check(g_alpha_init == 2,                                        "initialize() called on programmatic plugin");

    // delete_plugin
    int fini_before = g_alpha_fini;
    eng.delete_plugin("extra");
    check(eng.get_plugin("extra") == nullptr,                       "delete_plugin removes plugin");
    check(g_alpha_fini == fini_before + 1,                          "finalize() called on deleted plugin");

    // engine destructor calls finalize() on remaining plugins
    {
        int fini_snap = g_alpha_fini;
        {
            sandbox::engine temp;
            sandbox::properties m2;
            m2.set({"plugins", "a", "type"}, std::string("alpha_plugin"));
            temp.initialize(m2);
        } // ~engine() should call finalize()
        check(g_alpha_fini == fini_snap + 1,                        "~engine() calls finalize() on remaining plugins");
    }
}

// ============================================================
// dispatcher
// ============================================================

struct PlayerDied { int player_id; };
struct LevelLoaded { std::string name; };

static void test_dispatcher()
{
    std::cout << "\n[dispatcher]\n";

    sandbox::engine eng;
    sandbox::properties manifest;
    manifest.set({"plugins", "events", "type"}, std::string("sandbox::dispatcher"));
    eng.initialize(manifest);

    auto* bus = eng.find_plugin<sandbox::dispatcher>("events");
    check(bus != nullptr,                                           "dispatcher loaded from manifest");

    // subscribe + publish for PlayerDied
    int death_count = 0;
    int last_id     = -1;
    bus->subscribe<PlayerDied>([&](const PlayerDied& e) {
        ++death_count;
        last_id = e.player_id;
    });

    bus->publish(std::make_unique<PlayerDied>(PlayerDied{42}));
    check(death_count == 1 && last_id == 42,                        "publish dispatches to subscriber");

    bus->publish(std::make_unique<PlayerDied>(PlayerDied{7}));
    check(death_count == 2 && last_id == 7,                         "second publish updates state");

    // publish with no subscribers (must not crash)
    bus->publish(std::make_unique<LevelLoaded>(LevelLoaded{"world1"}));
    check(true,                                                     "publish with no subscribers is safe");

    // multiple subscribers on the same event type
    int second_count = 0;
    bus->subscribe<PlayerDied>([&](const PlayerDied&) { ++second_count; });
    bus->publish(std::make_unique<PlayerDied>(PlayerDied{1}));
    check(death_count == 3 && second_count == 1,                    "multiple subscribers both receive event");

    // subscribe to LevelLoaded now
    std::string loaded_name;
    bus->subscribe<LevelLoaded>([&](const LevelLoaded& e) { loaded_name = e.name; });
    bus->publish(std::make_unique<LevelLoaded>(LevelLoaded{"dungeon"}));
    check(loaded_name == "dungeon",                                 "subscribe works for second event type");
}

// ============================================================
// main
// ============================================================

int main()
{
    std::cout << "=== Sandbox Framework Smoke Test ===\n";

    test_properties();
    test_type_registry();
    test_engine();
    test_dispatcher();

    std::cout << std::format("\n=== {} passed  {} failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
