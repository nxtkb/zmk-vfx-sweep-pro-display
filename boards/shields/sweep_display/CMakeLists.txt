if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM) 
  zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_BUILT_IN app PRIVATE custom_status_screen.c)

  add_subdirectory_ifdef(CONFIG_ZMK_DISPLAY widgets/)
  add_subdirectory_ifdef(CONFIG_ZMK_DISPLAY fonts/)
  add_subdirectory_ifdef(CONFIG_ZMK_DISPLAY images/)

endif()