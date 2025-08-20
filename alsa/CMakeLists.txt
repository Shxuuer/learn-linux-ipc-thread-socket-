add_executable(alsa_loop main.c)

find_package(ALSA COMPONENTS asound REQUIRED)
target_link_libraries(alsa_loop PRIVATE ${ALSA_LIBRARIES})