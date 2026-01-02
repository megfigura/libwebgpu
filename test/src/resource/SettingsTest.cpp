#include <optional>
#include <string_view>
#include <catch2/catch_test_macros.hpp>

#include "resource/Settings.h"

resource::Settings loadSettings()
{
    return resource::Settings(nlohmann::json::parse(R"({
  "topString": "abc",
  "topBool": true,
  "topInt": 1234,
  "category": {
    "catString": "def",
    "catBool": false,
    "catInt": 5678
  }
})"));
}

TEST_CASE("Test top-level", "Settings")
{
    REQUIRE(loadSettings().getString("topString").value() == "abc");
    REQUIRE(loadSettings().getBool("topBool").value() == true);
    REQUIRE(loadSettings().getInt("topInt").value() == 1234);
}

TEST_CASE("Test category", "Settings")
{
    REQUIRE(loadSettings().getString("category.catString").value() == "def");
    REQUIRE(loadSettings().getBool("category.catBool").value() == false);
    REQUIRE(loadSettings().getInt("category.catInt").value() == 5678);
}

TEST_CASE("Test missing", "Settings")
{
    REQUIRE(!loadSettings().getString("missing.missingString").has_value());
    REQUIRE(!loadSettings().getBool("missingBool").has_value());
    REQUIRE(!loadSettings().getInt("missingInt").has_value());
}