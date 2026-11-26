#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <gpiod.h>

/* GPIO設定 */
#define LED_GPIO       17
#define SWITCH_GPIO    24

/* LEDの点滅間隔 */
#define SLOW_INTERVAL  1.0
#define FAST_INTERVAL  0.2

/* スレッド間で共有する情報 */
typedef struct {
    pthread_mutex_t mutex;  /* 共有変数への同時アクセスを防ぐ */
    int stop_flag;          /* スレッドの終了要求：0=継続、1=終了 */
    double blink_interval;  /* LEDの点滅間隔（秒） */
} thread_data_t;


/* 関数プロトタイプ宣言 */
static int is_stopping(thread_data_t* data);

/*
 * 関数名：sleep_ms
 * 概要  ：指定された時間だけ処理を待機させる。
 * 引数  ：milliseconds - 待機時間（ミリ秒）
 * 戻り値：なし
 */
static void sleep_ms(unsigned int milliseconds)
{
    struct timespec ts;

    /* ミリ秒を秒とナノ秒に変換する */
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000L;

    /* 指定した時間だけ処理を待機させる */
    nanosleep(&ts, NULL);
}

/*
 * 関数名：request_gpio
 * 概要  ：指定されたGPIOを入力または出力として要求する。
 * 引数  ：chip      - GPIOチップ
 *         offset    - GPIO番号
 *         direction - 入力または出力の方向
 * 戻り値：GPIO要求情報へのポインタ
 *         失敗した場合はNULL
 */
static struct gpiod_line_request* request_gpio(
    struct gpiod_chip* chip,
    unsigned int offset,
    enum gpiod_line_direction direction)
{
    struct gpiod_line_settings* settings = NULL;
    struct gpiod_line_config* line_config = NULL;
    struct gpiod_line_request* request = NULL;

    /* GPIOの設定を作成する */
    settings = gpiod_line_settings_new();
    if (settings == NULL) {
        perror("gpiod_line_settings_new");
        goto cleanup;
    }

    /* GPIOの入出力方向を設定する */
    if (gpiod_line_settings_set_direction(settings, direction) < 0) {
        perror("gpiod_line_settings_set_direction");
        goto cleanup;
    }

    if (direction == GPIOD_LINE_DIRECTION_OUTPUT) {
        /* 出力の場合は初期状態をLOWにする */
        if (gpiod_line_settings_set_output_value(
                settings, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_settings_set_output_value");
            goto cleanup;
        }
    } else {
        /* 入力の場合はプルダウンを設定する */
        if (gpiod_line_settings_set_bias(
                settings, GPIOD_LINE_BIAS_PULL_DOWN) < 0) {
            perror("gpiod_line_settings_set_bias");
            goto cleanup;
        }
    }

    /* GPIOの設定をまとめるための設定オブジェクトを作成する */
    line_config = gpiod_line_config_new();
    if (line_config == NULL) {
        perror("gpiod_line_config_new");
        goto cleanup;
    }

    /* 指定したGPIO番号に設定を適用する */
    if (gpiod_line_config_add_line_settings(
            line_config, &offset, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }

    /* GPIOの使用を要求する */
    request = gpiod_chip_request_lines(chip, NULL, line_config);
    if (request == NULL) {
        perror("gpiod_chip_request_lines");
    }

cleanup:
    /* 設定オブジェクトを解放する */
    if (line_config != NULL) {
        gpiod_line_config_free(line_config);
    }

    if (settings != NULL) {
        gpiod_line_settings_free(settings);
    }

    return request;
}

/*
 * 関数名：led_thread
 * 概要  ：LEDを指定された間隔で点滅させる。
 *         終了要求を受け取ると、LEDを消灯して終了する。
 * 引数  ：arg - スレッド間で共有する情報へのポインタ
 * 戻り値：なし
 */
static void* led_thread(void* arg)
{
    thread_data_t* data = arg;
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;
    double interval;
    int stop;

    /* GPIOチップを開く */
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL) {
        perror("gpiod_chip_open");
        return NULL;
    }

    /* LED用GPIOを出力として要求する */
    request = request_gpio(
        chip, LED_GPIO, GPIOD_LINE_DIRECTION_OUTPUT);

    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    while (1) {
        /* 共有変数を取得するため、ミューテックスをロックする */
        pthread_mutex_lock(&data->mutex);

        stop = data->stop_flag;
        interval = data->blink_interval;

        /* 共有変数の取得が終わったため、ロックを解除する */
        pthread_mutex_unlock(&data->mutex);

        /* 終了要求があれば、点滅処理を終了する */
        if (stop) {
            break;
        }

        /* LEDを点灯する */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("LED ON");
            break;
        }

        /* 点灯状態を指定時間維持する */
        sleep_ms((unsigned int)(interval * 1000));

        /* LEDを消灯する */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("LED OFF");
            break;
        }

        /* 消灯状態を指定時間維持する */
        sleep_ms((unsigned int)(interval * 1000));
    }

    /* スレッド終了時にLEDを消灯する */
    gpiod_line_request_set_value(
        request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE);

    /* GPIOの要求を解除する */
    gpiod_line_request_release(request);

    /* GPIOチップを閉じる */
    gpiod_chip_close(chip);

    return NULL;
}

/*
 * 関数名：switch_thread
 * 概要  ：スイッチの入力状態を監視する。
 *         スイッチが押されると、LEDの点滅間隔を切り替える。
 * 引数  ：arg - スレッド間で共有する情報へのポインタ
 * 戻り値：なし
 */
