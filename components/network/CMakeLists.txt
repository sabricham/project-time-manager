idf_component_register(
    SRCS "network.c"
    INCLUDE_DIRS "."
    REQUIRES utility esp_hap_core esp_hap_apple_profiles esp_hap_extras button app_wifi app_hap_setup_payload)

    