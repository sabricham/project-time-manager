idf_component_register(
    SRCS "encoder.c" "ky040.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    PRIV_REQUIRES queue_handler task_handler)