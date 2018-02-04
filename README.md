# redis-monitor
Fun project to learn some cpp libraries :
- [cpp_redis](https://github.com/Cylix/cpp_redis): Redis client
- [uWebSocket](https://github.com/uNetworking/uWebSockets): Websocket and http server
- [cpr](https://github.com/whoshuu/cpr): Python Requests in cpp (http client)
- [billboard.js (C3js fork)](https://naver.github.io/billboard.js): Chart library in js based on d3
- [cxxopts](https://github.com/jarro2783/cxxopts): Command line argument parsing
- [inja](https://github.com/pantor/inja): Template Engine similar to python Jinja
- [rocksdb](https://github.com/facebook/rocksdb): Embedded database

# Basic information

Rocks database store all processed data:
k + 000 + YYYY-MM-DD HH:MM:SS   -> key length of the first key in command line list
p + 000 + YYYY-MM-DD HH:MM:SS   -> # of pub per time step for the first pattern
c + 000                         -> counter to track # of pub for the first pattern

Note: 
- It seem that billboard/c3js have memory leak on updating datas. In the current state, memory will leak on each websocket call for update :(

TODO:
- Test with charjs instead of c3, hopping they don't have memory leak (charjs use canvas and c3js is svg based).
- Clean code add, abstraction for rocksdb to be able to desactivate it (persistance) and only using standard C++ type
- add alias for keys in command line with a default the index (KeyOfTheYears -> KTY [default:001])
