
/*
 * ファイル名：93thread_test.c
 * 概要：
 *   LED制御とスイッチ入力を別々のスレッドで実行する。
 *
 *   LED制御スレッド：
 *     LEDを1秒間隔で点灯・消灯する。
 *
 *   スイッチ入力スレッド：
 *     スイッチの入力状態を監視し、押されたことを検出する。
 *
 *   メインスレッド：
 *     2つのスレッドを作成し、30秒後に終了させる。
 *
 * コンパイル：
 *   gcc -g -pthread -o 93thread_test 93thread_test.c -lgpiod
 *
 * 実行：
 *   ./93thread_test
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <gpiod.h>

/* GPIO番号の定義 */
#define LED_GPIO       17U
#define SWITCH_GPIO    24U

/* スレッド間で共有する情報を管理する構造体 */
typedef struct {
    pthread_mutex_t mutex;  /* 共有データへの同時アクセスを防ぐmutex */
    int stop_flag;          /* スレッドの終了を通知するフラグ */
} thread_data_t;


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
    /* GPIOの設定を管理する変数 */
    struct gpiod_line_settings* settings = NULL;

    /* GPIOの設定をまとめる変数 */
    struct gpiod_line_config* line_config = NULL;

    /* GPIOの要求結果を格納する変数 */
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

    /* GPIOを出力として使用する場合 */
    if (direction == GPIOD_LINE_DIRECTION_OUTPUT) {

        /* 初期状態をLOW（消灯）に設定する */
        if (gpiod_line_settings_set_output_value(
                settings, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("gpiod_line_settings_set_output_value");
            goto cleanup;
        }

    } else {

        /* GPIOを入力として使用する場合 */
        /* 内部プルダウン抵抗を有効にする */
        if (gpiod_line_settings_set_bias(
                settings, GPIOD_LINE_BIAS_PULL_DOWN) < 0) {
            perror("gpiod_line_settings_set_bias");
            goto cleanup;
        }
    }

    /* GPIOの設定をまとめるための領域を作成する */
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

    /* GPIOチップに対してGPIOの使用を要求する */
    request = gpiod_chip_request_lines(chip, NULL, line_config);
    if (request == NULL) {
        perror("gpiod_chip_request_lines");
    }

cleanup:
    /* 使用が終わった設定情報を解放する */
    if (line_config != NULL) {
        gpiod_line_config_free(line_config);
    }

    if (settings != NULL) {
        gpiod_line_settings_free(settings);
    }

    /* GPIOの要求結果を呼び出し元に返す */
    return request;
}


/*
 * 関数名：is_stopping
 * 概要  ：スレッドの終了要求が出ているか確認する。
 * 引数  ：data - スレッド間で共有する情報
 * 戻り値：1 - 終了要求あり
 *         0 - 終了要求なし
 */
static int is_stopping(thread_data_t* data)
{
    int stop;

    /* stop_flagを安全に読み取るためmutexをロックする */
    pthread_mutex_lock(&data->mutex);

    /* 終了フラグの値を取得する */
    stop = data->stop_flag;

    /* 読み取りが終わったためmutexを解除する */
    pthread_mutex_unlock(&data->mutex);

    return stop;
}


/*
 * 関数名：led_thread
 * 概要  ：LEDの点灯・消灯を繰り返すスレッド。
 * 引数  ：arg - スレッド間で共有する情報
 * 戻り値：NULL
 */
static void* led_thread(void* arg)
{
    /* メインスレッドから渡された共有情報を取得する */
    thread_data_t* data = arg;

    /* GPIOチップとLEDの要求情報 */
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;

    /* GPIOチップを開く */
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (chip == NULL) {
        perror("gpiod_chip_open");
        return NULL;
    }

    /* LED用GPIOを出力として要求する */
    request = request_gpio(
        chip, LED_GPIO, GPIOD_LINE_DIRECTION_OUTPUT);

    /* GPIOの要求に失敗した場合は終了する */
    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    /* 終了要求が出るまでLEDの点灯・消灯を繰り返す */
    while (!is_stopping(data)) {

        /* LEDを点灯する（HIGH） */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_ACTIVE) < 0) {
            perror("LED ON");
            break;
        }

        /* 1秒間待機する */
        sleep(1);

        /* LEDを消灯する（LOW） */
        if (gpiod_line_request_set_value(
                request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE) < 0) {
            perror("LED OFF");
            break;
        }

        /* 1秒間待機する */
        sleep(1);
    }

    /* スレッド終了時にLEDが点灯したままにならないよう消灯する */
    gpiod_line_request_set_value(
        request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE);

    /* LED用GPIOの使用を終了する */
    gpiod_line_request_release(request);

    /* GPIOチップを閉じる */
    gpiod_chip_close(chip);

    return NULL;
}


/*
 * 関数名：switch_thread
 * 概要  ：スイッチの入力状態を監視するスレッド。
 * 引数  ：arg - スレッド間で共有する情報
 * 戻り値：NULL
 */
