file(GLOB_RECURSE AUTO_REGISTRATION_SOURCES "Public/*.h" "Private/*cpp" "Private/*h")
generate_module_reflection("EngineModule" OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)

add_library(AutoRegistration STATIC ${AUTO_REGISTRATION_SOURCES} "${OUT_TARGET_FILES}")
target_include_directories(AutoRegistration
    PUBLIC Public "${OUT_PUBLIC}"
    PRIVATE Private "${OUT_PRIVATE}")

target_link_libraries(AutoRegistration PUBLIC Log Core RHI CoreECS Renderer)