cmake_minimum_required(VERSION 3.10)
project(MyLearningProjects C CXX)

add_subdirectory(thread)
add_subdirectory(process)
add_subdirectory(signal)
add_subdirectory(ipc)
add_subdirectory(mmap)
add_subdirectory(socket)
add_subdirectory(async_io)
add_subdirectory(master_http_server)
add_subdirectory(epoll_http_server)

add_subdirectory(netlink)

add_subdirectory(alsa)