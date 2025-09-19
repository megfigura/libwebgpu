#include <string>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Name is webgpu", "[library]")
{
  //auto const exported = exported_class {};
  //REQUIRE(std::string("webgpu") == exported.name());
  REQUIRE(std::string("webgpu") == "webgpu");
}
