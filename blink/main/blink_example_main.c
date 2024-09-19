/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#include "esp_timer.h"
#include "driver/gptimer.h"


static const char *TAG = "example";

/***** 
 * DJZ - Hacking in some stuff from the gpio example to make
 * a squarewave
 *****/
#define GPIO_OUTPUT_IO_0    18
#define GPIO_OUTPUT_IO_1    19
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
/*
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * Let's say, GPIO_OUTPUT_IO_0=18, GPIO_OUTPUT_IO_1=19
 * In binary representation,
 * 1ULL<<GPIO_OUTPUT_IO_0 is equal to 0000000000000000000001000000000000000000 and
 * 1ULL<<GPIO_OUTPUT_IO_1 is equal to 0000000000000000000010000000000000000000
 * GPIO_OUTPUT_PIN_SEL                0000000000000000000011000000000000000000
 * */


/*
 * Let's experiment with some square wave generation
 * How fast can it go and still let the IDLE() tash run ?
 */
#define SQ_WAVE_INTERVAL (int64_t)1  // square wave interval
#define L_DELAY_INTERVAL (int64_t)10000  // experiment to get rid of watchdog ... how often to block
#define L_DELAY_DURATION (int64_t)10 // duration of block in mS

/*
 * Configuration Editor -> ESP System Settings 160 MHz, 100 Hz tic (default)
 * Exp #1: All out bit banging
 * a) No delay for IDLE() : 1.429 MHz w/ constant resets
 * 
 * Exp #2: Tic based, 1 uS interval
 * a)  w/ no delay for IDLE() : 220-270 KHz w/ constant resets
 * b) burst mode: 10 mS delay every 10 mS : generates burst every 10 mS and "Do Nothing" gets to run
 * c) burst mode: 1 mS delay every 10 mS : generates burst and constant resets (i.e. not enough time for IDLE())
 *    (makes sense since the tic interval is 10 mS)
 * 
 * Exp #3: Interrupt driven ... can interrupt IDLE()?
 */
#define SQUAREWAVE_TIC    1
#define SQUAREWAVE_BASIC  2
#define SQUAREWAVE_INTR   3

#define SQUAREWAVE_METHOD SQUAREWAVE_INTR
//#define WD_DELAY


static void square_wave(void)
{
    int64_t cur_us = 0;
    int64_t next_us = 0;
    uint8_t cur_level = 0;

    uint64_t cur_delay_us = 0;

    while(1)
    {
        /*
         * NOTE: even the responsible code still locks out the
         * IDLE() (i.e. stays in the while()) task and so generates the watchdog timer
         * ... no matter how slow I make the square wave
         */
#if SQUAREWAVE_METHOD == SQUAREWAVE_TIC
        if(((next_us = esp_timer_get_time()) - cur_us) > SQ_WAVE_INTERVAL)
        {
            gpio_set_level(GPIO_OUTPUT_IO_0, (cur_level = !cur_level));


            /****
            if(cur_level == 0)
            {
                gpio_set_level(GPIO_OUTPUT_IO_0, 1);
                cur_level = 1;
            }
            else
            {
                gpio_set_level(GPIO_OUTPUT_IO_0, 0);
                cur_level = 0;
            }
            ****/

            cur_us = next_us;
        }
#elif SQUAREWAVE_METHOD == SQUAREWAVE_BASIC
        gpio_set_level(GPIO_OUTPUT_IO_0, 1);
        gpio_set_level(GPIO_OUTPUT_IO_0, 0);
#endif

/*
 * play with trying to give IDLE() time to run
 */
#ifdef WD_DELAY
        /* trying to experiment to see if I can get rid of the watchdog trip */
        if((next_us - cur_delay_us) > L_DELAY_INTERVAL)
        {
            vTaskDelay(L_DELAY_DURATION / portTICK_PERIOD_MS); // needs to be in mS
            cur_delay_us = next_us;
        }
#else
        vTaskDelay(10 / portTICK_PERIOD_MS);
#endif // WD_DELAY

    } // while()
}


/*
 * square wave using an interrupt
 */
void square_wave_proc(void)
{
    static int cur_level = 0;

    gpio_set_level(GPIO_OUTPUT_IO_0, (cur_level = !cur_level));
}
static void square_wave_timer_init(void)
{
    ESP_LOGI(TAG, "Create timer handle");
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz, 1 tick=1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
        .on_alarm = square_wave_proc,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    ESP_LOGI(TAG, "Enable timer");
    ESP_ERROR_CHECK(gptimer_enable(gptimer));

    ESP_LOGI(TAG, "Start timer, stop it at alarm event");
    gptimer_alarm_config_t alarm_config1 = {
        .reload_count = 0,
        .alarm_count = 50, // period = 100 uS
        .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config1));
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

/* END OF SQUARE WAVE STUFF - DJZ */

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

// move one led along the strip and back
int8_t cur_led = -1;  // the led that is currently lit
uint8_t led_dir = 1;  // 1 = fwd, 0 = rev
uint8_t r = 16, g = 0, b = 0;
#define NUM_LEDS 20  // temporary hack TODO
static void ping_led(void)
{
    if(led_dir == 1)
    {
        cur_led++;
        if(cur_led >= NUM_LEDS)
        {
            led_dir = 0;
            cur_led--;
        }
    }
    else
    {
        cur_led--;
        if (cur_led < 0)
        {
            led_dir = 1;
            cur_led++;
        }
        
    }

    led_strip_clear(led_strip);
    led_strip_set_pixel(led_strip, cur_led, r, g, b);
    led_strip_refresh(led_strip);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 20, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO

static void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
    gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

#else
#error "unsupported LED type"
#endif

static void neopixel_example(void)
{
    /* Configure the peripheral according to the LED type */
    configure_led();

    while(1)
    {
        ping_led();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    /* DJZ - adding some stuff to make a square wave*/
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;  // enabled pull up  - DJZ
    //configure GPIO with the given settings
    gpio_config(&io_conf);
     //start gpio task
    /* end of square wave stuff*/

    xTaskCreate(neopixel_example, "neopixel_example", 4096, NULL, 10, NULL);

#if SQUAREWAVE_METHOD == SQUAREWAVE_INTR
    square_wave_timer_init();
#else
    xTaskCreate(square_wave, "square_wave", 2048, NULL, 10, NULL);
#endif

    while (1) {
#ifdef OG_EXAMPLE
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        blink_led();
        /* Toggle the LED state */
        s_led_state = !s_led_state;
#else
        ESP_LOGI(TAG, "Doing nothing ... \n");
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
