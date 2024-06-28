idf_component_register(
    SRCS "sh1106_i2c.c" "sh1106.c" "display.c" "bitmaps.c"
    INCLUDE_DIRS "."
    REQUIRES driver
    PRIV_REQUIRES queue_handler task_handler)