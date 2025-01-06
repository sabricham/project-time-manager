idf_component_register(
    SRCS "led.c"
    INCLUDE_DIRS "."
    REQUIRES led_strip
    PRIV_REQUIRES queue_handler task_handler)