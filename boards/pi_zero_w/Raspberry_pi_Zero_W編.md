## 9. マルチスレッド(pthread)
本章では、pthreadを使用したマルチスレッド処理について説明する。

pthreadの基本的な仕組みや関数の使い方を確認し、複数のスレッドで処理を実行するプログラムを作成する。

また、スレッド間でのデータ共有や排他制御について学習する。

### 9.1 pthreadとは

pthread（POSIX Threads）とは、LinuxなどのOSで
マルチスレッド処理を実現するための仕組みである。

#### プロセスとスレッドの違い

プロセスとは、実行中のプログラムと、その実行に必要な
メモリなどの資源を管理する単位である。

スレッドとは、プロセス内で処理を実行する単位である。
1つのプロセス内に複数のスレッドを作成することで、
複数の処理を並行して実行できる。

同じプロセス内のスレッドはメモリなどの資源を共有するため、
スレッド間でデータを共有できる。

#### Raspberry Pi Zero Wでpthreadを使用する理由

Raspberry Pi Zero WではLinuxが動作している。
Linuxはマルチスレッド処理に対応しており、
C言語からpthreadを使用してスレッドを作成・管理できる。

本手順書では、FreeRTOSを使用せず、
Linuxのpthreadを利用して複数の処理を並行して実行する。

### 9.2 pthreadを使用するプログラムの作成

pthreadを使用すると、1つのプログラム内で複数のスレッドを作成し、それぞれのスレッドで処理を実行できる。

ここでは、pthreadを使用するための基本的な関数と、プログラムの作成・コンパイル方法を説明する。

#### pthreadの基本的な関数

pthreadでは、主に次の関数を使用する。

| 関数 | 役割 |
| ---- | ---- |
| `pthread_create()` | 新しいスレッドを作成し、処理を開始する |
| `pthread_join()` | 指定したスレッドの終了を待つ |
| `pthread_mutex_lock()` | 排他制御を開始し、共有データへのアクセスを制限する |
| `pthread_mutex_unlock()` | 排他制御を解除する |

#### pthreadを使用したプログラムの作成

pthreadを使用するには、`pthread.h`をインクルードする。

```c
#include <stdio.h>
#include <pthread.h>

/* スレッドで実行する関数 */
void* thread_func(void* arg)
{
    printf("スレッドで処理を実行します。\n");

    return NULL;
}

int main(void)
{
    pthread_t thread;

    /* スレッドを作成する */
    pthread_create(&thread, NULL, thread_func, NULL);

    /* スレッドの終了を待つ */
    pthread_join(thread, NULL);

    printf("メインスレッドの処理を終了します。\n");

    return 0;
}
```

このプログラムでは、`pthread_create()`で新しいスレッドを作成し、`thread_func()`を実行する。

その後、`pthread_join()`で作成したスレッドの終了を待つ。

#### コンパイル方法

pthreadを使用するプログラムは、コンパイル時に`-pthread`オプションを指定する。

```bash
gcc -g -pthread -o test test.c　-lgpiod
```

| オプション | 役割 |
| ---- | ---- |
| `-pthread` | pthreadを使用するためのコンパイル・リンク設定を行う |
| `-o thread_test` | 出力する実行ファイル名を指定する |

コンパイル後、次のコマンドでプログラムを実行する。

```bash
./thread_test
```
### 9.3 題材5：複数タスク

#### プログラムの説明

ここでは、pthreadを使用して、LED制御とスイッチ入力を別々のスレッドで実行するプログラムを作成する。

それぞれのスレッドで実行する処理は次のとおりである。

| スレッド | 処理内容 |
| ---- | ---- |
| スレッド1 | LEDを一定間隔で点灯・消灯する |
| スレッド2 | スイッチの入力状態を監視する |

メインスレッドで2つのスレッドを作成し、それぞれの処理を並行して実行する。