static void* switch_thread(void* arg)
{
    /* メインスレッドから渡された共有情報を取得する */
    thread_data_t* data = arg;

    /* GPIOチップとスイッチの要求情報 */
    struct gpiod_chip* chip = NULL;
    struct gpiod_line_request* request = NULL;

    /* スイッチの現在の入力値 */
    enum gpiod_line_value value;

    /* 直前のスイッチ状態（0：未押下、1：押下中） */
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

    /* GPIOの要求に失敗した場合は終了する */
    if (request == NULL) {
        gpiod_chip_close(chip);
        return NULL;
    }

    /* 終了要求が出るまでスイッチの入力を監視する */
    while (!is_stopping(data)) {

        /* スイッチの現在の入力状態を取得する */
        value = gpiod_line_request_get_value(request, SWITCH_GPIO);

        /* 入力値の取得に失敗した場合は終了する */
        if (value < 0) {
            perror("gpiod_line_request_get_value");
            break;
        }

        /* スイッチの入力値を表示する（デバッグ用） */
        //printf("SWITCH_GPIO = %d\n", value);
        //fflush(stdout);

        /*
        * Active High方式：
        * スイッチが押されるとGPIOがHIGHになる。
        *
        * previous_pressedを使用することで、
        * 押し続けている間に何度も表示されることを防ぐ。
        */

        /* スイッチが押されている場合 */
        if (value == GPIOD_LINE_VALUE_ACTIVE) {
        
            /* 前回は押されていなかった場合 */
            if (!previous_pressed) {
                printf("スイッチが押されました。\n");
                fflush(stdout);
            
                /* 押下状態を記録する */
                previous_pressed = 1;
            }
        
        } else {
        
            /* スイッチが離されたため未押下状態に戻す */
            previous_pressed = 0;
        }

        /* 100ミリ秒待機してから次の入力確認を行う */
        usleep(100000);
    }

    /* スイッチ用GPIOの使用を終了する */
    gpiod_line_request_release(request);

    /* GPIOチップを閉じる */
    gpiod_chip_close(chip);

    return NULL;
}


/*
 * 関数名：main
 * 概要  ：スレッドを作成し、終了処理を行う。
 */
int main(void)
{
    /* 各スレッドを識別するためのID */
    pthread_t led_id;
    pthread_t switch_id;

    /* スレッド間で共有する情報を初期化する */
    thread_data_t data = {
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .stop_flag = 0
    };

    /* pthread関数の戻り値を格納する変数 */
    int ret;

    /* LED制御用スレッドを作成する */
    ret = pthread_create(&led_id, NULL, led_thread, &data);
    if (ret != 0) {
        /* スレッド作成に失敗した場合はエラー内容を表示する */
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));

        /* mutexを破棄してプログラムを終了する */
        pthread_mutex_destroy(&data.mutex);
        return 1;
    }

    /* スイッチ入力用スレッドを作成する */
    ret = pthread_create(&switch_id, NULL, switch_thread, &data);
    if (ret != 0) {
        /* スレッド作成に失敗した場合はエラー内容を表示する */
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));

        /*
         * スイッチ入力用スレッドの作成に失敗したため、
         * 先に作成したLED制御用スレッドに終了を通知する。
         */

        /* 終了フラグを変更するためmutexをロックする */
        pthread_mutex_lock(&data.mutex);

        /* LED制御用スレッドに終了を通知する */
        data.stop_flag = 1;

        /* 終了フラグの変更が終わったためmutexを解除する */
        pthread_mutex_unlock(&data.mutex);

        /* LED制御用スレッドの終了を待つ */
        pthread_join(led_id, NULL);

        /* mutexを破棄してプログラムを終了する */
        pthread_mutex_destroy(&data.mutex);
        return 1;
    }

    /* 2つのスレッドによる並行処理を30秒間実行する */
    printf("30秒間、並行処理を実行します。\n");
    sleep(30);

    /*
     * 30秒経過したため、各スレッドに終了を通知する。
     * stop_flagは複数スレッドから参照されるため、
     * mutexを使用して安全に変更する。
     */

    /* 終了フラグを変更するためmutexをロックする */
    pthread_mutex_lock(&data.mutex);

    /* 各スレッドに終了を通知する */
    data.stop_flag = 1;

    /* 終了フラグの変更が終わったためmutexを解除する */
    pthread_mutex_unlock(&data.mutex);

    /*
     * pthread_join()で各スレッドの終了を待つ。
     * スレッドが終了する前に共有データを破棄しないようにする。
     */

    /* LED制御用スレッドの終了を待つ */
    pthread_join(led_id, NULL);

    /* スイッチ入力用スレッドの終了を待つ */
    pthread_join(switch_id, NULL);

    /* 共有データの保護に使用したmutexを破棄する */
    pthread_mutex_destroy(&data.mutex);

    printf("プログラムを終了します。\n");

    return 0;
}
