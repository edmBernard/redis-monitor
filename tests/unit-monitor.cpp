#include "catch.hpp"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"
#include "rocksdb/slice_transform.h"
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
  SECTION("multi key") {
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

TEST_CASE("MonitorLength with stl database and build in prefix", "[Monitor]") {
  eb::StlDatabase database;
  eb::MonitorLength monitor(database, database.buildPrefix('c', 1));

  monitor.add("MYDATE1", 1);
  SECTION("get from database") {
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE1") == 1);
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE2") == 0);
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).at("MYDATE1") == 1);
  }
  SECTION("get from monitor") {
    REQUIRE(monitor.get().count("MYDATE1") == 1);
    REQUIRE(monitor.get().count("MYDATE2") == 0);
    REQUIRE(monitor.get().at("MYDATE1") == 1);
    REQUIRE(monitor.lastData.first == "MYDATE1");
    REQUIRE(monitor.lastData.second == 1);
  }
  SECTION("multi key") {
    monitor.add("MYDATE2", 2);
    SECTION("get from database") {
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE1") == 1);
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE2") == 1);
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).at("MYDATE2") == 2);
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

TEST_CASE("multi MonitorLength with stl database", "[Monitor]") {
  eb::StlDatabase database;
  eb::MonitorLength monitor1(database, database.buildPrefix('c', 1));
  eb::MonitorLength monitor2(database, database.buildPrefix('c', 2));
  eb::MonitorLength monitor3(database, database.buildPrefix('d', 3));

  monitor1.add("MYDATE1", 1);
  monitor2.add("MYDATE1", 2);
  monitor3.add("MYDATE1", 3);

  REQUIRE(monitor1.get().count("MYDATE1") == 1);
  REQUIRE(monitor1.get().count("MYDATE2") == 0);
  REQUIRE(monitor1.get().at("MYDATE1") == 1);
  REQUIRE(monitor1.lastData.first == "MYDATE1");
  REQUIRE(monitor1.lastData.second == 1);
  REQUIRE(monitor2.get().count("MYDATE1") == 1);
  REQUIRE(monitor2.get().count("MYDATE2") == 0);
  REQUIRE(monitor2.get().at("MYDATE1") == 2);
  REQUIRE(monitor2.lastData.first == "MYDATE1");
  REQUIRE(monitor2.lastData.second == 2);
  REQUIRE(monitor3.get().count("MYDATE1") == 1);
  REQUIRE(monitor3.get().count("MYDATE2") == 0);
  REQUIRE(monitor3.get().at("MYDATE1") == 3);
  REQUIRE(monitor3.lastData.first == "MYDATE1");
  REQUIRE(monitor3.lastData.second == 3);
}

TEST_CASE("MonitorLength with rocks database and build in prefix", "[Monitor]") {
  eb::RocksdbDatabase database("/tmp/redis_monitor", true);
  eb::MonitorLength monitor(database, database.buildPrefix('c', 1));

  monitor.add("MYDATE1", 1);
  SECTION("get from database") {
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE1") == 1);
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE2") == 0);
    REQUIRE(database.hgetall(database.buildPrefix('c', 1)).at("MYDATE1") == 1);
  }
  SECTION("get from monitor") {
    REQUIRE(monitor.get().count("MYDATE1") == 1);
    REQUIRE(monitor.get().count("MYDATE2") == 0);
    REQUIRE(monitor.get().at("MYDATE1") == 1);
    REQUIRE(monitor.lastData.first == "MYDATE1");
    REQUIRE(monitor.lastData.second == 1);
  }
  SECTION("multi key") {
    monitor.add("MYDATE2", 2);
    SECTION("get from database") {
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE1") == 1);
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).count("MYDATE2") == 1);
      REQUIRE(database.hgetall(database.buildPrefix('c', 1)).at("MYDATE2") == 2);
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

// TEST_CASE("multi MonitorLength with rocks database", "[Monitor]") {
//   eb::RocksdbDatabase database("/tmp/redis_monitor", true);
//   eb::MonitorLength monitor1(database, database.buildPrefix('c', 1));
//   eb::MonitorLength monitor2(database, database.buildPrefix('c', 2));
//   eb::MonitorLength monitor3(database, database.buildPrefix('d', 3));

//   monitor1.add("MYDATE1", 1);
//   monitor2.add("MYDATE1", 2);
//   monitor3.add("MYDATE1", 3);

//   REQUIRE(monitor1.get().count("MYDATE1") == 1);
//   REQUIRE(monitor1.get().count("MYDATE2") == 0);
//   REQUIRE(monitor1.get().at("MYDATE1") == 1);
//   REQUIRE(monitor1.lastData.first == "MYDATE1");
//   REQUIRE(monitor1.lastData.second == 1);
//   REQUIRE(monitor2.get().count("MYDATE1") == 1);
//   REQUIRE(monitor2.get().count("MYDATE2") == 0);
//   REQUIRE(monitor2.get().at("MYDATE1") == 2);
//   REQUIRE(monitor2.lastData.first == "MYDATE1");
//   REQUIRE(monitor2.lastData.second == 2);
//   REQUIRE(monitor3.get().count("MYDATE1") == 1);
//   REQUIRE(monitor3.get().count("MYDATE2") == 0);
//   REQUIRE(monitor3.get().at("MYDATE1") == 3);
//   REQUIRE(monitor3.lastData.first == "MYDATE1");
//   REQUIRE(monitor3.lastData.second == 3);
// }