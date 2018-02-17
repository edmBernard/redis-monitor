#include "catch.hpp"
#include "tools.hpp"

TEST_CASE("MonitorLength with stl database", "[Monitor]") {
  eb::StlDatabase database;
  eb::MonitorLength monitor(database, "prefix1");

  monitor.add("MYDATE1", 1);
  SECTION("get from database") {
      REQUIRE(database.hgetall("prefix1").count("MYDATE1") == 1);
      REQUIRE(database.hgetall("prefix1").count("MYDATE2") == 0);
      REQUIRE(database.hgetall("prefix1").at("MYDATE1") == 1);
  }
  SECTION("get from monitor") {
      REQUIRE(monitor.get().count("MYDATE1") == 1);
      REQUIRE(monitor.get().count("MYDATE2") == 0);
      REQUIRE(monitor.get().at("MYDATE1") == 1);
      REQUIRE(monitor.lastData.first == "MYDATE1");
      REQUIRE(monitor.lastData.second == 1);

  }
  SECTION("multi key")
  {
    monitor.add("MYDATE2", 2);
    SECTION("get from database") {
        REQUIRE(database.hgetall("prefix1").count("MYDATE1") == 1);
        REQUIRE(database.hgetall("prefix1").count("MYDATE2") == 1);
        REQUIRE(database.hgetall("prefix1").at("MYDATE2") == 2);
    }
    SECTION("get from monitor") {
        REQUIRE(monitor.get().count("MYDATE1") == 1);
        REQUIRE(monitor.get().count("MYDATE2") == 1);
        REQUIRE(monitor.get().at("MYDATE2") == 2);
        REQUIRE(monitor.lastData.first == "MYDATE2");
        REQUIRE(monitor.lastData.second == 2);
    }
  }
}
