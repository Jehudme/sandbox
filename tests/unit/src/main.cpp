// sandbox_unit_tests/src/main.cpp
//
// Catch2 main entry point for sandbox unit tests.
// Using Catch2's provided main (CATCH_CONFIG_RUNNER would be needed for custom
// setup — we rely on the default main from Catch2::Catch2WithMain instead,
// but since we link Catch2::Catch2 we supply our own).

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
