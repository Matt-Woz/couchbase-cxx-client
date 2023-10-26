include(cmake/CPM.cmake)

# https://cmake.org/cmake/help/v3.28/policy/CMP0063.html
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)

if(NOT TARGET fmt::fmt)
  cpmaddpackage(
    NAME
    fmt
    GIT_TAG
    10.1.1
    VERSION
    10.1.1
    GITHUB_REPOSITORY
    "fmtlib/fmt"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON")
endif()

if(NOT TARGET spdlog::spdlog)
  cpmaddpackage(
    NAME
    spdlog
    VERSION
    1.12.0
    GITHUB_REPOSITORY
    "gabime/spdlog"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON"
    "SPDLOG_FMT_EXTERNAL ON")
endif()

if(NOT TARGET Microsoft.GSL::GSL)
  cpmaddpackage(
    NAME
    gsl
    VERSION
    4.0.0
    GITHUB_REPOSITORY
    "microsoft/gsl"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON")
endif()

if(NOT TARGET hdr_histogram::hdr_histogram_static)
  cpmaddpackage(
    NAME
    hdr_histogram
    GIT_TAG
    0.11.8
    VERSION
    0.11.8
    GITHUB_REPOSITORY
    "HdrHistogram/HdrHistogram_c"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON"
    "HDR_LOG_REQUIRED OFF"
    "HDR_HISTOGRAM_BUILD_SHARED OFF"
    "HDR_HISTOGRAM_BUILD_PROGRAMS OFF")
endif()
if(NOT MSVC)
  target_compile_options(hdr_histogram_static PRIVATE -Wno-unused-parameter)
endif()

if(NOT TARGET llhttp::llhttp)
  cpmaddpackage(
    NAME
    llhttp
    GIT_TAG
    release/v9.1.3
    VERSION
    9.1.3
    GITHUB_REPOSITORY
    "nodejs/llhttp"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_STATIC_LIBS ON")
endif()

if(NOT TARGET snappy)
  cpmaddpackage(
    NAME
    snappy
    GIT_TAG
    1.1.10
    VERSION
    1.1.10
    GITHUB_REPOSITORY
    "google/snappy"
    OPTIONS
    "CMAKE_C_VISIBILITY_PRESET hidden"
    "CMAKE_CXX_VISIBILITY_PRESET hidden"
    "CMAKE_POSITION_INDEPENDENT_CODE ON"
    "BUILD_SHARED_LIBS OFF"
    "SNAPPY_INSTALL OFF"
    "SNAPPY_BUILD_TESTS OFF"
    "SNAPPY_BUILD_BENCHMARKS OFF")
endif()
if(NOT MSVC)
  # https://github.com/google/snappy/pull/156
  target_compile_options(snappy PRIVATE -Wno-sign-compare)
endif()

if(NOT TARGET asio::asio)
  cpmaddpackage(
    NAME
    asio
    GIT_TAG
    asio-1-28-1
    VERSION
    1.28.1
    GITHUB_REPOSITORY
    "chriskohlhoff/asio")
endif()

# ASIO doesn't use CMake, we have to configure it manually. Extra notes for using on Windows:
#
# 1) If _WIN32_WINNT is not set, ASIO assumes _WIN32_WINNT=0x0501, i.e. Windows XP target, which is definitely not the
# platform which most users target.
#
# 2) WIN32_LEAN_AND_MEAN is defined to make Winsock2 work.
if(asio_ADDED)
  add_library(asio INTERFACE)

  target_include_directories(asio SYSTEM INTERFACE ${asio_SOURCE_DIR}/asio/include)

  target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)

  target_link_libraries(asio INTERFACE Threads::Threads)

  set_target_properties(asio PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

  if(WIN32)
    # macro see @ https://stackoverflow.com/a/40217291/1746503
    macro(get_win32_winnt version)
      if(CMAKE_SYSTEM_VERSION)
        set(ver ${CMAKE_SYSTEM_VERSION})
        string(
          REGEX MATCH
                "^([0-9]+).([0-9])"
                ver
                ${ver})
        string(
          REGEX MATCH
                "^([0-9]+)"
                verMajor
                ${ver})
        # Check for Windows 10, b/c we'll need to convert to hex 'A'.
        if("${verMajor}" MATCHES "10")
          set(verMajor "A")
          string(
            REGEX
            REPLACE "^([0-9]+)"
                    ${verMajor}
                    ver
                    ${ver})
        endif("${verMajor}" MATCHES "10")
        # Remove all remaining '.' characters.
        string(
          REPLACE "."
                  ""
                  ver
                  ${ver})
        # Prepend each digit with a zero.
        string(
          REGEX
          REPLACE "([0-9A-Z])"
                  "0\\1"
                  ver
                  ${ver})
        set(${version} "0x${ver}")
      endif()
    endmacro()

    if(NOT DEFINED _WIN32_WINNT)
      get_win32_winnt(ver)
      set(_WIN32_WINNT ${ver})
    endif()

    message(STATUS "Set _WIN32_WINNT=${_WIN32_WINNT}")

    target_compile_definitions(asio INTERFACE _WIN32_WINNT=${_WIN32_WINNT} WIN32_LEAN_AND_MEAN)
  endif()

  add_library(asio::asio ALIAS asio)
endif()

set(TAOCPP_JSON_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(TAOCPP_JSON_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(PEGTL_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(PEGTL_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
add_subdirectory(third_party/json)

include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/third_party/json/include)
include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/third_party/json/external/PEGTL/include)
include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/third_party/cxx_function)
include_directories(BEFORE SYSTEM ${PROJECT_SOURCE_DIR}/third_party/expected/include)

add_library(jsonsl OBJECT ${PROJECT_SOURCE_DIR}/third_party/jsonsl/jsonsl.c)
set_target_properties(jsonsl PROPERTIES C_VISIBILITY_PRESET hidden POSITION_INDEPENDENT_CODE TRUE)
target_include_directories(jsonsl PUBLIC SYSTEM ${PROJECT_SOURCE_DIR}/third_party/jsonsl)
