/* 4.1　赤 LED の点灯と消灯を0.5秒ごとに切り替える。 */
#define _POSIX_C_SOURCE 200809L
#include <gpiod.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static const unsigned int red_led = 17;

static volatile sig_atomic_t stop_requested;

static void request_stop(int signal_number)
{
    (void)signal_number;
    stop_requested = 1;
}

static int set_stop_handler(void)
{
    struct sigaction action = {0};
    action.sa_handler = request_stop;
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGINT, &action, NULL) < 0 || sigaction(SIGTERM, &action, NULL) < 0) {
        perror("sigaction");
        return -1;
    }
    return 0;
}

static struct gpiod_line_request *request_gpio(const char *path, bool active_low)
{
    struct gpiod_chip *chip = gpiod_chip_open(path);
    if (chip == NULL) {
        perror(path);
        return NULL;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    struct gpiod_line_config *config = gpiod_line_config_new();
    struct gpiod_request_config *options = gpiod_request_config_new();
    struct gpiod_line_request *request = NULL;

    if (settings == NULL || config == NULL || options == NULL) {
        perror("GPIO configuration");
        goto done;
    }

    /* GPIO17（物理ピン11）を、消灯状態から出力に使う。 */
    gpiod_line_settings_set_active_low(settings, active_low);
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
        gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE) < 0 ||
        gpiod_line_config_add_line_settings(config, &red_led, 1, settings) < 0) {
        perror("LED settings");
        goto done;
    }

    gpiod_request_config_set_consumer(options, "led_blink");
    request = gpiod_chip_request_lines(chip, options, config);
    if (request == NULL)
        perror("GPIO request");

done:
    gpiod_request_config_free(options);
    gpiod_line_config_free(config);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    return request;
}

int main(int argc, char **argv)
{
    if (argc != 3 || (strcmp(argv[2], "high") != 0 && strcmp(argv[2], "low") != 0)) {
        fprintf(stderr, "Usage: %s /dev/gpiochipN high|low\n", argv[0]);
        return 1;
    }
    if (set_stop_handler() < 0)
        return 1;

    struct gpiod_line_request *request = request_gpio(argv[1], strcmp(argv[2], "low") == 0);
    if (request == NULL)
        return 1;

    const struct timespec half_second = { .tv_nsec = 500000000L };
    enum gpiod_line_value value = GPIOD_LINE_VALUE_ACTIVE;
    int result = 0;

    while (!stop_requested) {
        if (gpiod_line_request_set_value(request, red_led, value) < 0) {
            perror("LED write");
            result = 1;
            break;
        }
        if (nanosleep(&half_second, NULL) < 0 && errno != EINTR) {
            perror("nanosleep");
            result = 1;
            break;
        }
        value = (value == GPIOD_LINE_VALUE_ACTIVE)
              ? GPIOD_LINE_VALUE_INACTIVE : GPIOD_LINE_VALUE_ACTIVE;
    }

    if (gpiod_line_request_set_value(request, red_led, GPIOD_LINE_VALUE_INACTIVE) < 0) {
        perror("LED off");
        result = 1;
    }
    gpiod_line_request_release(request);
    return result;
}
