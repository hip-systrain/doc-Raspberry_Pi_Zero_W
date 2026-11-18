## 3. プロジェクト作成

### 3.1 新規プロジェクトの作成

開発 PC に作業フォルダを作成する。第 1 章の手順で Raspberry Pi Zero W に SSH 接続できることを前提とする。

1. 開発 PC のターミナルで `mkdir pi_zero_w_c` を実行する。
   
   > 確認: 作業ディレクトリに `pi_zero_w_c` フォルダができれば成功。

2. SSH で接続した Pi のターミナルで `mkdir -p ~/pi_zero_w_c` を実行する。
   
   > 確認: `ls -d ~/pi_zero_w_c` で作業フォルダのパスが表示されれば成功。

### 3.2 プロジェクトの構成（フォルダ・ファイルの役割）

題材 1 では、開発 PC の作業フォルダに次のファイルを置く。

| ファイル | 役割 |
| --- | --- |
| `led_blink.c` | 赤 LED を点滅させる C ソース。4.1 節で作成する。 |
| `Makefile` | コンパイルとリンクの方法を定義する。 |
| `led_blink` | Pi 上で `make` を実行すると生成される実行ファイル。 |

1. 開発 PC の作業フォルダに、次の内容を `Makefile` として保存する。

   ```makefile
   CC = gcc
   CFLAGS += -std=c11 -Wall -Wextra -g
   CPPFLAGS += $(shell pkg-config --cflags libgpiod)
   LDLIBS += $(shell pkg-config --libs libgpiod)

   led_blink: led_blink.o
   	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
   ```

   > 確認: ファイル名が `Makefile` になっていれば成功。  
   > 注意: リンクコマンドの行頭にはタブを入力する。

2. 開発 PC のターミナルで、`Makefile` を Pi の作業フォルダへ転送する。`USER` と `HOST` は第 1 章で SSH 接続に使用したユーザー名と接続先に置き換える。

   ```sh
   scp pi_zero_w_c/Makefile USER@HOST:pi_zero_w_c/
   ```

   > 確認: Pi 側で `ls ~/pi_zero_w_c` を実行し、`Makefile` が表示されれば成功。

## 4. コーディング・ビルド・書込み(RTOSなし)

開発 PC で作成したソースを `scp` で Pi に転送し、Pi 上でビルドして実行する。この手順書では、この流れがテンプレートの「書込み」にあたる。

### 4.1 題材1: LED点滅

IoT 学習 HAT の赤 LED を 0.5 秒ごとに点灯・消灯する。点灯から次の点灯までは約 1 秒となる。

#### ① 作るもの

Pi 上で C プログラムを実行し、`libgpiod` で赤 LED の GPIO を制御する。

#### ② 配線

```mermaid
flowchart LR
    PI["Pi Zero W GPIO17（物理ピン 11）"] --> HAT["IoT 学習 HAT の赤 LED"]
```

第 1 章で取り付けた HAT を使用する。

#### ③ プログラム

1. Pi のターミナルで `pkg-config --modversion libgpiod` を実行する。

   > 確認: この下書きは libgpiod 2.x 用。第 1 章とバージョンをそろえ、1.x の場合は採用する API を確定してから進む。