static void* switch_thread(void* arg)
{
    thread_data_t* data = arg;
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;
    enum gpiod_line_value value;
    double interval;
    int previous_pressed = 0;

    /* GPIOチップを開く */
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL) {
        perror("gpiod_chip_open");
        return NULL;
    }

    /* スイッチ用GPIOを入力として要求する */
    request = request_gpio(
        chip, SWITCH_GPIO, GPIOD_LINE_DIRECTION_INPUT);

    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    while (1) {
        /* 終了要求があれば、入力監視を終了する */
        if (is_stopping(data)) {
            break;
        }

        /* スイッチの現在の入力値を取得する */
        value = gpiod_line_request_get_value(request, SWITCH_GPIO);

        if (value < 0) {
            perror("gpiod_line_request_get_value");
            break;
        }

        /*
         * Active High：
         * GPIOがHIGHになったときにスイッチ押下と判断する。
         */
        if (value == GPIOD_LINE_VALUE_ACTIVE) {
            /* 前回押されていなかった場合のみ、押下処理を行う */
            if (!previous_pressed) {
                /* チャタリング対策として少し待機する */
                sleep_ms(30);

                /* 待機後にスイッチの状態を再確認する */
                value = gpiod_line_request_get_value(
                    request, SWITCH_GPIO);

                if (value < 0) {
                    perror("gpiod_line_request_get_value");
                    break;
                }

                /* 待機後もHIGHであれば、押下を確定する */
                if (value == GPIOD_LINE_VALUE_ACTIVE) {
                    /* 共有変数を変更するため、ミューテックスをロックする */
                    pthread_mutex_lock(&data->mutex);

                    /* 点滅間隔を遅い状態と速い状態で切り替える */
                    if (data->blink_interval == SLOW_INTERVAL) {
                        data->blink_interval = FAST_INTERVAL;
                    } else {
                        data->blink_interval = SLOW_INTERVAL;
                    }

                    /* 切り替え後の点滅間隔を取得する */
                    interval = data->blink_interval;

                    /* 共有変数の変更が終わったため、ロックを解除する */
                    pthread_mutex_unlock(&data->mutex);

                    /* 切り替え後の点滅間隔を表示する */
                    printf("LED点滅間隔：%.1f秒\n", interval);

                    /* 押下済みとして記録し、連続検出を防ぐ */
                    previous_pressed = 1;
                }
            }
        } else {
            /* スイッチが離されたら、次の押下を検出できるようにする */
            previous_pressed = 0;
        }

        /* 次の入力確認まで少し待機する */
        sleep_ms(50);
    }

    /* GPIOの要求を解除する */
    gpiod_line_request_release(request);

    /* GPIOチップを閉じる */
    gpiod_chip_close(chip);

    return NULL;
}

/*
 * 関数名：is_stopping
 * 概要  ：スレッドの終了要求状態を確認する。
 *         共有変数を安全に読み取るため、ミューテックスを使用する。
 * 引数  ：data - スレッド間で共有する情報へのポインタ
 * 戻り値：終了要求がある場合は1、ない場合は0
 */
static int is_stopping(thread_data_t* data)
{
    int stop;

    /* 共有変数を読み取るため、ミューテックスをロックする */
    pthread_mutex_lock(&data->mutex);

    stop = data->stop_flag;

    /* 読み取りが終わったため、ロックを解除する */
    pthread_mutex_unlock(&data->mutex);

    return stop;
}

/*
 * 関数名：main
 * 概要  ：プログラムの開始処理と終了処理を行う。
 *         LED制御用スレッドとスイッチ入力用スレッドを作成し、
 *         30秒間並行処理を実行する。
 * 引数  ：なし
 * 戻り値：正常終了の場合は0、異常終了の場合は1
 */
int main(void)
{
    pthread_t led_id;
    pthread_t switch_id;

    /* スレッド間で共有する情報を初期化する */
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

        /* 使用しなくなったミューテックスを破棄する */
        pthread_mutex_destroy(&data.mutex);

        return 1;
    }

    /* スイッチ入力用スレッドを作成する */
    ret = pthread_create(&switch_id, NULL, switch_thread, &data);
    if (ret != 0) {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));

        /* 作成済みのLEDスレッドに終了を通知する */
        pthread_mutex_lock(&data.mutex);
        data.stop_flag = 1;
        pthread_mutex_unlock(&data.mutex);

        /* LEDスレッドの終了を待つ */
        pthread_join(led_id, NULL);

        /* ミューテックスを破棄する */
        pthread_mutex_destroy(&data.mutex);

        return 1;
    }

    /* 並行処理の実行開始を表示する */
    printf("30秒間、並行処理を実行します。\n");

    /* 30秒間、メインスレッドを待機させる */
    sleep(30);

    /* 各スレッドに終了を通知する */
    pthread_mutex_lock(&data.mutex);
    data.stop_flag = 1;
    pthread_mutex_unlock(&data.mutex);

    /* 各スレッドの終了を待つ */
    pthread_join(led_id, NULL);
    pthread_join(switch_id, NULL);

    /* ミューテックスを破棄する */
    pthread_mutex_destroy(&data.mutex);

    /* プログラムの終了を表示する */
    printf("プログラムを終了します。\n");

    return 0;
}