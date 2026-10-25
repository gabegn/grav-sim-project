add_library(common STATIC)
target_sources(common PRIVATE
    src/progargs.cpp
    src/particle.cpp
    src/config_io.cpp
    include/point.hpp
    include/enclosure.hpp
    include/particle.hpp
    include/physics.hpp
    include/config_io.hpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
