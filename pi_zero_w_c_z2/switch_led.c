/* 4.2　白スイッチ SW1 を押している間だけ赤 LED を点灯する。 */
#define _POSIX_C_SOURCE 200809L
#include <gpiod.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

static const unsigned int red_led = 17;
static const unsigned int white_switch = 24;

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

static struct gpiod_line_request *request_gpio(const char *path)
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
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
        gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE) < 0 ||
        gpiod_line_config_add_line_settings(config, &red_led, 1, settings) < 0) {
        perror("LED settings");
        goto done;
    }

    /* GPIO24（物理ピン18）は、離すと0、押すと1になる入力。 */
    gpiod_line_settings_reset(settings);
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT) < 0 ||
        gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN) < 0 ||
        gpiod_line_config_add_line_settings(config, &white_switch, 1, settings) < 0) {
        perror("switch settings");
        goto done;
    }

    gpiod_request_config_set_consumer(options, "switch_led");
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
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/gpiochipN\n", argv[0]);
        return 1;
    }
    if (set_stop_handler() < 0)
        return 1;

    struct gpiod_line_request *request = request_gpio(argv[1]);
    if (request == NULL)
        return 1;

    const struct timespec sample_interval = { .tv_nsec = 10000000L };  /* 10 ms */
    enum gpiod_line_value led_value = GPIOD_LINE_VALUE_INACTIVE;
    int result = 0;

    while (!stop_requested) {
        enum gpiod_line_value pressed = gpiod_line_request_get_value(request, white_switch);
        if (pressed == GPIOD_LINE_VALUE_ERROR) {
            perror("switch read");
            result = 1;
            break;
        }
        /* スイッチの状態が変わったときだけ LED を書き換える。 */
        if (pressed != led_value) {
            if (gpiod_line_request_set_value(request, red_led, pressed) < 0) {
                perror("LED write");
                result = 1;
                break;
            }
            led_value = pressed;
        }
        if (nanosleep(&sample_interval, NULL) < 0 && errno != EINTR) {
            perror("nanosleep");
            result = 1;
            break;
        }
    }

    if (gpiod_line_request_set_value(request, red_led, GPIOD_LINE_VALUE_INACTIVE) < 0) {
        perror("LED off");
        result = 1;
    }
    gpiod_line_request_release(request);
    return result;
}
