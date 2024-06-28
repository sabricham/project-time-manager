idf_component_register(
    SRCS "button.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    PRIV_REQUIRES queue_handler task_handler)