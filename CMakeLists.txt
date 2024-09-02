cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Set project name
set(PROJECT_NAME "conflicts")
set(${PROJECT_NAME}_DEPENDENCIES "")
set(${PROJECT_NAME}_INTERFACES "requirements")

# Option for building tests
option(${PROJECT_NAME}_BUILD_TESTS "Build tests" OFF)
if(${PROJECT_NAME}_BUILD_TESTS)
	list(APPEND VCPKG_MANIFEST_FEATURES "tests")
	enable_testing()
endif()

project(${PROJECT_NAME})

add_subdirectory(src)

if(${PROJECT_NAME}_BUILD_TESTS)
	add_subdirectory(tests)
endif()