```C
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

    /* 指定したGPIO番号に設定を適用する */
    if (gpiod_line_config_add_line_settings(
            line_config, &offset, 1, settings) < 0) {
        perror("gpiod_line_config_add_line_settings");
        goto cleanup;
    }

    /* GPIOチップに対してGPIOの使用を要求する */
    request = gpiod_chip_request_lines(chip, NULL, line_config);

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

    /* LED用GPIOを出力として要求する */
    request = request_gpio(
        chip, LED_GPIO, GPIOD_LINE_DIRECTION_OUTPUT);

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

    /* スイッチ用GPIOを入力として要求する */
    request = request_gpio(
        chip, SWITCH_GPIO, GPIOD_LINE_DIRECTION_INPUT);

    /* 終了要求が出るまでスイッチの入力を監視する */
    while (!is_stopping(data)) {

        /* スイッチの現在の入力状態を取得する */
        value = gpiod_line_request_get_value(request, SWITCH_GPIO);

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

    /* スイッチ入力用スレッドを作成する */
    ret = pthread_create(&switch_id, NULL, switch_thread, &data);

    /* 2つのスレッドによる並行処理を30秒間実行する */
    printf("30秒間、並行処理を実行します。\n");
    sleep(30);

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

```

#### 処理の流れ

プログラム全体の処理の流れを以下に示す。

1. メインスレッドでLED制御用のスレッドを作成する。
2. メインスレッドでスイッチ入力用のスレッドを作成する。
3. LED制御用のスレッドでLEDの点灯・消灯を繰り返す。
4. スイッチ入力用のスレッドでスイッチの状態を繰り返し確認する。
5. 各スレッドの終了後、メインスレッドで終了を待つ。

#### プログラムの作成

`pthread_create()`を使用して、LED制御用とスイッチ入力用のスレッドを作成する。

それぞれのスレッドでは、専用の関数を実行する。

また、`pthread_join()`を使用して、作成したスレッドの終了を待つ。

#### コンパイル・実行

pthreadを使用するため、コンパイル時に`-pthread`オプションを指定する。

```bash
gcc -g -pthread -o no5 no5.c -lgpiod
```

コンパイル後、以下のコマンドでプログラムを実行する。

```bash
./no5
```

#### 動作確認

以下の動作を確認する。

- LEDが一定間隔で点灯・消灯すること。
- LEDの点灯・消灯中もスイッチの入力を受け付けること。
- プログラムを終了した際に、各スレッドが終了すること。

### 9.4 題材6：タスク間通信

#### プログラムの説明

ここでは、pthreadを使用して、スイッチの操作に応じてLEDの点滅速度を切り替えるプログラムを作成する。

それぞれのスレッドで実行する処理は次のとおりである。

| スレッド | 処理内容 |
| ---- | ---- |
| スレッド1 | LEDを点灯・消灯する |
| スレッド2 | スイッチの入力を監視し、LEDの点滅間隔を変更する |

スイッチ入力用のスレッドで変更した点滅間隔を、LED制御用のスレッドで参照することで、スレッド間でデータを共有する。

```C
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

    /* LED用GPIOを出力として要求する */
    request = request_gpio(
        chip, LED_GPIO, GPIOD_LINE_DIRECTION_OUTPUT);

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

    /* スイッチ用GPIOを入力として要求する */
    request = request_gpio(
        chip, SWITCH_GPIO, GPIOD_LINE_DIRECTION_INPUT);

    while (1) {
        /* 終了要求があれば、入力監視を終了する */
        if (is_stopping(data)) {
            break;
        }

        /* スイッチの現在の入力値を取得する */
        value = gpiod_line_request_get_value(request, SWITCH_GPIO);

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
 * 概要  ：スレッドを作成し、終了処理を行う。
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

    /* スイッチ入力用スレッドを作成する */
    ret = pthread_create(&switch_id, NULL, switch_thread, &data);

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
```

#### スレッド間のデータ共有

同じプロセス内のスレッドはメモリを共有するため、複数のスレッドから同じ変数にアクセスできる。

