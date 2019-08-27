cmake_minimum_required(VERSION 3.1)

project(telescopes)
include_directories(.)

#set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_GCC_FLAGS "-Wall -Wextra -Wconversion -pedantic -std=gnu11")

set(THREADS_PREFER_PTHREAD_FLAG ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")
find_package(Allegro REQUIRED)
find_package(Threads REQUIRED)
find_package(ptask REQUIRED)

set(ptask_INCLUDE_DIRS "${ptask_DIR}/include")
message("Include dir ${ptask_INCLUDE_DIRS}")


add_executable(observatory src/main.c src/observatory.c src/gui.c)

include_directories ("${ptask_INCLUDE_DIRS}")
include_directories(${ALLEGRO_INCLUDE_DIR})

#add_subdirectory(src)
#add_subdirectory(project)

if(TRACE)
	target_link_libraries(observatory lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
	target_link_libraries(observatory ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()
