set(${PROJECT_NAME}_LINK_PUBLIC_LIBS "")
set(${PROJECT_NAME}_LINK_INTERFACE_LIBS "")

foreach(_DEP_LIB ${${PROJECT_NAME}_DEPENDENCIES})
    find_package(${_DEP_LIB} CONFIG REQUIRED)
    list(APPEND ${PROJECT_NAME}_LINK_PUBLIC_LIBS ${_DEP_LIB}::${_DEP_LIB})
endforeach()

foreach(_DEP_LIB ${${PROJECT_NAME}_INTERFACES})
    find_package(${_DEP_LIB} CONFIG REQUIRED)
    list(APPEND ${PROJECT_NAME}_LINK_INTERFACE_LIBS ${_DEP_LIB}::${_DEP_LIB})
endforeach()

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LINK_PUBLIC_LIBS})
target_link_libraries(${PROJECT_NAME} INTERFACE ${${PROJECT_NAME}_LINK_INTERFACE_LIBS})

target_include_directories(${PROJECT_NAME} 
    INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "include/${PROJECT_NAME}.hpp")

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_targets
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Generate and install *-targets.cmake
install(EXPORT ${PROJECT_NAME}_targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION share/${PROJECT_NAME}
)

# Generate the config file
if("${${PROJECT_NAME}_DEPENDENCIES}" STREQUAL "" AND "${PROJECT_NAME}_INTERFACES" STREQUAL "")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")\n"
    )
else()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "include(CMakeFindDependencyMacro)\n"
    )
    foreach(_DEPENDENCY ${${PROJECT_NAME}_DEPENDENCIES})
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            "find_dependency(${_DEPENDENCY} CONFIG REQUIRED)\n"
        )
    endforeach()
    foreach(_DEPENDENCY ${${PROJECT_NAME}_INTERFACES})
        file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
            "find_dependency(${_DEPENDENCY} CONFIG REQUIRED)\n"
        )
    endforeach()
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        "include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}-targets.cmake\")\n"
    )
endif()

# Install the generated config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
    DESTINATION share/${PROJECT_NAME}
)
