idf_component_register(
    SRCS "main.c" 
    INCLUDE_DIRS "." 
    PRIV_REQUIRES queue_handler task_handler display encoder manager led)