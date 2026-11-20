/* 4.4　Linux の周期タイマを使って赤 LED を点滅させる。 */
#define _POSIX_C_SOURCE 200809L
#include <gpiod.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

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

    gpiod_request_config_set_consumer(options, "timer_blink");
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

    int result = 1;
    int fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (fd < 0) {
        perror("timerfd_create");
        goto done;
    }
    /* 初回も2回目以降も0.5秒ごとに通知を受ける。 */
    const struct itimerspec period = {
        .it_value    = { .tv_nsec = 500000000L },
        .it_interval = { .tv_nsec = 500000000L }
    };
    if (timerfd_settime(fd, 0, &period, NULL) < 0) {
        perror("timerfd_settime");
        goto done;
    }

    enum gpiod_line_value value = GPIOD_LINE_VALUE_INACTIVE;
    while (!stop_requested) {
        uint64_t expirations;
        ssize_t n = read(fd, &expirations, sizeof expirations);
        if (n < 0 && errno == EINTR)
            continue;
        if (n != (ssize_t)sizeof expirations) {
            if (n >= 0)
                errno = EIO;
            perror("timerfd read");
            goto done;
        }
        /* 遅れて複数回分の通知を受けたら、奇数回分のときだけ反転する。 */
        if (expirations % 2 != 0) {
            value = (value == GPIOD_LINE_VALUE_ACTIVE)
                  ? GPIOD_LINE_VALUE_INACTIVE : GPIOD_LINE_VALUE_ACTIVE;
            if (gpiod_line_request_set_value(request, red_led, value) < 0) {
                perror("LED write");
                goto done;
            }
        }
    }
    result = 0;

done:
    if (fd >= 0)
        close(fd);
    if (gpiod_line_request_set_value(request, red_led, GPIOD_LINE_VALUE_INACTIVE) < 0) {
        perror("LED off");
        result = 1;
    }
    gpiod_line_request_release(request);
    return result;
}
