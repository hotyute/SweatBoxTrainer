cmake_minimum_required(VERSION 3.23)

project(SweatBoxTrainer VERSION 1.0.0 LANGUAGES CXX RC)

set(SBT_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SweatBoxTrainer")

set(SBT_HEADERS
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/SweatBoxTrainer.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/Resource.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/airport.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/basic_stream.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/calc_cycles.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/clinc2.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/config.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/constants.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/filereader.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/framework.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/geoutils.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/globals.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/guicon.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/guidialogue.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/packets_in.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/packets_out.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/point2d.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/save.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/targetver.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/tools.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/usermanager.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/AircraftPingTask.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/AircraftState.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/FlightModel.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/RouteManager.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/aircraft.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/aircraft/command_handler.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/nav/navaids.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/sim/simulation_context.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/tools/thread_pool.h"
    "${SBT_PROJECT_DIR}/include/SweatBoxTrainer/tools/timed_task.h"
)

set(SBT_SOURCES
    "${SBT_PROJECT_DIR}/src/SweatBoxTrainer.cpp"
    "${SBT_PROJECT_DIR}/src/airport.cpp"
    "${SBT_PROJECT_DIR}/src/basic_stream.cpp"
    "${SBT_PROJECT_DIR}/src/calc_cycles.cpp"
    "${SBT_PROJECT_DIR}/src/clinc2.cpp"
    "${SBT_PROJECT_DIR}/src/config.cpp"
    "${SBT_PROJECT_DIR}/src/filereader.cpp"
    "${SBT_PROJECT_DIR}/src/geoutils.cpp"
    "${SBT_PROJECT_DIR}/src/globals.cpp"
    "${SBT_PROJECT_DIR}/src/guicon.cpp"
    "${SBT_PROJECT_DIR}/src/guidialogue.cpp"
    "${SBT_PROJECT_DIR}/src/packets_in.cpp"
    "${SBT_PROJECT_DIR}/src/packets_out.cpp"
    "${SBT_PROJECT_DIR}/src/point2d.cpp"
    "${SBT_PROJECT_DIR}/src/save.cpp"
    "${SBT_PROJECT_DIR}/src/tools.cpp"
    "${SBT_PROJECT_DIR}/src/usermanager.cpp"
    "${SBT_PROJECT_DIR}/src/aircraft/AircraftPingTask.cpp"
    "${SBT_PROJECT_DIR}/src/aircraft/FlightModel.cpp"
    "${SBT_PROJECT_DIR}/src/aircraft/RouteManager.cpp"
    "${SBT_PROJECT_DIR}/src/aircraft/aircraft.cpp"
    "${SBT_PROJECT_DIR}/src/aircraft/command_handler.cpp"
    "${SBT_PROJECT_DIR}/src/sim/simulation_context.cpp"
    "${SBT_PROJECT_DIR}/src/tools/thread_pool.cpp"
    "${SBT_PROJECT_DIR}/src/tools/timed_task.cpp"
)

set(SBT_RESOURCES
    "${SBT_PROJECT_DIR}/resources/SweatBoxTrainer.rc"
)

add_executable(SweatBoxTrainer WIN32
    ${SBT_HEADERS}
    ${SBT_SOURCES}
    ${SBT_RESOURCES}
)

target_compile_features(SweatBoxTrainer PRIVATE cxx_std_20)

target_include_directories(SweatBoxTrainer PRIVATE
    "${SBT_PROJECT_DIR}/include"
    "${SBT_PROJECT_DIR}/resources"
)

target_compile_definitions(SweatBoxTrainer PRIVATE
    NOMINMAX
    UNICODE
    _UNICODE
    _WINDOWS
)

if(MSVC)
    target_compile_options(SweatBoxTrainer PRIVATE /EHsc)
endif()

target_link_libraries(SweatBoxTrainer PRIVATE
    ws2_32
    winmm
)

source_group(TREE "${SBT_PROJECT_DIR}/include" PREFIX "include" FILES ${SBT_HEADERS})
source_group(TREE "${SBT_PROJECT_DIR}/src" PREFIX "src" FILES ${SBT_SOURCES})
source_group(TREE "${SBT_PROJECT_DIR}/resources" PREFIX "resources" FILES ${SBT_RESOURCES})

install(TARGETS SweatBoxTrainer RUNTIME DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data/" DESTINATION data OPTIONAL)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packets.txt" DESTINATION . OPTIONAL)

set(CPACK_PACKAGE_NAME "SweatBoxTrainer")
set(CPACK_PACKAGE_VENDOR "SweatBoxTrainer")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "ZIP")

include(CPack)
