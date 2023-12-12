include(FindProtobuf)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto 
                                            svg.proto 
                                            map_renderer.proto 
                                            graph.proto 
                                            transport_router.proto)
add_library(proto ${PROTO_HDRS} ${PROTO_SRCS})