2. 開発 PC の作業フォルダに、次の内容を `led_blink.c` として保存する。

   ```c
   #define _POSIX_C_SOURCE 200809L
   #include <gpiod.h>
   #include <signal.h>
   #include <stdio.h>
   #include <string.h>
   #include <time.h>

   static volatile sig_atomic_t stop;

   static void stop_blinking(int signal_number)
   {
       (void)signal_number;
       stop = 1;
   }

   int main(int argc, char **argv)
   {
       const unsigned int led = 17;
       const struct timespec half_second = { .tv_nsec = 500000000L };
       struct gpiod_chip *chip = NULL;
       struct gpiod_line_settings *settings = NULL;
       struct gpiod_line_config *config = NULL;
       struct gpiod_line_request *request = NULL;
       int result = 1;

       if (argc != 3 || (strcmp(argv[2], "high") != 0 &&
                         strcmp(argv[2], "low") != 0)) {
           fprintf(stderr, "Usage: %s /dev/gpiochipN high|low\n", argv[0]);
           return 1;
       }

       if (signal(SIGINT, stop_blinking) == SIG_ERR) {
           perror("signal");
           return 1;
       }

       chip = gpiod_chip_open(argv[1]);
       settings = gpiod_line_settings_new();
       config = gpiod_line_config_new();
       if (!chip || !settings || !config) {
           fprintf(stderr, "Failed to open or configure GPIO\n");
           goto cleanup;
       }

       // GPIO17 を消灯状態で出力に設定する。
       gpiod_line_settings_set_active_low(settings, strcmp(argv[2], "low") == 0);
       if (gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT) < 0 ||
           gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE) < 0 ||
           gpiod_line_config_add_line_settings(config, &led, 1, settings) < 0) {
           perror("GPIO settings");
           goto cleanup;
       }

       request = gpiod_chip_request_lines(chip, NULL, config);
       if (!request) {
           perror("GPIO request");
           goto cleanup;
       }

       // 点灯と消灯を 0.5 秒ごとに切り替える。
       enum gpiod_line_value value = GPIOD_LINE_VALUE_ACTIVE;
       while (!stop) {
           if (gpiod_line_request_set_value(request, led, value) < 0) {
               perror("GPIO write");
               goto cleanup;
           }
           if (nanosleep(&half_second, NULL) < 0 && !stop) {
               perror("nanosleep");
               goto cleanup;
           }
           value = value == GPIOD_LINE_VALUE_ACTIVE ?
               GPIOD_LINE_VALUE_INACTIVE : GPIOD_LINE_VALUE_ACTIVE;
       }
       result = 0;

   cleanup:
       if (request) {
           if (gpiod_line_request_set_value(request, led, GPIOD_LINE_VALUE_INACTIVE) < 0) {
               perror("GPIO off");
               result = 1;
           }
           gpiod_line_request_release(request);
       }
       if (config)
           gpiod_line_config_free(config);
       if (settings)
           gpiod_line_settings_free(settings);
       if (chip)
           gpiod_chip_close(chip);
       return result;
   }
   ```

   > 確認: `Makefile` と同じフォルダに `led_blink.c` があれば成功。

#### ④ ビルドと書込み

1. 開発 PC のターミナルで、ソースを Pi に転送する。`USER` と `HOST` は 3.2 節と同じ値を使用する。

   ```sh
   scp pi_zero_w_c/led_blink.c USER@HOST:pi_zero_w_c/
   ```

   > 確認: Pi 側で `ls ~/pi_zero_w_c` を実行し、`led_blink.c` が表示されれば成功。

2. Pi のターミナルで、`cd ~/pi_zero_w_c` を実行する。

   > 確認: `pwd` の出力の末尾が `/pi_zero_w_c` であれば成功。

3. Pi のターミナルで、`make` を実行する。

   > 確認: 初回は gcc のコンパイル・リンクコマンドが表示される。エラーがなく、`ls -l led_blink` で実行ファイルを確認できれば成功。

4. Pi のターミナルで、`./led_blink /dev/gpiochipN high` を実行する。`/dev/gpiochipN` を実機で BCM GPIO17 がオフセット 17 に対応するデバイスに置き換える。HAT のサンプルで赤 LED が Low 点灯と確認できた場合は、`high` を `low` に置き換える。

   > 確認: エラーが表示されず、赤 LED が点滅すれば成功。

#### ⑤ 動作確認

1. 赤 LED が約 0.5 秒点灯し、約 0.5 秒消灯する動作を繰り返すことを確認する。
2. Pi のターミナルで `Ctrl+C` を押す。

   > 確認: 点滅が止まり、ターミナルにプロンプトが戻れば成功。終了後の LED の状態も実機で確認する。

<!-- 実機確認後、第1章に合わせて OS、gcc、make、libgpiod のバージョンを記録する。
     HAT のサンプルと gpioinfo を照合し、GPIO チップ名・オフセット・点灯極性を確定する。 -->

<!-- 参考資料候補: https://libgpiod.readthedocs.io/en/master/core_line_settings.html -->
