file(GLOB_RECURSE TOOLS_SOURCES "Public/*.h" "Private/*cpp" "Private/*h" "../../3rdParty/OpenFBX/src/*")
add_library(Tools STATIC ${TOOLS_SOURCES})
target_include_directories(Tools
    PUBLIC Public
    PRIVATE Private "../../3rdParty/OpenFBX/src")

target_link_libraries(Tools PUBLIC Core CoreECS)