add_executable(master_http_server 
    main.c
    thread_poll.c
    http.c
)

configure_file(index.html index.html COPYONLY)