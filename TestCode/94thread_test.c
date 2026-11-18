#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <gpiod.h>

#define LED_GPIO       17
#define SWITCH_GPIO    24

#define SLOW_INTERVAL  1.0
#define FAST_INTERVAL  0.2

// gcc -pthread -o 94thread_test 94thread_test.c -lgpiod
// ./94thread_test

/* スレッド間で共有する情報 */
typedef struct {
    pthread_mutex_t mutex;
    int stop_flag;
    double blink_interval;
} thread_data_t;

/* 関数プロトタイプ宣言 */
static int is_stopping(thread_data_t* data);

/* 指定した時間だけ待機する */
static void sleep_ms(unsigned int milliseconds)
{
    struct timespec ts;

    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000L;

    nanosleep(&ts, NULL);
}

/* GPIOの要求を行う */
static struct gpiod_line_request* request_gpio(
    struct gpiod_chip* chip,
    unsigned int offset,
    enum gpiod_line_direction direction)
{
    struct gpiod_line_settings* settings = NULL;
    struct gpiod_line_config* line_config = NULL;
    struct gpiod_line_request* request = NULL;

    settings = gpiod_line_settings_new();
    if (settings == NULL) {
        perror("gpiod_line_settings_new");
        goto cleanup;
    }

    if (gpiod_line_settings_set_direction(settings, direction) < 0) {
        perror("gpiod_line_settings_set_direction");
        goto cleanup;
    }

    if (direction == GPIOD_LINE_DIRECTION_OUTPUT) {
        if (gpiod_line_settings_set_output_value(
                settings, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_settings_set_output_value");
            goto cleanup;
        }
    } else {
        if (gpiod_line_settings_set_bias(
                settings, GPIOD_LINE_BIAS_PULL_DOWN) < 0) {
            perror("gpiod_line_settings_set_bias");
            goto cleanup;
        }
    }

    line_config = gpiod_line_config_new();
    if (line_config == NULL) {
        perror("gpiod_line_config_new");
        goto cleanup;
    }

    if (gpiod_line_config_add_line_settings(
            line_config, &offset, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }

    request = gpiod_chip_request_lines(chip, NULL, line_config);
    if (request == NULL) {
        perror("gpiod_chip_request_lines");
    }

cleanup:
    if (line_config != NULL) {
        gpiod_line_config_free(line_config);
    }

    if (settings != NULL) {
        gpiod_line_settings_free(settings);
    }

    return request;
}

/* LED制御用スレッド */
static void* led_thread(void* arg)
{
    thread_data_t* data = arg;
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;
    double interval;
    int stop;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL) {
        perror("gpiod_chip_open");
        return NULL;
    }

    request = request_gpio(
        chip, LED_GPIO, GPIOD_LINE_DIRECTION_OUTPUT);

    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    while (1) {
        /* 共有変数を取得する */
        pthread_mutex_lock(&data->mutex);
        stop = data->stop_flag;
        interval = data->blink_interval;
        pthread_mutex_unlock(&data->mutex);

        if (stop) {
            break;
        }

        /* LEDを点灯する */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("LED ON");
            break;
        }

        sleep_ms((unsigned int)(interval * 1000));

        /* LEDを消灯する */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("LED OFF");
            break;
        }

        sleep_ms((unsigned int)(interval * 1000));
    }

    /* 終了時にLEDを消灯する */
    gpiod_line_request_set_value(
        request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE);

    gpiod_line_request_release(request);
    gpiod_chip_close(chip);

    return NULL;
}

/* スイッチ入力用スレッド */
static void* switch_thread(void* arg)
{
    thread_data_t* data = arg;
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;
    enum gpiod_line_value value;
    double interval;
    int previous_pressed = 0;

    chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL) {
        perror("gpiod_chip_open");
        return NULL;
    }

    request = request_gpio(
        chip, SWITCH_GPIO, GPIOD_LINE_DIRECTION_INPUT);

    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    while (1) {
        if (is_stopping(data)) {
            break;
        }

        value = gpiod_line_request_get_value(request, SWITCH_GPIO);

        if (value < 0) {
            perror("gpiod_line_request_get_value");
            break;
        }

        /* Active High：HIGHになったらスイッチ押下 */
        if (value == GPIOD_LINE_VALUE_ACTIVE) {
            if (!previous_pressed) {
                /* チャタリング対策として少し待機する */
                sleep_ms(30);

                value = gpiod_line_request_get_value(
                    request, SWITCH_GPIO);

                if (value < 0) {
                    perror("gpiod_line_request_get_value");
                    break;
                }

                if (value == GPIOD_LINE_VALUE_ACTIVE) {
                    /* 共有変数を変更する */
                    pthread_mutex_lock(&data->mutex);

                    if (data->blink_interval == SLOW_INTERVAL) {
                        data->blink_interval = FAST_INTERVAL;
                    } else {
                        data->blink_interval = SLOW_INTERVAL;
                    }

                    interval = data->blink_interval;

                    pthread_mutex_unlock(&data->mutex);

                    printf("LED点滅間隔：%.1f秒\n", interval);

                    previous_pressed = 1;
                }
            }
        } else {
            previous_pressed = 0;
        }

        sleep_ms(50);
    }

    gpiod_line_request_release(request);
    gpiod_chip_close(chip);

    return NULL;
}

/* スレッドの終了状態を確認する */
static int is_stopping(thread_data_t* data)
{
    int stop;

    pthread_mutex_lock(&data->mutex);
    stop = data->stop_flag;
    pthread_mutex_unlock(&data->mutex);

    return stop;
}

int main(void)
{
    pthread_t led_id;
    pthread_t switch_id;
    thread_data_t data = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .stop_flag = 0,
        .blink_interval = SLOW_INTERVAL
    };
    int ret;

    /* LED制御用スレッドを作成する */
    ret = pthread_create(&led_id, NULL, led_thread, &data);
    if (ret != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));
        pthread_mutex_destroy(&data.mutex);
        return 1;
    }

    /* スイッチ入力用スレッドを作成する */
    ret = pthread_create(&switch_id, NULL, switch_thread, &data);
    if (ret != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));

        pthread_mutex_lock(&data.mutex);
        data.stop_flag = 1;
        pthread_mutex_unlock(&data.mutex);

        pthread_join(led_id, NULL);
        pthread_mutex_destroy(&data.mutex);
        return 1;
    }

    printf("30秒間、並行処理を実行します。\n");
    sleep(30);

    /* 各スレッドに終了を通知する */
    pthread_mutex_lock(&data.mutex);
    data.stop_flag = 1;
    pthread_mutex_unlock(&data.mutex);

    /* 各スレッドの終了を待つ */
    pthread_join(led_id, NULL);
    pthread_join(switch_id, NULL);

    pthread_mutex_destroy(&data.mutex);

    printf("プログラムを終了します。\n");

    return 0;
}