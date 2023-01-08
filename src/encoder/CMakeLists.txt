add_executable(encoder encoder.cpp TransformPartition.cpp SlantHierarchy.cpp Hierarchical4DEncoder.cpp ../View.cpp ../ProbabilityModel.cpp ../MultiscaleTransform.cpp ../Matrix.cpp ../LightField.cpp ../IntegerString.cpp ../Block4D.cpp ../ABACoder.cpp)
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/encoder) # include files from ProjectB

install(TARGETS encoder RUNTIME DESTINATION bin)