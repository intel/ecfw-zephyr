target_sources(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/flashhdr.c
    ${CMAKE_CURRENT_LIST_DIR}/softstrap.c
    ${CMAKE_CURRENT_LIST_DIR}/task_handler.c
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/flashhdr.h
    ${CMAKE_CURRENT_LIST_DIR}/softstrap.h
    ${CMAKE_CURRENT_LIST_DIR}/task_handler.h
    ${CMAKE_CURRENT_LIST_DIR}/memops.h
    )

target_include_directories(app
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    )
