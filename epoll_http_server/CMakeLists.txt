add_executable(epoll_http_server 
    main.c
    http.c
)

configure_file(index.html index.html COPYONLY)