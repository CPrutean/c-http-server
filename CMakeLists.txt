cmake_minimum_required(VERSION 3.23)

project(http-server)

add_library(http-server-lib STATIC
  src/server.c
  src/http_parser.c
  src/router.c
  src/thread_pool.c
)

target_include_directories(http-server-lib PUBLIC include)

set_target_properties(http-server-lib PROPERTIES C_STANDARD 11)

if(WIN32)
  target_link_libraries(http-server-lib PRIVATE w2_32)
elseif(UNIX)
  target_link_libraries(http-server-lib PRIVATE pthread)
endif()


add_executable(http-server src/main.c)
target_link_libraries(http-server PRIVATE http-server-lib)
set_target_properties(http-server PROPERTIES C_STANDARD 11)




if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(http-server-lib PRIVATE /W4)
else()
  target_compile_options(http-server-lib PRIVATE -Wall -Wextra -pedantic)
endif()
