find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ../inc)
include(GoogleTest)
include(CTest)

add_executable(runTests testGenetics.cpp testIndividual.cpp testInstruction.cpp testMath.cpp testMidi.cpp testOsc.cpp testPopulation.cpp testPerformance.cpp)
target_link_libraries(runTests ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
gtest_discover_tests(runTests)

