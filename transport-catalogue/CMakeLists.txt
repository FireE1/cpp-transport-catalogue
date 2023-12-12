cmake_minimum_required(VERSION 3.12)
 
project(final_project_15)
set(CMAKE_CXX_STANDARD 17)
 
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
 
add_subdirectory(proto)

set(UTILITY ./Utility/geo.h 
            ./Utility/geo.cpp 
            ./Utility/ranges.h)
 
set(TRANSPORT_CATALOGUE ./Transport_Catalogue/domain.h 
                        ./Transport_Catalogue/domain.cpp
                        ./Transport_Catalogue/transport_catalogue.h 
                        ./Transport_Catalogue/transport_catalogue.cpp)
                      
set(ROUTER ./Router/graph.h
           ./Router/router.h        
           ./Router/transport_router.h 
           ./Router/transport_router.cpp)
                              
set(JSON ./Json/json.h 
         ./Json/json.cpp 
         ./Json/json_builder.h 
         ./Json/json_builder.cpp 
         ./Json/json_reader.h 
         ./Json/json_reader.cpp)
               
set(SVG ./SVG/svg.h 
        ./SVG/svg.cpp)
        
set(MAP_RENDERER ./Map_renderer/map_renderer.h
                 ./Map_renderer/map_renderer.cpp)
              
set(SERIALIZATION ./Serialization/serialization.h 
                  ./Serialization/serialization.cpp)
                 
set(REQUEST_HANDLER ./Request_handler/request_handler.h 
                    ./Request_handler/request_handler.cpp)
 
add_executable(transport_catalogue main.cpp 
                                   ${UTILITY}
                                   ${TRANSPORT_CATALOGUE}
                                   ${ROUTER} 
                                   ${JSON}
                                   ${SVG} 
                                   ${MAP_RENDERER} 
                                   ${SERIALIZATION}
                                   ${REQUEST_HANDLER})
 
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(transport_catalogue proto "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
