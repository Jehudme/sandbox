#include <catch2/catch_session.hpp>

#include <catch2/catch_test_macros.hpp>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}

TEST_CASE("Suite Placeholder", "[suite]") {
    REQUIRE(true);
}
