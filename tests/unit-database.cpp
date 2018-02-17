#include "catch.hpp"
#include "tools.hpp"

TEST_CASE("stl database", "[Database]") {
  eb::StlDatabase database;

    database.hset("prefix1", "key1", 11);
    REQUIRE(database.hgetall("prefix1").at("key1") == 11);

    database.hset("prefix2", "key1", 21);
    REQUIRE(database.hgetall("prefix2").at("key1") == 21);
    REQUIRE(database.hgetall("prefix1").at("key1") == 11);

    database.hset("prefix1", "key2", 12);
    REQUIRE(database.hgetall("prefix1").at("key2") == 12);

}

// TEST_CASE("rocksdb database", "[Database]") {
//   eb::RocksdbDatabase database;

//     database.hset("prefix1", "key1", 11);
//     REQUIRE(database.hgetall("prefix1").at("key1") == 11);

//     database.hset("prefix2", "key1", 21);
//     REQUIRE(database.hgetall("prefix2").at("key1") == 21);
//     REQUIRE(database.hgetall("prefix1").at("key1") == 11);

//     database.hset("prefix1", "key2", 12);
//     REQUIRE(database.hgetall("prefix1").at("key2") == 12);

// }

