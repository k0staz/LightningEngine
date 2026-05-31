cmake_minimum_required(VERSION 3.23)

project(LightningEngine)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(ProjectOptions INTERFACE)

target_compile_definitions(ProjectOptions INTERFACE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Debug>:TRACY_ENABLE>
    $<$<CONFIG:Release>:RELEASE>

    $<$<PLATFORM_ID:Windows>:PLATFORM_WINDOWS>
)

if(MSVC)
    target_compile_options(ProjectOptions INTERFACE 
        /W4 /permissive- 
        $<$<CONFIG:Debug>:/Od /Zi>        
        $<$<CONFIG:Release>:/O2 /Oi /Gy>
    )
    add_compile_options(/utf-8)
else() # GCC / Clang
    target_compile_options(ProjectOptions INTERFACE 
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Release>:-O3>
    )
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

function(generate_module_asset_reflection MODULE_NAME GEN_DIR ALL_MODULE_HEADERS OUT_MASTER_HEADER OUT_TARGET_FILES OUT_PUBLIC_DIR OUT_PRIVATE_DIR) 
    set(ASSET_TYPE_REGISTRATION_SCRIPT "${CMAKE_SOURCE_DIR}/Tools/GenerateAssetTypeRegistrationFiles.py")

    set(ASSET_GEN_DIR "${GEN_DIR}/AssetTypes")
    set(LOCAL_PUBLIC_DIR  "${ASSET_GEN_DIR}/Public")
    set(LOCAL_MASTER_HEADER  "${LOCAL_PUBLIC_DIR}/AssetMasterFile.gen.h")
    set(LOCAL_PRIVATE_DIR  "${ASSET_GEN_DIR}/Private")
    set(MASTER_CPP "${LOCAL_PRIVATE_DIR}/AssetMasterFile.gen.cpp")
    set(CONSOLIDATED_DEPFILE "${ASSET_GEN_DIR}/AssetMasterFile.d")

    set(${OUT_MASTER_HEADER} "${LOCAL_MASTER_HEADER}" PARENT_SCOPE)
    set(${OUT_PUBLIC_DIR} "${LOCAL_PUBLIC_DIR}" PARENT_SCOPE)
    set(${OUT_PRIVATE_DIR} "${LOCAL_PRIVATE_DIR}" PARENT_SCOPE)

    # Dry Run for predicting targets
    execute_process(
        COMMAND ${Python3_EXECUTABLE} "${ASSET_TYPE_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
                --dry-run
        OUTPUT_VARIABLE LOCAL_TARGET_FILES # Captures stdout into this variable
        RESULT_VARIABLE DRY_RUN_RESULT
    )

    set(${OUT_TARGET_FILES} "${LOCAL_TARGET_FILES}" PARENT_SCOPE)

    if(NOT DRY_RUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Reflection dry-run failed for module ${MODULE_NAME}!")
    endif()

    # Generates registration files and master files
    add_custom_command(
        OUTPUT "${MASTER_CPP}"
        COMMAND ${Python3_EXECUTABLE} "${ASSET_TYPE_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
        DEPFILE "${CONSOLIDATED_DEPFILE}"
        COMMENT "Running Asset reflection generation script for module: ${MODULE_NAME}"
        VERBATIM
        )
endfunction()

function(generate_module_ecs_reflection MODULE_NAME GEN_DIR ALL_MODULE_HEADERS OUT_MASTER_HEADER OUT_TARGET_FILES OUT_PUBLIC_DIR OUT_PRIVATE_DIR)
    set(ECS_REGISTRATION_SCRIPT "${CMAKE_SOURCE_DIR}/Tools/GenerateECSSystemRegistrationFiles.py")

    set(ECS_GEN_DIR "${GEN_DIR}/ECS")
    set(LOCAL_PUBLIC_DIR "${ECS_GEN_DIR}/Public")
    set(LOCAL_MASTER_HEADER "${LOCAL_PUBLIC_DIR}/ECSMasterFile.h")
    set(LOCAL_PRIVATE_DIR "${ECS_GEN_DIR}/Private")
    set(MASTER_CPP "${LOCAL_PRIVATE_DIR}/ECSMasterFile.gen.cpp")
    set(CONSOLIDATED_DEPFILE "${ECS_GEN_DIR}/ECSMasterFile.d")

    set(${OUT_MASTER_HEADER} "${LOCAL_MASTER_HEADER}" PARENT_SCOPE)
    set(${OUT_PUBLIC_DIR} "${LOCAL_PUBLIC_DIR}" PARENT_SCOPE)
    set(${OUT_PRIVATE_DIR} "${LOCAL_PRIVATE_DIR}" PARENT_SCOPE)

    # Dry Run for predicting targets
    execute_process(
        COMMAND ${Python3_EXECUTABLE} "${ECS_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
                --dry-run
        OUTPUT_VARIABLE LOCAL_TARGET_FILES # Captures stdout into this variable
        RESULT_VARIABLE DRY_RUN_RESULT
    )
    
    set(${OUT_TARGET_FILES} "${LOCAL_TARGET_FILES}" PARENT_SCOPE)

    if(NOT DRY_RUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Reflection dry-run failed for module ${MODULE_NAME}!")
    endif()

    # Generates registration files and master files
    add_custom_command(
        OUTPUT "${MASTER_CPP}"
        COMMAND ${Python3_EXECUTABLE} "${ECS_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
        DEPFILE "${CONSOLIDATED_DEPFILE}"
        COMMENT "Running ECS reflection generation script for module: ${MODULE_NAME}"
        VERBATIM
        )
endfunction()

function(generate_module_shader_reflection MODULE_NAME GEN_DIR ALL_MODULE_HEADERS OUT_MASTER_HEADER OUT_TARGET_FILES OUT_PUBLIC_DIR OUT_PRIVATE_DIR)
    set(SHADER_REGISTRATION_SCRIPT "${CMAKE_SOURCE_DIR}/Tools/GenerateShaderRegistrationFiles.py")

    set(SHADER_GEN_DIR "${GEN_DIR}/Shader")
    set(LOCAL_PUBLIC_DIR "${SHADER_GEN_DIR}/Public")
    set(LOCAL_MASTER_HEADER "${LOCAL_PUBLIC_DIR}/ShaderMasterRegistryFile.gen.h")
    set(LOCAL_PRIVATE_DIR "${SHADER_GEN_DIR}/Private")
    set(MASTER_CPP "${LOCAL_PRIVATE_DIR}/ShaderMasterRegistryFile.gen.cpp")
    set(CONSOLIDATED_DEPFILE "${SHADER_GEN_DIR}/ShaderMasterFile.d")
    
    set(${OUT_MASTER_HEADER} "${LOCAL_MASTER_HEADER}" PARENT_SCOPE)
    set(${OUT_PUBLIC_DIR} "${LOCAL_PUBLIC_DIR}" PARENT_SCOPE)
    set(${OUT_PRIVATE_DIR} "${LOCAL_PRIVATE_DIR}" PARENT_SCOPE)

    # Dry Run for predicting targets
    execute_process(
        COMMAND ${Python3_EXECUTABLE} "${SHADER_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
                --dry-run
        OUTPUT_VARIABLE LOCAL_TARGET_FILES # Captures stdout into this variable
        RESULT_VARIABLE DRY_RUN_RESULT
    )

    set(${OUT_TARGET_FILES} "${LOCAL_TARGET_FILES}" PARENT_SCOPE)

    if(NOT DRY_RUN_RESULT EQUAL 0)
        message(FATAL_ERROR "Reflection dry-run failed for module ${MODULE_NAME}!")
    endif()

    # Generates registration files and master files
    add_custom_command(
        OUTPUT "${MASTER_CPP}"
        COMMAND ${Python3_EXECUTABLE} "${SHADER_REGISTRATION_SCRIPT}"
                --header-files ${ALL_MODULE_HEADERS}
                --gen-folder "${LOCAL_PRIVATE_DIR}"
                --master-header "${LOCAL_MASTER_HEADER}"
                --master-cpp "${MASTER_CPP}"
                --module-name "${MODULE_NAME}"
                --depfile "${CONSOLIDATED_DEPFILE}"
        DEPFILE "${CONSOLIDATED_DEPFILE}"
        COMMENT "Running Shader reflection generation script for module: ${MODULE_NAME}"
        VERBATIM
        )
endfunction()

function(generate_module_reflection MODULE_NAME OUT_TARGET_FILES OUT_PUBLIC OUT_PRIVATE)
    set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/${MODULE_NAME}")
    # Track adding or removing headers
    file(GLOB_RECURSE ALL_MODULE_HEADERS CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/../*.h"
        )

    generate_module_asset_reflection("${MODULE_NAME}" "${GEN_DIR}" "${ALL_MODULE_HEADERS}" ASSET_MASTER_FILE ASSET_TARGET_FILES ASSET_PUBLIC_DIR ASSET_PRIVATE_DIR)
    generate_module_ecs_reflection("${MODULE_NAME}" "${GEN_DIR}" "${ALL_MODULE_HEADERS}" ECS_MASTER_FILE ECS_TARGET_FILES ECS_PUBLIC_DIR ECS_PRIVATE_DIR)
    generate_module_shader_reflection("${MODULE_NAME}" "${GEN_DIR}" "${ALL_MODULE_HEADERS}" SHADER_MASTER_FILE SHADER_TARGET_FILES SHADER_PUBLIC_DIR SHADER_PRIVATE_DIR)

    set(LOCAL_TARGET_FILES "${ASSET_TARGET_FILES}" "${ECS_TARGET_FILES}" "${SHADER_TARGET_FILES}")
    set(LOCAL_PUBLIC "${ASSET_PUBLIC_DIR}" "${ECS_PUBLIC_DIR}" "${SHADER_PUBLIC_DIR}")
    set(LOCAL_PRIVATE "${ASSET_PRIVATE_DIR}" "${ECS_PRIVATE_DIR}" "${SHADER_PRIVATE_DIR}")

    set_source_files_properties(
    ${LOCAL_TARGET_FILES}
    PROPERTIES GENERATED TRUE
    )

    set(${OUT_TARGET_FILES} "${LOCAL_TARGET_FILES}" PARENT_SCOPE)
    set(${OUT_PUBLIC} "${LOCAL_PUBLIC}" PARENT_SCOPE)
    set(${OUT_PRIVATE} "${LOCAL_PRIVATE}" PARENT_SCOPE)
endfunction()

macro(add_library targetName)
    _add_library(${targetName} ${ARGN})
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        if(TARGET ProjectOptions AND NOT "${targetName}" STREQUAL "ProjectOptions")
            target_link_libraries("${targetName}" PRIVATE ProjectOptions)
        endif()
    endif()
endmacro()

macro(add_executable targetName)
    _add_executable(${targetName} ${ARGN})
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        if(TARGET ProjectOptions)
            target_link_libraries("${targetName}" PRIVATE ProjectOptions)
        endif()
    endif()
endmacro()

add_subdirectory(Engine/3rdParty/spdlog)
add_subdirectory(Engine/3rdParty/tracy)

add_subdirectory(Engine/Source/ClientBridge)
add_subdirectory(Engine/Source/EngineBridge)
add_subdirectory(Engine/Source/Log)
add_subdirectory(Engine/Source/Core)
add_subdirectory(Engine/Source/System)
add_subdirectory(Engine/Source/RHI)
add_subdirectory(Engine/Source/Windows/Application)
add_subdirectory(Engine/Source/Renderer)
add_subdirectory(Engine/Source/Windows/D3D11RHI)
add_subdirectory(Engine/Source/CoreECS)
add_subdirectory(Engine/Source/Tools)
add_subdirectory(Engine/Source/Client)
add_subdirectory(Engine/Source/AutoRegistration)
add_subdirectory(Engine/Source/Engine)
add_subdirectory(Application)

