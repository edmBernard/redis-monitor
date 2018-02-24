#include "catch.hpp"
#include "database.hpp"

TEST_CASE("stl database random prefix", "[Database]") {
  eb::StlDatabase database;

  database.hset("prefix1", "key1", "11");
  REQUIRE(database.hgetall("prefix1").at("key1") == "11");

  database.hset("prefix2", "key1", "21");
  REQUIRE(database.hgetall("prefix2").at("key1") == "21");
  REQUIRE(database.hgetall("prefix1").at("key1") == "11");

  database.hset("prefix1", "key2", "12");
  REQUIRE(database.hgetall("prefix1").at("key2") == "12");
}

TEST_CASE("stl database prefix", "[Database]") {
  eb::StlDatabase database;

  REQUIRE(database.buildPrefix('c', 1) == "c001");

  database.prefixLength = 10;
  REQUIRE(database.buildPrefix('d', 2) == "d000000002");

  database.prefixLength = 2;
  REQUIRE(database.buildPrefix('e', 3) == "e3");
}

TEST_CASE("stl database normalized prefix", "[Database]") {
  eb::StlDatabase database;

  database.hset(database.buildPrefix('a', 1), "key1", "11");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key1") == "11");

  database.hset(database.buildPrefix('b', 1), "key1", "21");
  REQUIRE(database.hgetall(database.buildPrefix('b', 1)).at("key1") == "21");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key1") == "11");

  database.hset(database.buildPrefix('a', 1), "key2", "12");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key2") == "12");
}

TEST_CASE("rocksdb database normalized prefix", "[Database]") {
  eb::RocksdbDatabase database;

  database.hset(database.buildPrefix('a', 1), "key1", "11");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key1") == "11");

  database.hset(database.buildPrefix('b', 1), "key1", "21");
  REQUIRE(database.hgetall(database.buildPrefix('b', 1)).at("key1") == "21");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key1") == "11");

  database.hset(database.buildPrefix('a', 1), "key2", "12");
  REQUIRE(database.hgetall(database.buildPrefix('a', 1)).at("key2") == "12");
}
