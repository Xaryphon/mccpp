include(CTest)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

function(mccpp_test target)
    add_executable(${target} ${ARGN})
    target_include_directories(${target} PRIVATE ../src)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED YES)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS NO)
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    catch_discover_tests(${target})
endfunction()

mccpp_test(test_proto_varint proto/varint.cc)
mccpp_test(test_client_extract_bits client/extract_bits.cc)
