# redis-monitor
Fun project to learn some cpp libraries :
- [cpp_redis](https://github.com/Cylix/cpp_redis): Redis client
- [uWebSocket](https://github.com/uNetworking/uWebSockets): Websocket and http server
- [cpr](https://github.com/whoshuu/cpr): Python Requests in cpp (http client)
- [billboard.js (C3js fork)](https://naver.github.io/billboard.js): Chart library in js based on d3
- [cxxopts](https://github.com/jarro2783/cxxopts): Command line argument parsing
- [inja](https://github.com/pantor/inja): Template Engine similar to python Jinja
- [rocksdb](https://github.com/facebook/rocksdb): Embedded database

# Dependency

* uWebsocket depends on openssl
```bash
apt-get install libssl-dev
```
* redis_cpp depends on libuv
```bash
apt-get install libuv1-dev
```

# Basic information

Rocks database store all processed data:
- `k + 000 + YYYY-MM-DD HH:MM:SS` : key length of the first key in command line list
- `p + 000 + YYYY-MM-DD HH:MM:SS` : # of pub per time step for the first pattern
- `c + 000` : counter to track # of pub for the first pattern

TODO:
- Clean code add abstraction for rocksdb to be able to desactivate it (persistance) and only using standard C++ type (map/vector)
- add 3 letters alias for keys in command line currently key alias in database is its index in command line 
