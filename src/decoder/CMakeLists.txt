add_executable(decoder decoder.cpp SlantHierarchyDecoder.cpp PartitionDecoder.cpp Hierarchical4DDecoder.cpp ../View.cpp ../ProbabilityModel.cpp ../MultiscaleTransform.cpp ../Matrix.cpp ../LightField.cpp ../IntegerString.cpp ../Block4D.cpp ../ABADecoder.cpp ../ABACoder.cpp)
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/decoder)

install(TARGETS decoder RUNTIME DESTINATION bin)