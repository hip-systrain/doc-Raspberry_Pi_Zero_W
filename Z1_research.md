# 調査報告
ここでは、手順書作成に向けた調査についてまとめる。<br>
実際の手順についての記録は別途、動作確認報告にて示すこととする。

## 0章 概要
### 0.0節 Raspberry Pi Zero Wの概要
Raspberry Piは、基板上にコンピュータとしての機能を備えた小型コンピュータである。microSDカードにOSをインストールすることで、一般的なPCと同様にプログラムを実行できる。<br>
今回使用するRaspberry Pi Zero Wは、Wi-Fiなどの無線通信機能を備えた小型モデルである。ネットワーク経由でPCから接続できるほか、GPIOと呼ばれる端子を利用して外部機器と通信することもできる。

参考資料
- [製品紹介：Raspberry Pi Zero W](https://www.raspberrypi.com/products/raspberry-pi-zero-w/)
- [Started Guide](https://www.raspberrypi.com/documentation/computers/getting-started.html)

### 0.1節 この手順書でできること
本手順書では、Raspberry Pi Zero Wの開発環境構築から、例題を用いたプログラムの作成・ビルド・書込みまでの一連の手順を説明する。<br>
また、シリアルターミナルを使用したログの確認やデバッグ、トラブルシューティングについても解説し、Raspberry Pi Zero Wを使用した基本的な開発ができるようになることを目的とする。

### 0.2節 必要な機材
共有されたスライドに以下の品目を追加する。
- USBケーブル(Type-C)

| 品名 | 数 | 用途 |
| --- | --- | --- |
| Raspberry Pi Zero W | 1人1台 | 開発するボード |
| IoT学習HATキット | 1人1個 | 題材で使う LED・スイッチなど |
| microSD カード(16GB 以上) | 1人1枚 | OSとプログラムの保存 |
| USBケーブル(micro-B)と電源 | 1人1式 | Raspberry Piへの給電 |
| CH340E USBシリアル変換モジュール | 必要な数 | シリアルターミナル用 |
| ジャンパワイヤ(メス-メス) | 1人2本 | シリアル通信の配線 |
| USBケーブル(Type-C) | 1人1本 | CH340Eへの給電およびPCとのシリアル通信 |

### 0.3節 前提知識・事前準備

参考資料
- [CH340E使用　USBシリアル変換モジュール](https://akizukidenshi.com/goodsaffix/AE-CH340E-TYPEC.pdf?utm_source=chatgpt.com)


## 1章 開発環境構築
### 1.1節 ツールのインストール
必要なツールを以下の表にまとめる。

| ツール名 | カテゴリ | 用途 |
| --- | --- | --- |
| Raspberry Pi Imager | ソフトウェア | microSDカードにRaspberry Pi OSを書き込む |
| SSH | コマンドラインツール | Raspberry Piにリモート接続して操作する |
| SCP | コマンドラインツール | PCとRaspberry Pi間でファイルを転送する |
| apt | OS標準コマンド | パッケージのインストール・更新・管理を行う |
| build-essential | APTパッケージ | GCCやMakeなど、C言語のコンパイル・ビルドに必要なツールをまとめて導入する |
| libgpiod-dev | APTパッケージ | C言語からGPIOを操作するための開発ライブラリ |
| gdb | APTパッケージ | C言語プログラムのデバッグを行う |
| Python 3 | OS標準ソフト | Pythonプログラムを実行する |
| python3-gpiozero | APTパッケージ | PythonからGPIOを操作するためのライブラリ |
| python3-serial | APTパッケージ | Pythonからシリアル通信を行うためのライブラリ |

上記の表で使用したカテゴリについて、以下にまとめる。

| カテゴリ | 意味 |
| --- | --- |
| ソフトウェア | PCなどにインストールして使用するアプリケーション |
| OS標準ソフト | OSに標準で搭載されているソフトウェア |
| OS標準コマンド | OSに標準で用意されているコマンド |
| コマンドラインツール | コマンド操作によって特定の機能を利用するツール |
| デバイスドライバ | OSが接続された機器を認識・制御するためのソフトウェア |
| APTパッケージ | aptを利用して導入するソフトウェアやライブラリ |


### 1.2節 ドライバのインストール
今回の開発では、CH340Eを搭載したUSBシリアル変換モジュールを使用する。

| ドライバ名 | 用途 |
| --- | --- |
| USBシリアル変換ドライバ（CH341SER） | PCでCH340Eを認識するため |

### 1.3節 インストールできたことの確認
インストールする項目について、どのように確認するかを示す。
工程としては、「確認→インストール→確認」の手順で行うものとする。

| 項目名 | 確認方法 | 確認内容 |
| Raspberry Pi Imager | アプリケーション一覧から起動する | 起動できること |
| SSH | PowerShellで ssh -V を実行する | SSHクライアントのバージョンが表示されること |
| SCP | PowerShellで scp を実行する | SCPコマンドの使用方法が表示されること |
| apt | apt --version | aptのバージョンが表示されること |
| build-essential | gcc --version、make --version | GCCとMakeのバージョンが表示されること |
| libgpiod-dev | pkg-config --modversion libgpiod | libgpiodのバージョンが表示されること |
| gdb | gdb --version | GDBのバージョンが表示されること |
| Python 3 | python3 --version | Python 3のバージョンが表示されること |
| python3-gpiozero | python3 -c "import gpiozero; print(gpiozero.__version__)" | GPIO Zeroのバージョンが表示されること |
| python3-serial | python3 -c "import serial; print(serial.__version__)" | pySerialのバージョンが表示されること |
| USBシリアル変換ドライバ（CH341SER） | デバイスマネージャーの「ポート（COMとLPT）」を確認する | 「USB-SERIAL CH340」などのデバイスが表示され、正常に認識されていること |


## 2章 シリアルターミナル
### 2.1節 使用するツールと接続方法
#### 使用ツールの選定
シリアルターミナルとして使用するツールについて、以下の3つを比較・検討する。<br>
比較した結果、GUI上で通信設定や操作を簡単に行えるTera Termを採用し、以降はTera Termを使用したシリアル通信の手順について説明する。

| ツール | 主な役割 | メリット | デメリット |
| --- | --- | --- | --- |
| コマンドプロンプト | Windowsのコマンドを実行する | ・Windowsに標準搭載されており、環境準備が不要 | ・標準機能だけではシリアル通信ができない |
| PowerShell | コマンド実行に加え、ファイル操作や設定確認などを効率的に行う | ・Windowsに標準搭載されており、環境準備が不要 <br>・SSH接続時、PCとPiの2画面で作業できる<br>・ボーレートなどの通信条件を設定できる | ・シリアル通信を行うためのコードを用意する必要がある |
| Tera Term | シリアル通信を行い、Piから送信された文字を受信・表示する | ・GUIで操作でき、設定や操作が簡単<br>・ボーレートなどの通信条件を画面上で設定できる | ・ソフトウェアのインストールが必要 |

#### 接続方法
Tera Termを起動し、USBシリアル変換モジュールを使用してRaspberry Piとシリアル接続する。

| 手順 | 操作内容 | 確認内容 |
| --- | --- | --- |
| 1 | Tera Termを起動する | Tera Termが起動すること |
| 2 | 「新しい接続」画面で「シリアル」を選択する | シリアル接続を選択できること |
| 3 | 接続先のポート（COM番号）を選択する | USBシリアル変換モジュールに割り当てられたCOMポートが表示されること |
| 4 | 「OK」をクリックする | Tera Termの端末画面が表示されること |

### 2.2節 通信設定(ポート、ボーレートなど)


### 2.3節 動作確認


