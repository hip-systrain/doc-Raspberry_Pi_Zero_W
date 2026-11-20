/* 4.3　白スイッチ SW1 を押した回数を UART で PC に送る。 */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <gpiod.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

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

    /* GPIO24（物理ピン18）は、離すと0、押すと1になる入力。 */
    if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT) < 0 ||
        gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_DOWN) < 0 ||
        gpiod_line_config_add_line_settings(config, &white_switch, 1, settings) < 0) {
        perror("switch settings");
        goto done;
    }

    gpiod_request_config_set_consumer(options, "uart_count");
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

static int parse_baud(const char *text, speed_t *speed)
{
    static const struct {
        const char *name;
        speed_t value;
    } baud_rates[] = {
        { "9600", B9600 }, { "19200", B19200 }, { "38400", B38400 },
        { "57600", B57600 }, { "115200", B115200 }
    };
    for (size_t i = 0; i < sizeof baud_rates / sizeof baud_rates[0]; ++i) {
        if (strcmp(text, baud_rates[i].name) == 0) {
            *speed = baud_rates[i].value;
            return 0;
        }
    }
    return -1;
}

static int open_uart(const char *path, speed_t speed, struct termios *original)
{
    int fd = open(path, O_RDWR | O_NOCTTY | O_CLOEXEC);
    if (fd < 0) {
        perror(path);
        return -1;
    }
    if (tcgetattr(fd, original) < 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }

    /* 8データビット、パリティなし、1ストップビット、フロー制御なし。 */
    struct termios settings = *original;
    cfmakeraw(&settings);
    settings.c_cflag &= ~(CSTOPB | CRTSCTS);
    settings.c_cflag |= CLOCAL | CREAD;
    settings.c_iflag &= ~(IXON | IXOFF | IXANY);
    if (cfsetispeed(&settings, speed) < 0 || cfsetospeed(&settings, speed) < 0 ||
        tcsetattr(fd, TCSANOW, &settings) < 0) {
        perror("UART settings");
        close(fd);
        return -1;
    }
    return fd;
}

static int send_count(int fd, uint64_t count)
{
    char line[32];  /* 64ビットの10進数、改行、終端文字が収まる。 */
    int length = snprintf(line, sizeof line, "%" PRIu64 "\r\n", count);
    if (length < 0 || (size_t)length >= sizeof line) {
        fprintf(stderr, "Cannot format UART count\n");
        return -1;
    }

    size_t sent = 0;
    while (sent < (size_t)length) {
        ssize_t n = write(fd, line + sent, (size_t)length - sent);
        if (n < 0 && errno == EINTR) {
            if (stop_requested)
                return 0;
            continue;
        }
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            perror("UART write");
            return -1;
        }
        sent += (size_t)n;
    }
    return 0;
}

int main(int argc, char **argv)
{
    speed_t speed;
    if (argc != 4 || parse_baud(argv[3], &speed) < 0) {
        fprintf(stderr, "Usage: %s /dev/gpiochipN /dev/serial0 "
                        "9600|19200|38400|57600|115200\n", argv[0]);
        return 1;
    }
    if (set_stop_handler() < 0)
        return 1;

    struct gpiod_line_request *request = request_gpio(argv[1]);
    if (request == NULL)
        return 1;

    struct termios original;
    int fd = open_uart(argv[2], speed, &original);
    if (fd < 0) {
        gpiod_line_request_release(request);
        return 1;
    }

    const struct timespec sample_interval = { .tv_nsec = 10000000L };  /* 10 ms */
    const unsigned int samples_to_confirm = 3;
    /* 起動時に押されていた場合は、一度離してから最初の押下を数える。 */
    enum gpiod_line_value stable = GPIOD_LINE_VALUE_ACTIVE;
    unsigned int changed_samples = 0;
    uint64_t count = 0;
    int result = 0;

    while (!stop_requested) {
        enum gpiod_line_value sample = gpiod_line_request_get_value(request, white_switch);
        if (sample == GPIOD_LINE_VALUE_ERROR) {
            perror("switch read");
            result = 1;
            break;
        }

        /* 同じ変化を3回続けて読んだら確定。押したときだけ1回送る。 */
        if (sample == stable) {
            changed_samples = 0;
        } else {
            ++changed_samples;
            if (changed_samples == samples_to_confirm) {
                stable = sample;
                changed_samples = 0;
                if (stable == GPIOD_LINE_VALUE_ACTIVE) {
                    ++count;
                    if (send_count(fd, count) < 0) {
                        result = 1;
                        break;
                    }
                }
            }
        }
        if (nanosleep(&sample_interval, NULL) < 0 && errno != EINTR) {
            perror("nanosleep");
            result = 1;
            break;
        }
    }

    if (tcsetattr(fd, TCSADRAIN, &original) < 0) {
        perror("restore UART settings");
        result = 1;
    }
    close(fd);
    gpiod_line_request_release(request);
    return result;
}
