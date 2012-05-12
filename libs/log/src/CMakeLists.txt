add_library(boost_log
  attribute_set.cpp
  attribute_values_view.cpp
  slim_string.cpp
  core.cpp
  stream_provider.cpp
  severity_level.cpp
  exceptions.cpp
  sink_frontends.cpp
  text_ostream_backend.cpp
  text_file_backend.cpp
)
set_target_properties(boost_log PROPERTIES
  COMPILE_DEFINITIONS BOOST_LOG_BUILDING_THE_LIB=1
)

# Ensure C++ ABI compatibility with Boost libraries compiled in C++98 mode,
# which is currently the default for C++ compilers supporting C++11.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(boost_log PROPERTIES COMPILE_FLAGS "-std=c++98 -Wno-long-long")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_target_properties(boost_log PROPERTIES COMPILE_FLAGS "-std=c++98 -Wno-long-long")
endif()
