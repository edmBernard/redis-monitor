#include "catch.hpp"
#include "monitor.hpp"

TEST_CASE("unique counter", "[Tic]") {

  rm::Tic counter;
  REQUIRE(counter.get() == 0);

  counter.incr();
  REQUIRE(counter.get() == 1);

  counter.reset();
  REQUIRE(counter.get() == 0);

  for (int i = 0; i < 100; ++i) {
    counter.incr();
  }
  REQUIRE(counter.get() == 100);
  counter.reset();
  REQUIRE(counter.get() == 0);
}

TEST_CASE("multiple counter entanglement", "[Tic]") {

  rm::Tic counter1;
  rm::Tic counter2;
  REQUIRE(counter1.get() == 0);
  counter1.incr();
  REQUIRE(counter1.get() == 1);

  REQUIRE(counter2.get() == 0);
  counter2.incr();
  REQUIRE(counter2.get() == 1);

  counter1.reset();
  REQUIRE(counter1.get() == 0);

  REQUIRE(counter2.get() == 1);
  counter2.reset();
  REQUIRE(counter2.get() == 0);

  for (int i = 0; i < 100; ++i) {
    counter1.incr();
    counter2.incr();
  }
  REQUIRE(counter1.get() == 100);
  REQUIRE(counter2.get() == 100);
  counter1.reset();
  REQUIRE(counter1.get() == 0);
  REQUIRE(counter2.get() == 100);
}