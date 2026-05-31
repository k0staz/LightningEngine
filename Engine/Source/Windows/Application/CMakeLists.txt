file(GLOB_RECURSE APP_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
add_library(WindowsApplication STATIC ${APP_SOURCES})
target_include_directories(WindowsApplication
    PUBLIC Public
    PRIVATE Private)

target_link_libraries(WindowsApplication PUBLIC Core Log)