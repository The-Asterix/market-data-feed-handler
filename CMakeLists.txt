cmake_minimum_required(VERSION 3.16)
project(MarketDataFeedHandler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release-level optimization for realistic benchmark numbers.
# Without this, an unoptimized debug build will report much slower,
# misleading numbers.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
find_package(Threads REQUIRED)

add_executable(feed_handler src/main.cpp src/parser.cpp src/order_book.cpp)
add_executable(generate_data src/generate_sample_data.cpp)

add_executable(publisher src/publisher.cpp src/network.cpp)
target_link_libraries(publisher Threads::Threads)

add_executable(subscriber src/subscriber.cpp src/parser.cpp src/order_book.cpp src/network.cpp)
target_link_libraries(subscriber Threads::Threads)

add_executable(benchmark src/benchmark.cpp src/parser.cpp src/order_book.cpp)
