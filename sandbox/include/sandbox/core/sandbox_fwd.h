#pragma once

// Forward declarations for all major classes in the Sandbox meta-engine.
// Include this header (instead of full headers) wherever only a pointer or
// reference to one of these types is needed.

namespace sandbox
{
    class engine;
    class extension;
    class properties;
    class registry;
    class logger;
    class scoped_logger;

    namespace extensions
    {
        class caches;
        class clock;
        class dependencies;
        class diagnostics;
        class events;
        class filesystem;
        class logger;
        class scopes;
        class serializer;
        class stages;
        class storage;
        class systems;
        class triggers;
    }
}
