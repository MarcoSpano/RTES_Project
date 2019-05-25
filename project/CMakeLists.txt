# Add include directory
include_directories(.)
include_directories(../src/)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

find_package(Allegro REQUIRED)

include_directories(${ALLEGRO_INCLUDE_DIR})

add_executable(prova ptask_prova.c)
add_executable(osservatorio osservatorio.c)

if(TRACE)
  target_link_libraries(prova lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
  target_link_libraries(osservatorio lttng ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt dl lttng-ust)
else()
  target_link_libraries(prova ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
  target_link_libraries(osservatorio ptask Threads::Threads ${ALLEGRO_LIBRARY} m rt)
endif()
