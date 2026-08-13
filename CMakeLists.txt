cmake_minimum_required(VERSION 3.16)
project(MarketDataFeedHandler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(feed_handler src/main.cpp src/parser.cpp src/order_book.cpp)
add_executable(generate_data src/generate_sample_data.cpp)
