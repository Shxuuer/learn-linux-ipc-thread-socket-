add_executable(alsa_test main.c)

find_package(ALSA COMPONENTS asound REQUIRED)
target_link_libraries(alsa_test PRIVATE ${ALSA_LIBRARIES})