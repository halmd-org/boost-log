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
