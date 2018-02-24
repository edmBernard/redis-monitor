#include "catch.hpp"

#include "database.hpp"
#include "monitor.hpp"

TEST_CASE("MonitorLength with stl database", "[Monitor]") {
  rm::StlDatabase database;
  rm::MonitorLength monitor(database, 1);

  monitor.add("MYDATE1", "1");
  SECTION("get from monitor") {
    REQUIRE(monitor.get().count("MYDATE1") == 1);
    REQUIRE(monitor.get().count("MYDATE2") == 0);
    REQUIRE(monitor.get().at("MYDATE1") == "1");
    REQUIRE(monitor.lastData.first == "MYDATE1");
    REQUIRE(monitor.lastData.second == "1");
  }
  SECTION("multi key") {
    monitor.add("MYDATE2", "2");
    SECTION("get from monitor") {
      REQUIRE(monitor.get().count("MYDATE1") == 1);
      REQUIRE(monitor.get().count("MYDATE2") == 1);
      REQUIRE(monitor.get().at("MYDATE2") == "2");
      REQUIRE(monitor.lastData.first == "MYDATE2");
      REQUIRE(monitor.lastData.second == "2");
    }
  }
}

TEST_CASE("MonitorLength with stl database and build in prefix", "[Monitor]") {
  rm::StlDatabase database;
  rm::MonitorLength monitor(database, 2);

  monitor.add("MYDATE1", "1");
  SECTION("get from monitor") {
    REQUIRE(monitor.get().count("MYDATE1") == 1);
    REQUIRE(monitor.get().count("MYDATE2") == 0);
    REQUIRE(monitor.get().at("MYDATE1") == "1");
    REQUIRE(monitor.lastData.first == "MYDATE1");
    REQUIRE(monitor.lastData.second == "1");
  }
  SECTION("multi key") {
    monitor.add("MYDATE2", "2");
    SECTION("get from monitor") {
      REQUIRE(monitor.get().count("MYDATE1") == 1);
      REQUIRE(monitor.get().count("MYDATE2") == 1);
      REQUIRE(monitor.get().at("MYDATE2") == "2");
      REQUIRE(monitor.lastData.first == "MYDATE2");
      REQUIRE(monitor.lastData.second == "2");
    }
  }
}

TEST_CASE("multi MonitorLength with stl database", "[Monitor]") {
  rm::StlDatabase database;
  rm::MonitorLength monitor1(database, 1);
  rm::MonitorLength monitor2(database, 2);
  rm::MonitorLength monitor3(database, 3);

  monitor1.add("MYDATE1", "1");
  monitor2.add("MYDATE1", "2");
  monitor3.add("MYDATE1", "3");

  REQUIRE(monitor1.get().count("MYDATE1") == 1);
  REQUIRE(monitor1.get().count("MYDATE2") == 0);
  REQUIRE(monitor1.get().at("MYDATE1") == "1");
  REQUIRE(monitor1.lastData.first == "MYDATE1");
  REQUIRE(monitor1.lastData.second == "1");
  REQUIRE(monitor2.get().count("MYDATE1") == 1);
  REQUIRE(monitor2.get().count("MYDATE2") == 0);
  REQUIRE(monitor2.get().at("MYDATE1") == "2");
  REQUIRE(monitor2.lastData.first == "MYDATE1");
  REQUIRE(monitor2.lastData.second == "2");
  REQUIRE(monitor3.get().count("MYDATE1") == 1);
  REQUIRE(monitor3.get().count("MYDATE2") == 0);
  REQUIRE(monitor3.get().at("MYDATE1") == "3");
  REQUIRE(monitor3.lastData.first == "MYDATE1");
  REQUIRE(monitor3.lastData.second == "3");
}

TEST_CASE("MonitorLength with rocks database and build in prefix", "[Monitor]") {
  rm::RocksdbDatabase database("/tmp/redis_monitor", true);
  rm::MonitorLength monitor(database, 1);

  monitor.add("MYDATE1", "1");
  SECTION("get from monitor") {
    REQUIRE(monitor.get().count("MYDATE1") == 1);
    REQUIRE(monitor.get().count("MYDATE2") == 0);
    REQUIRE(monitor.get().at("MYDATE1") == "1");
    REQUIRE(monitor.lastData.first == "MYDATE1");
    REQUIRE(monitor.lastData.second == "1");
  }
  SECTION("multi key") {
    monitor.add("MYDATE2", "2");
    SECTION("get from monitor") {
      REQUIRE(monitor.get().count("MYDATE1") == 1);
      REQUIRE(monitor.get().count("MYDATE2") == 1);
      REQUIRE(monitor.get().at("MYDATE2") == "2");
      REQUIRE(monitor.lastData.first == "MYDATE2");
      REQUIRE(monitor.lastData.second == "2");
    }
  }
}

TEST_CASE("multi MonitorLength with rocks database", "[Monitor]") {
  rm::RocksdbDatabase database("/tmp/redis_monitor", true);
  rm::MonitorLength monitor1(database, 1);
  rm::MonitorLength monitor2(database, 2);
  rm::MonitorLength monitor3(database, 3);

  monitor1.add("MYDATE1", "1");
  monitor2.add("MYDATE1", "2");
  monitor3.add("MYDATE1", "3");

  REQUIRE(monitor1.get().count("MYDATE1") == 1);
  REQUIRE(monitor1.get().count("MYDATE2") == 0);
  REQUIRE(monitor1.get().at("MYDATE1") == "1");
  REQUIRE(monitor1.lastData.first == "MYDATE1");
  REQUIRE(monitor1.lastData.second == "1");
  REQUIRE(monitor2.get().count("MYDATE1") == 1);
  REQUIRE(monitor2.get().count("MYDATE2") == 0);
  REQUIRE(monitor2.get().at("MYDATE1") == "2");
  REQUIRE(monitor2.lastData.first == "MYDATE1");
  REQUIRE(monitor2.lastData.second == "2");
  REQUIRE(monitor3.get().count("MYDATE1") == 1);
  REQUIRE(monitor3.get().count("MYDATE2") == 0);
  REQUIRE(monitor3.get().at("MYDATE1") == "3");
  REQUIRE(monitor3.lastData.first == "MYDATE1");
  REQUIRE(monitor3.lastData.second == "3");
}