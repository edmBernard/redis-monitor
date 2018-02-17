#include "catch.hpp"
#include "tools.hpp"

TEST_CASE("counter testing", "[Tic]") {

  eb::Tic counter;

  REQUIRE(counter.get() == 0);

  SECTION("increment of one") {
    counter.incr();

    REQUIRE(counter.get() == 1);
  }
  SECTION("increment of 100") {
    for (int i = 0; i < 100; ++i) {
      counter.incr();
    }

    REQUIRE(counter.get() == 100);
  }
  SECTION("reset") {
    for (int i = 0; i < 100; ++i) {
      counter.incr();
    }
    counter.reset();
    REQUIRE(counter.get() == 0);
  }
}
