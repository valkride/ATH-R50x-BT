# Main component CMakeLists.txt

idf_component_register(
    SRCS 
        "main.c"
        "bluetooth_manager.c"
        "audio_manager.c"
        "power_manager.c"
        "ui_manager.c"
    INCLUDE_DIRS 
        "."
    REQUIRES 
        driver
        esp_bt
        esp_bt_stack
        esp_a2dp
        esp_avrc
        esp_hf
        esp_i2s
        esp_adc
        esp_pm
        esp_sleep
        esp_timer
        nvs_flash
        freertos
)
