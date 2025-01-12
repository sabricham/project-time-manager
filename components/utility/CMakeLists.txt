idf_component_register(
    SRCS "gpioHandler.c" "timerHandler.c" "queueHandler" "taskHandler"
    INCLUDE_DIRS "."
    REQUIRES driver)