cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)
project("demo" LANGUAGES CXX C)

set(TARGET demo)

set(DEMO_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

set(SRC_FILE ${DEMO_ROOT}/demo.cpp)
add_executable(demo ${SRC_FILE})

include(FetchContent)

FetchContent_Declare(
    json 
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

find_library(MATH_LIBRARY m)

add_custom_command(
    OUTPUT ${DEMO_ROOT}/qiskit/dist/c/qiskit.h
    COMMAND cd ../qiskit && make c || true && cd ../build
    COMMENT "Compiling qiskit..."
)

add_custom_command(
    OUTPUT ${DEMO_ROOT}/qrmi/target/release/libqrmi.a
    COMMAND cd ../qrmi && cargo build --release || true && cd ../build
    COMMENT "Compiling qrmi..."
)

add_custom_target(t_qiskit
    DEPENDS ${DEMO_ROOT}/qiskit/dist/c/qiskit.h
)

add_custom_target(t_qrmi
    DEPENDS ${DEMO_ROOT}/qrmi/target/release/libqrmi.a
)

add_dependencies(demo t_qiskit)
add_dependencies(demo t_qrmi)

set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")

target_link_libraries(demo
    PRIVATE
    "-L${DEMO_ROOT}/qiskit/dist/c/lib -L${DEMO_ROOT}/qrmi/target/release -Wl,-rpath ${DEMO_ROOT}/qiskit/dist/c/lib -Wl,-rpath ${DEMO_ROOT}/qrmi/target/release"
    ${MATH_LIBRARY}
    qiskit
    qrmi
    nlohmann_json::nlohmann_json
)


# path to headerfile
  target_include_directories(demo
    PRIVATE
      ${DEMO_ROOT}/qiskit/dist/c/include
      ${DEMO_ROOT}/qrmi
      ${DEMO_ROOT}
      ${DEMO_ROOT}/qcpp/src
      nlohmann_json::nlohmann_json
  )
