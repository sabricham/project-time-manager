idf_component_register(
    SRCS "queueHandler.c" "taskHandler.c" "gpioHandler.c" "timerHandler.c"
    INCLUDE_DIRS "."
    REQUIRES driver)