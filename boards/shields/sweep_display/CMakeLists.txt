if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
  zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  add_subdirectory(images)
  add_subdirectory(fonts)

  target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE custom_status_screen.c)
  target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE widgets/logo.c)
  target_sources_ifdef(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM app PRIVATE widgets/kbd_name.c)
endif()
