cmake_minimum_required(VERSION 3.16)
project(MarketDataFeedHandler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# ---- Testing ----
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_executable(unit_tests
    tests/test_parser.cpp
    tests/test_order_book.cpp
    src/parser.cpp
    src/order_book.cpp
)
target_link_libraries(unit_tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
