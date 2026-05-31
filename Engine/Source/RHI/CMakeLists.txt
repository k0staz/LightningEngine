file(GLOB_RECURSE RHI_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(RHI STATIC ${RHI_SOURCES})
target_include_directories(RHI
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(RHI PUBLIC Log Core)