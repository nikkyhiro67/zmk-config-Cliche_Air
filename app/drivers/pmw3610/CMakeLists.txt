zephyr_library()
zephyr_library_sources_ifdef(CONFIG_ZMK_PMW3610 pmw3610.c)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