本プログラムでは、LEDの点滅間隔を共有変数として管理する。

| 共有変数 | 役割 |
| ---- | ---- |
| LEDの点滅間隔 | LED制御用スレッドが点滅間隔を参照し、スイッチ入力用スレッドが値を変更する |

複数のスレッドから同じ変数にアクセスする場合、データの読み書きが競合する可能性がある。

そのため、共有変数にアクセスする際には排他制御を行う。

#### 排他制御

排他制御とは、複数のスレッドが共有データに同時にアクセスしないように制御する仕組みである。

pthreadでは、mutexを使用して排他制御を行う。

| 関数 | 役割 |
| ---- | ---- |
| `pthread_mutex_lock()` | mutexをロックし、共有データへのアクセスを開始する |
| `pthread_mutex_unlock()` | mutexのロックを解除する |

共有変数を読み書きする際は、次の順番で処理する。

1. `pthread_mutex_lock()`でmutexをロックする。
2. 共有変数の値を読み取る、または変更する。
3. `pthread_mutex_unlock()`でmutexのロックを解除する。

これにより、共有変数へのアクセスを安全に行うことができる。

#### 処理の流れ

プログラム全体の処理の流れを以下に示す。

1. メインスレッドで共有変数とmutexを初期化する。
2. LED制御用のスレッドを作成する。
3. スイッチ入力用のスレッドを作成する。
4. LED制御用のスレッドで共有変数から点滅間隔を取得する。
5. スイッチ入力用のスレッドでスイッチの状態を確認する。
6. スイッチが操作された場合、mutexで排他制御を行い、共有変数の点滅間隔を変更する。
7. 各スレッドの終了後、メインスレッドで終了を待つ。

#### プログラムの作成

LED制御用のスレッドとスイッチ入力用のスレッドを作成する。

共有変数へのアクセスにはmutexを使用する。

LED制御用のスレッドでは、点滅間隔を取得した後にmutexのロックを解除してから、LEDの点灯・消灯や待機処理を行う。

#### コンパイル・実行

pthreadを使用するため、コンパイル時に`-pthread`オプションを指定する。

```bash
gcc -g -pthread -o no6 no6.c -lgpiod
```

コンパイル後、以下のコマンドでプログラムを実行する。

```bash
./no6
```

#### 動作確認

以下の動作を確認する。

- LEDが点滅すること。
- 白スイッチを操作するとLEDの点滅速度が切り替わること。
- スイッチの操作を繰り返しても正常に動作すること。
<--
### 9.5 GDBを使用したマルチスレッドのデバッグ

#### デバッグの説明

第6章で学習したGDBの操作を、pthreadを使用したプログラムにも適用する。

マルチスレッドのプログラムでは、複数のスレッドがそれぞれ処理を実行するため、GDBを使用して各スレッドの状態や実行位置を確認する。

#### GDBの操作

マルチスレッドのデバッグでは、主に次のコマンドを使用する。

| コマンド | 役割 |
| ---- | ---- |
| `info threads` | スレッドの一覧を表示する |
| `thread 2` | スレッドIDが2のスレッドに切り替える |
| `thread apply all bt` | すべてのスレッドのバックトレースを表示する |
| `break 関数名` | 指定した関数にブレークポイントを設定する |
| `continue` | 次のブレークポイントまで実行する |

#### 動作確認

以下の操作を行い、マルチスレッドのデバッグ方法を確認する。

1. デバッグ情報付きでプログラムをコンパイルする。
2. GDBを起動する。
3. `break`コマンドでスレッド内の関数にブレークポイントを設定する。
4. `run`コマンドでプログラムを実行する。
5. `info threads`コマンドでスレッドの一覧を確認する。
6. `thread`コマンドで確認したいスレッドに切り替える。
7. `thread apply all bt`コマンドですべてのスレッドの実行位置を確認する。
8. `continue`コマンドで次のブレークポイントまで実行する。
-->