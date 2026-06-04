file(GLOB_RECURSE APPLICATION_SOURCES "Source/*cpp")

add_executable(Application WIN32  ${APPLICATION_SOURCES})

target_include_directories(Application
    PRIVATE Source
)

target_link_libraries(Application PRIVATE Engine)

set_target_properties(Application PROPERTIES
        VS_COPY_TO_OUT_DIR ON
)