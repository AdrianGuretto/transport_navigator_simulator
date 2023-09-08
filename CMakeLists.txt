cmake_minimum_required(VERSION 3.11)

project(TransportRouter CXX)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

set(JSON_LIB "src/headers/json_builder.h" "src/headers/json.h" "src/headers/json_reader.h"
             "src/json_builder.cpp" "src/json.cpp" "src/json_reader.cpp")

set(MAP_RENDER_LIB "src/headers/svg.h" "src/headers/map_renderer.h"
                    "src/svg.cpp" "src/map_renderer.cpp")

set(TRANSPORT_CATALOGUE_FILES "src/headers/transport_catalogue.h" "src/headers/transport_router.h" "src/headers/router.h" "src/headers/request_handler.h"
                                "src/transport_catalogue.cpp" "src/transport_router" "src/request_handler.cpp")

set(SUPPORT_FILES "src/headers/domain.h" "src/headers/geo.h" "src/headers/graph.h" "src/headers/router.h"
                    "src/domain.cpp" "src/geo.cpp")


add_executable(TransportCatalogue main.cpp ${SYSTEM_LIBS} ${JSON_LIB} ${MAP_RENDER_LIB} ${TRANSPORT_CATALOGUE_FILES} ${SUPPORT_FILES})