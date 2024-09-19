/************************
 * MonitoringZimKnives Project
 * 
 * Using this to learn vscode/esp-idf
 * - started with empty example
 * - added the "hello world" code ... built, flashed, worked
 * - changed the SPI flash size to 4M to match the dev board
 * - Next: Create a freertos process
 * 
 ************************/

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "hal/gpio_types.h"

#include "htu21d.h"

#include "monitoring_zimknives.h"

#define STACK_SIZE 2048
TaskHandle_t xHandle_1 = NULL;
TaskHandle_t xHandle_2 = NULL;

void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

#ifdef RESTART_ENABLE
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
#endif


    /*
     * create sample task 1
     */
    xTaskCreate( sample_process_1, "sample_process_1", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle_1 );
    configASSERT( xHandle_1 ); /* check whether the returned handle is NULL */

    /*
     * create sample task 2
     */
    xTaskCreate( sample_process_2, "sample_process_2", STACK_SIZE, NULL, tskIDLE_PRIORITY, &xHandle_2 );
    configASSERT( xHandle_2 ); /* check whether the returned handle is NULL */

}

void sample_process_1(void *pvParameters)  {

    float temp, hum;
    int   reterr = HTU21D_ERR_OK;

    if((reterr = htu21d_init(I2C_NUM_0, 21, 22,  GPIO_PULLUP_ONLY,  GPIO_PULLUP_ONLY)) == HTU21D_ERR_OK)
        printf("HTU21D init OK\n");
    else
        printf("HTU21D init returned error code %d\n", reterr);

    while(1)  {
        printf("sample_process_1: executing on core %d\n", xPortGetCoreID());
        temp = ht21d_read_temperature();
        hum = ht21d_read_humidity();
        printf("Temp = %f   Humidity = %f\n", temp, hum);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void sample_process_2(void *pvParameters)  {
    while(1)  {
        printf("sample_process_2: executing on core %d\n", xPortGetCoreID());
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
