<!--
  手順書テンプレート(全ボード共通)
  - このファイルをコピーして、ボードごとのフォルダに保存してから書き始める。
  - 章の番号と名前は変えない。該当しない章は消さずに「本ボードでは作成しない(理由:〜)」と書く。
  - ボード独自の内容は、第9章以降に追加章として書く。
  - このコメントはプレビューには表示されない。消さずに残してよい。
-->

# 【Raspberry Pi Zero W】開発手順書

| 項目 | 内容 |
| ---- | ---- |
| 対象ボード | Raspberry Pi Zero W |
| 開発言語 | C / Python |
| IDE・ツール | Tera Term 5.7.1 / Raspberry Pi Imager v2.0.12 / <br>OpenSSH_10 / GCC 14.2.0 / <br>GNU Make 4.4.1 / GDB 16.2 |
| SDK・ライブラリ | libgpiod 2.2.1 |
| 開発PCのOS | Windows 11 26H2 |
| 最終検証日 | 2026/11/20 |

## 改版履歴

| 版 | 日付 | 担当者 | 内容 |
|---|---|---|---|
| 0.1 | (yyyy/mm/dd) | | 新規作成 |

---

## 0. 概要

本手順書では、Raspberry Pi Zero Wを使用した開発環境の構築から、プログラムの作成・ビルド・実行、デバッグまでの手順を説明する。

Raspberry Pi Zero Wでは、Linuxを搭載した Raspberry Pi OS 上でプログラムを実行する。PCからSSHやシリアル通信を利用して操作し、GPIOなどの機能を使用した開発を行う。

### 0.1 この手順書でできること

本手順書に沿って作業することで、Raspberry Pi Zero Wを使用した基本的なプログラム開発ができるようになることを目的とする。

具体的には、以下の内容を学習する。

- Raspberry Pi OSの導入と開発環境の構築
- PCからRaspberry Pi Zero WへのSSH接続
- シリアルターミナルを使用した通信とログの確認
- プロジェクトの作成
- C言語・Pythonによるプログラムの作成、ビルド、実行
- GPIOを使用したLEDやスイッチの制御
- UARTを使用したシリアル通信
- GDBを使用したプログラムのデバッグ
- pthreadを使用したマルチスレッド処理

### 0.2 必要な機材

| 品名 | 数量 | 備考 |
|---|---|---|
| Raspberry Pi Zero W | 1台 | 開発対象のボード |
| IoT学習HATキット | 1個 | LED・スイッチなどの制御に使用 |
| microSDカード（16GB以上） | 1枚 | OSとプログラムの保存に使用 |
| USBケーブル（micro-B）と電源 | 1式 | Raspberry Pi Zero Wへの給電に使用 |
| CH340E USBシリアル変換モジュール | 必要な数 | PCとRaspberry Pi Zero W間のシリアル通信に使用 |
| ジャンパワイヤ（メス-メス） | 2本 | Raspberry Pi Zero WとCH340Eの接続に使用 |
| USBケーブル（Type-C） | 1本 | CH340EとPCの接続に使用 |

### 0.3 前提知識・事前準備

本手順書を進めるにあたり、以下の知識と準備を必要とする。

#### 前提知識

| 項目 | 説明 |
|---|---|
| Windowsの基本操作 | アプリケーションの起動、ファイル操作、デバイスマネージャーの確認など |
| コマンド操作 | Linuxのターミナルでコマンドを入力する基本操作 |
| C言語の基礎 | 変数、条件分岐、繰り返し、関数などの基本的な文法 |
| Pythonの基礎 | 変数、条件分岐、繰り返し、関数などの基本的な文法 |

#### 用語の説明

| 用語 | 説明 |
|---|---|
| Raspberry Pi OS | Raspberry Piで動作するLinuxベースのOS |
| microSDカード | OSやプログラムなどのデータを保存する記憶媒体 |
| SSH | ネットワークを経由して、別のコンピュータに接続し、コマンド操作を行うための通信方式 |
| GPIO | LEDやスイッチなどの外部機器と信号の入出力を行うための端子 |
| UART | 送信線と受信線を使用して、データを1ビットずつ順番に送受信するシリアル通信方式 |

#### 事前準備

- 開発用PCを用意する。
- Raspberry Pi Zero Wと必要な周辺機器を用意する。
- Raspberry Pi Zero Wを接続するWi-FiのSSIDとパスワードを確認する。
- PCにソフトウェアをインストールできる環境を用意する。

#### シリアル通信の通信条件

本手順書では、UARTによるシリアル通信に以下の通信条件を使用する。

| 通信条件 | 設定値 | 説明 |
|---|---|---|
| ボーレート | 115200 bps | 1秒間に送信するビット数を表す |
| データ長 | 8 bit | 1回のデータ送信で使用するデータビット数を表す |
| パリティ | None | パリティビットによる誤り検出を行わない設定 |
| ストップビット | 1 bit | 1回のデータ送信の終了を示すビット数を表す |
| フロー制御 | None | 送受信のタイミングを制御する仕組みを使用しない設定 |
## 1. 開発環境構築

本章では、Raspberry Pi Zero W で C 言語のプログラムを開発するために必要なツールとドライバを準備する。

### 1.1 ツールのインストール

本手順では、開発PCから Raspberry Pi Zero W を操作し、Raspberry Pi 上でプログラムのビルドやデバッグを行うためのツールを使用する。

#### 1.1.1 Raspberry Pi Imager のインストール

本節では、Raspberry Pi Zero W に OS を書き込むために使用する Raspberry Pi Imager のインストール手順を記載する。

1. 下記のリンクより、Raspberry Pi 公式ソフトウェアページにアクセスする。

[Raspberry Pi 公式ソフトウェアページ](https://www.raspberrypi.com/software/)

2. Windows 用の Raspberry Pi Imager をダウンロードする。

![Raspberry Pi Imager ダウンロード画面](../images/z1_RaspberryPiImagerDownload.png)

3. ダウンロードしたインストーラーをダブルクリックする。

4. ユーザーアカウント制御の画面が表示された場合は、内容を確認してインストールを許可する。

5. インストール画面が表示されたら、画面の指示に従ってインストールする。

![Raspberry Pi Imager インストール画面](../images/z1_RaspberryPiImagerInstall.png)

6. インストール完了後、スタートメニューから Raspberry Pi Imager を起動する。

![Raspberry Pi Imager 起動画面](../images/z1_RaspberryPiImagerStart.png)

> 確認：Raspberry Pi Imager のメイン画面が表示されればインストール成功。

> 注意：Raspberry Pi Imager で設定したホスト名・ユーザー名・パスワードは、後で SSH 接続するときに使用するため、忘れないように控えておくこと。


#### 1.1.1 TeraTermのインストール
本節では、PC とマイコンをシリアル通信で接続し、データの送受信や確認をするために使用する TeraTerm のインストール手順を記載する。
1. 下記のリンクより、 TerTerm 公式 GitHub にアクセスする。  
[TerTerm 公式 GitHub](https://github.com/TeraTermProject/teraterm/releases)
1. TeraTerm 5.7.1の Assets から **teraterm-5.7.1-x64.exe** をダウンロードする。  
![TeraTermダウンロード画面](../images/Z1_TeraTermDownLoad.png)  
1. ダウンロードした**teraterm-5.7.1-x64.exe** をダブルクリックする。
![TeraTermダウンロード選択画面](../images/S1_TeraTermDownLoadSelect.png)  
1. 「インストールモードの選択」画面が開かれ、**現在のユーザー用にインストール(M)(推奨)** を押下する。  
![インストールモードの選択画面](../images/S1_TeraTermInstallMode.png)  
1. 「セットアップに使用する言語の選択」画面が開かれ、インストール中に利用する言語を**日本語**を選択する。
![セットアップ言語選択画面](../images/S1_SetupLanguageSelect.png)  
1. 「使用許諾契約書の同意」画面が開かれ、同意する(A)を押下する。  
![使用許諾契約書画面](../images/S1_TeraTermLicense.png)  
1. 次へ(N)を押下する。  
![使用許諾契約書次へ画面](../images/S1_TeraTermLicenseNext.png)    
1. 「インストール先の指定」画面が開かれ、次へ(N)を押下する。  
![インストール先指定画面](../images/S1_TeraTermInstallLocate.png)  
1. 「コンポーネントの選択」画面が開かれ、次へ(N)を押下する。  
![コンポーネントの選択画面](../images/S1_TeraTermComponent.png)  
1. 「言語の選択」画面が開かれ、日本語(J)を押下する。  
![言語の選択画面](../images/S1_TeraTermLangugae.png)  
1. 「スタートメニューフォルダーの指定」画面が開かれ、次へ(N)を押下する。  
![スタートメニューフォルダーの指定画面](../images/S1_TeraTermStartMenu.png)  
1. 「追加タスクの選択」画面が開かれ、次へ(N)を押下する。  
![追加タスクの選択画面](../images/S1_TeraTermAddTask.png)  
1. 「インストールの準備完了」画面が開かれ、インストール(I)を押下する。  
![インストール準備完了](../images/S1_TeraTermSetupOK.png)  
    >確認：TeraTerm セットアップウィザードの完了が表示されればインストール成功  
    ![インストール完了画面](../images/S1_TeraTermInstallOK.png)  

#### 1.1.2 Windows OpenSSH の確認

Windows OpenSSH は、開発PCから Raspberry Pi Zero W に SSH 接続したり、ファイルを転送したりするために使用する。

1. コマンドプロンプトを起動する。
2. 以下のコマンドを実行する。

```cmd
ssh -V
```

3. OpenSSH のバージョンが表示されることを確認する。

> 確認：OpenSSH のバージョンが表示されれば成功。

> 注意：コマンドが認識されない場合は、Windows のオプション機能から OpenSSH クライアントをインストールする。

#### 1.1.3 Raspberry Pi 側の開発ツールの準備

Raspberry Pi 上で C 言語のプログラムをビルド・デバッグするため、以下のツールを使用する。

| ツール | 用途 |
|---|---|
| GCC | C 言語のコンパイル |
| GNU Make | ビルド作業の自動化 |
| GDB | プログラムのデバッグ |
| libgpiod | GPIO の操作 |

1. Tera Term を起動する。
2. Raspberry Pi Zero W に SSH 接続する。
3. パッケージ情報を更新する。

```bash
sudo apt update
```

4. 必要な開発ツールをインストールする。

```bash
sudo apt install -y build-essential
```

> 確認：インストールが完了し、エラーが表示されなければ成功。

### 1.2 ドライバのインストール

本手順では、CH340E USB シリアル変換モジュールを開発PCで認識させるためのドライバを準備する。

1. CH340E USB シリアル変換モジュールを開発PCに接続する。
2. Windows のデバイスマネージャーを起動する。
3. 「ポート（COM と LPT）」に接続した機器が表示されるか確認する。
4. 機器が認識されない場合は、CH340E 用ドライバを入手する。
5. ダウンロードしたドライバのインストーラーを実行する。
6. 画面の指示に従ってインストールする。
7. 必要に応じて USB シリアル変換モジュールを接続し直す。

> 確認：「ポート（COM と LPT）」に USB-SERIAL CH340 などの機器が表示されれば成功。

> 注意：ドライバの名称や表示内容は、ドライバのバージョンによって異なる場合がある。

### 1.3 インストールできたことの確認

インストールしたツールが使用できることを確認する。

#### 1.3.1 開発PC側の確認

1. コマンドプロンプトを起動する。
2. 以下のコマンドを実行する。

```cmd
ssh -V
```

3. OpenSSH のバージョンが表示されることを確認する。
4. Tera Term を起動できることを確認する。
5. デバイスマネージャーで CH340E が認識されていることを確認する。

> 確認：各ツールが起動でき、CH340E が認識されていれば成功。

#### 1.3.2 Raspberry Pi 側の確認

1. Tera Term を起動する。
2. Raspberry Pi Zero W に SSH 接続する。
3. GCC のバージョンを確認する。

```bash
gcc --version
```

4. GNU Make のバージョンを確認する。

```bash
make --version
```

5. GDB のバージョンを確認する。

```bash
gdb --version
```

6. libgpiod のバージョンを確認する。

```bash
pkg-config --modversion libgpiod
```

> 確認：各ツールのバージョンが表示されれば成功。

## 2. シリアルターミナルの使い方

本章では、CH340E USB シリアル変換モジュールを使用して、開発PCと Raspberry Pi Zero W をシリアル通信で接続し、Tera Term から操作する方法を説明する。

### 2.1 使用するツールと接続方法

本手順では、Tera Term と CH340E USB シリアル変換モジュールを使用して、Raspberry Pi Zero W と開発PCを接続する。

#### 2.1.1 使用するツール

| ツール | 用途 |
|---|---|
| Tera Term | シリアル通信によるコマンド入力・出力確認 |
| CH340E USB シリアル変換モジュール | PCとRaspberry Pi間のシリアル通信 |
| USBケーブル | CH340Eと開発PCの接続 |
| ジャンパーワイヤー | CH340EとRaspberry Piの接続 |

#### 2.1.2 Raspberry Pi と CH340E の接続

1. Raspberry Pi Zero W の電源が OFF になっていることを確認する。
2. CH340E USB シリアル変換モジュールと Raspberry Pi Zero W の GPIO ピンをジャンパーワイヤーで接続する。

| CH340E側 | Raspberry Pi Zero W側 | 物理ピン番号 | 
| -------- | --------------------- | ------------ | 
| TXD      | GPIO15（RXD）         | 10           | 
| RXD      | GPIO14（TXD）         | 8            | 
| GND      | GND                   | 6            | 

<!--※Raspberry Pi Zero W へのログインに必要なためTXDも接続する-->

![Raspberry Pi Zero WとCH340Eの接続図](../images/S2_SerialConnection.png)

3. CH340E USB シリアル変換モジュールを USB ケーブルで開発PCに接続する。
4. Raspberry Pi Zero W の電源を ON にする。

> 確認：CH340E が開発PCに接続され、Raspberry Pi Zero W の電源が ON になっていれば成功。

> 注意：CH340E の信号は 5V、Raspberry Pi Zero W は 3.3V。CH340E の TXD を Raspberry Pi Zero W につなぐと、Raspberry Pi Zero W が壊れるおそれがある。

#### 2.1.3 Tera Term の起動と接続

1. Tera Term を起動する。
2. 「新しい接続」画面で「シリアル」を選択する。
3. 接続先として CH340E に割り当てられた COM ポートを選択する。
4. 「OK」を押下する。

![Tera Term新しい接続画面](../images/S2_TeraTermNewConnection.png)

> 確認：Tera Term の画面が開き、選択した COM ポートに接続できれば成功。

### 2.2 通信設定

Raspberry Pi Zero W とシリアル通信を行うため、Tera Term の通信設定を行う。

1. Tera Term のメニューから「設定」→「シリアルポート」を選択する。
2. 以下の通信設定を行う。

| 設定項目 | 設定値 |
|---|---|
| ポート | CH340E に割り当てられた COM ポート |
| スピード | 115200 |
| データ | 8 bit |
| パリティ | none |
| ストップビット | 1 bit |
| フロー制御 | none |

![Tera Termシリアルポート設定画面](../images/S2_TeraTermSerialSetting.png)

3. 設定内容を確認して「OK」を押下する。

> 確認：各通信設定が表の値と一致していれば成功。

### 2.3 動作確認

シリアル通信によって Raspberry Pi Zero W の起動メッセージが表示されることを確認する。

1. Raspberry Pi Zero W の電源を OFF にする。
```bash
sudo shutdown -h now
```
2. Tera Term がシリアルポートに接続されていることを確認する。
3. Raspberry Pi Zero W の電源を ON にする。
4. Tera Term の画面に起動メッセージが表示されることを確認する。
5. ログインを求められた場合は、Raspberry Pi Imager で設定したユーザー名とパスワードを入力する。

> 確認：Raspberry Pi Zero W の起動メッセージが表示され、ログイン後にコマンドを入力できれば成功。

> 注意：パスワード入力中は、入力した文字が画面に表示されない。

## 5. コーディング・ビルド・書込み（FreeRTOS）

本手順書で使用するRaspberry Pi Zero Wは、Linuxが動作するボードである。

そのため、リアルタイムOSであるFreeRTOSの導入・設定は行わない。

また、FreeRTOSを使用した複数タスクの実行やタスク間通信についても、本手順書では実施しない。

なお、複数の処理を並行して実行する方法については、Linuxのpthreadを使用し、第9章「マルチスレッド（pthread）」で説明する。

### 5.1 FreeRTOSの導入・設定

Raspberry Pi Zero WではLinuxを使用するため、FreeRTOSの導入・設定は実施しない。

### 5.2 題材5：複数タスク

FreeRTOSを使用しないため、複数タスクの作成・実行は実施しない。

複数の処理を並行して実行する方法については、第9章「マルチスレッド（pthread）」で説明する。

### 5.3 題材6：タスク間通信

FreeRTOSを使用しないため、FreeRTOSによるタスク間通信は実施しない。

pthreadを使用したスレッド間のデータ共有と排他制御については、第9章「マルチスレッド（pthread）」で説明する。

## 6. デバッグ

デバッグとは、プログラムに含まれる不具合（バグ）の原因を調査し、修正する作業である。

プログラムを実行しながら処理の流れや変数の値を確認することで、想定どおりに動作していない箇所を特定する。

本研修では、デバッガとしてGDBを使用する。

#### GDBとは

GDB（GNU Debugger）は、プログラムの実行状態を確認しながらデバッグを行うためのツールである。

ブレークポイントの設定、ステップ実行、変数やメモリの値の確認などをコマンドで操作できる。

GCCと同様にLinux環境で利用でき、コマンドラインから操作する。

IDEに搭載されたデバッガを使用できない環境でも利用できるため、本研修ではGDBを使用する。

### 6.1 デバッガの接続（配線図・写真）

GDBを使用するために、Raspberry Pi Zero WへのSSH接続と、デバッグ用プログラムのビルドを行う。

#### 1. SSH接続

パソコンからSSHを使用してRaspberry Pi Zero Wに接続する。

#### 2. GDBのインストール確認

1. 以下のコマンドを実行する。

   ```bash
   gdb --version
   ```

2. GDBのバージョン情報が表示されることを確認する。

   > 確認：GDBのバージョン情報が表示されれば成功。

3. GDBがインストールされていない場合は、以下のコマンドでインストールする。

   ```bash
   sudo apt update
   sudo apt install gdb
   ```

#### 3. デバッグ対象のディレクトリへ移動

デバッグ対象のソースコードが保存されているディレクトリに移動する。

```bash
cd ディレクトリ名
```

#### 4. デバッグ用ビルド

1. コンパイル時にデバッグ情報を付加するため、`-g` オプションを指定してビルドする。

   ```bash
   gcc -g -o 実行ファイル名 ソースファイル名 -lgpiod
   ```

   pthreadを使用するプログラムの場合は、`-pthread` を追加する。

   ```bash
   gcc -g -pthread -o 実行ファイル名 ソースファイル名 -lgpiod
   ```

   ※ `-g` は、GDBでソースコードや変数の情報を確認するためのデバッグ情報を実行ファイルに付加するオプションである。

2. ビルド結果を確認する。

   > 確認：コンパイルエラーが発生せず、実行ファイルが作成されれば成功。

### 6.2 デバッグの開始

GDBを起動し、デバッグ対象のプログラムを実行する。

#### 1. GDBの起動

1. 以下のコマンドを実行してGDBを起動する。

   ```bash
   gdb ./実行ファイル名
   ```

2. GDBのコマンド入力画面が表示されることを確認する。

   > 確認：`(gdb)` と表示されれば成功。

#### 2. プログラムの実行

1. 以下のコマンドを実行してプログラムを開始する。

   ```gdb
   run
   ```

2. プログラムが最後まで実行されることを確認する。

   > 確認：プログラムが正常に終了すれば成功。

#### 3. デバッグの終了

以下のコマンドを実行してGDBを終了する。

```gdb
quit
```

### 6.3 ブレークポイント・ステップ実行

ブレークポイントを設定してプログラムの実行を一時停止し、ステップ実行によって処理の流れを確認する。

#### 1. ブレークポイントの設定

ブレークポイントは、プログラムの実行を一時停止させる位置を指定する機能である。

1. 以下のコマンドを実行し、デバッグ対象のプログラムに含まれる関数を確認する。

   ```gdb
   info functions
   ```

2. 以下のコマンドを実行し、指定した関数の先頭にブレークポイントを設定する。

   ```gdb
   break 関数名
   ```

   例：`main`関数にブレークポイントを設定する場合

   ```gdb
   break main
   ```

3. ブレークポイントが設定されたことを確認する。

   ```gdb
   info breakpoints
   ```

   > 確認：指定した関数にブレークポイントが設定されていれば成功。

また、ファイル名と行番号を指定して設定することもできる。

```gdb
break ファイル名:行番号
```

例：

```gdb
break main.c:20
```

#### 2. プログラムの実行・停止

1. ブレークポイントを設定後、以下のコマンドを実行してプログラムを開始する。

   ```gdb
   run
   ```

2. プログラムがブレークポイントの位置で停止することを確認する。

   > 確認：設定した位置で実行が停止すれば成功。

#### 3. ステップ実行

プログラムがブレークポイントで停止した状態から、ソースコードを確認し、ステップ実行によって処理の流れを確認する。

##### ① ソースコードの確認

`list`コマンドを使用すると、デバッグ対象のソースコードを表示できる。

```gdb
list
```

行番号を指定して表示することもできる。

```gdb
list 行番号
```

例：

```gdb
list 20
```

##### ② ステップ実行

以下のコマンドを使用して、プログラムを1行ずつ実行する。

| コマンド | 内容 |
|---|---|
| `next`（`n`） | 次の行へ進む。関数呼び出しがあっても関数の中には入らない。 |
| `step`（`s`） | 次の行へ進む。関数呼び出しがある場合は関数の中に入る。 |
| `continue`（`c`） | 次のブレークポイントまで実行を再開する。 |

#### 4. ブレークポイントの削除

設定されているブレークポイントは、以下のコマンドで確認できる。

```gdb
info breakpoints
```

ブレークポイントを削除する場合は、以下のコマンドを実行する。

```gdb
delete ブレークポイント番号
```

例：

```gdb
delete 1
```

> 確認：指定したブレークポイントが削除されていれば成功。

### 6.4 変数・メモリ・レジスタの確認

プログラムの実行を一時停止した状態で、変数やメモリ、CPUのレジスタに格納されている値を確認する。

#### 1. 変数の確認

`print` コマンドを使用すると、指定した変数の値を確認できる。

```gdb
print 変数名
```

例：

```gdb
print count
```

また、`info locals` を使用すると、現在の関数内で使用されているローカル変数を確認できる。

```gdb
info locals
```

`display` コマンドを使用すると、ステップ実行のたびに指定した変数の値を自動的に表示できる。

```gdb
display 変数名
```

#### 2. メモリの確認

`x` コマンドを使用すると、指定したメモリアドレスに格納されているデータを確認できる。

```gdb
x/4xb アドレス
```

例：

```gdb
x/4xb &reg
```

変数 `reg` が格納されているメモリアドレスから4バイト分のデータを16進数で表示する。

変数のアドレスを指定する場合は、変数名の前に `&` を付ける。

| 指定 | 内容 |
|---|---|
| `4` | 表示するデータの個数 |
| `x` | 16進数で表示 |
| `b` | 1バイト単位で表示 |
| `&reg` | 変数 `reg` のメモリアドレス |

※ 変数名だけを指定すると、変数の値がメモリアドレスとして扱われる。そのため、変数自体が格納されているメモリを確認する場合は `&` を付ける。

#### 3. レジスタの確認

レジスタとは、CPU内部にある高速な記憶領域であり、演算やデータの受け渡しなどに使用される。

`info registers` コマンドを使用すると、CPUのレジスタに格納されている値を確認できる。

```gdb
info registers
```

> 確認：レジスタ名と格納されている値が表示されれば成功。

※ 表示されるレジスタの種類はCPUのアーキテクチャによって異なる。

## 7. トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| | | |

## 8. 参考資料
- GDBマニュアルの和訳(https://www.asahi-net.or.jp/~wg5k-ickw/html/online/gdb-5.0/gdb-ja_toc.html)
- pthreads(7) — POSIXスレッドの概要(https://man7.org/linux/man-pages/man7/pthreads.7.html?utm_source=chatgpt.com)

## 9. マルチスレッド(pthread)
本章では、pthreadを使用したマルチスレッド処理について説明する。

pthreadの基本的な仕組みや関数の使い方を確認し、複数のスレッドで処理を実行するプログラムを作成する。

また、スレッド間でのデータ共有や排他制御、GDBを使用したデバッグ方法について学習する。

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
gcc -pthread -o thread_test thread_test.c　-lgpiod
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

```mermaid
sequenceDiagram
    autonumber
    participant Main as メインスレッド
    participant LED as LED制御スレッド
    participant SW as スイッチ入力スレッド
    participant GPIO as GPIO(libgpiod)

    Main->>Main: mutex・stop_flag初期化

    Main->>LED: pthread_create()
    activate LED
    LED->>GPIO: gpiod_chip_open()
    LED->>GPIO: request_gpio(GPIO17, OUTPUT)
    GPIO-->>LED: request

    Main->>SW: pthread_create()
    activate SW
    SW->>GPIO: gpiod_chip_open()
    SW->>GPIO: request_gpio(GPIO24, INPUT)
    GPIO-->>SW: request

    Main->>Main: 30秒間 sleep()

    par LED制御
        loop 終了要求まで繰り返す
            LED->>GPIO: GPIO17をHIGH
            LED->>LED: sleep(1秒)
            LED->>GPIO: GPIO17をLOW
            LED->>LED: sleep(1秒)
            LED->>Main: is_stopping()で終了確認
        end
    and スイッチ監視
        loop 終了要求まで繰り返す
            SW->>GPIO: GPIO24の入力値を取得
            GPIO-->>SW: 入力値
            alt スイッチが押された
                SW->>SW: 前回の状態を確認
                alt 前回は未押下
                    SW->>Main: 押下メッセージを表示
                    Note over SW: previous_pressed = 1
                end
            else スイッチが離された
                SW->>SW: previous_pressed = 0
            end
            SW->>SW: usleep(100ms)
        end
    end

    Main->>Main: mutexをロック
    Main->>Main: stop_flag = 1
    Main->>Main: mutexをアンロック

    Main->>LED: pthread_join()
    LED->>GPIO: GPIO17をLOW
    LED->>GPIO: gpiod_line_request_release()
    LED->>GPIO: gpiod_chip_close()
    LED-->>Main: スレッド終了
    deactivate LED

    Main->>SW: pthread_join()
    SW->>GPIO: gpiod_line_request_release()
    SW->>GPIO: gpiod_chip_close()
    SW-->>Main: スレッド終了
    deactivate SW

    Main->>Main: pthread_mutex_destroy()
    Main->>Main: プログラム終了
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
gcc -pthread -o thread_test thread_test.c -lgpiod
```

コンパイル後、以下のコマンドでプログラムを実行する。

```bash
./thread_test
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

```mermaid
sequenceDiagram
    autonumber
    participant Main as メインスレッド
    participant LED as LED制御スレッド
    participant SW as スイッチ入力スレッド
    participant Mutex as mutex
    participant GPIO as GPIO(libgpiod)

    Main->>Main: mutex・stop_flag初期化
    Note over Main: blink_interval = 1.0秒

    Main->>LED: pthread_create()
    activate LED
    LED->>GPIO: gpiod_chip_open()
    LED->>GPIO: request_gpio(GPIO17, OUTPUT)
    GPIO-->>LED: request

    Main->>SW: pthread_create()
    activate SW
    SW->>GPIO: gpiod_chip_open()
    SW->>GPIO: request_gpio(GPIO24, INPUT)
    GPIO-->>SW: request

    Main->>Main: 30秒間 sleep()

    par LED制御
        loop 終了要求まで繰り返す
            LED->>Mutex: pthread_mutex_lock()
            LED->>LED: stop_flag・blink_interval取得
            LED->>Mutex: pthread_mutex_unlock()

            alt stop_flag == 1
                LED->>LED: ループ終了
            else stop_flag == 0
                LED->>GPIO: GPIO17をHIGH
                LED->>LED: interval秒待機
                LED->>GPIO: GPIO17をLOW
                LED->>LED: interval秒待機
            end
        end
    and スイッチ監視
        loop 終了要求まで繰り返す
            SW->>Mutex: is_stopping()で終了状態を確認
            SW->>Mutex: pthread_mutex_lock()
            SW->>Mutex: stop_flag取得・unlock

            alt 終了要求あり
                SW->>SW: ループ終了
            else 終了要求なし
                SW->>GPIO: GPIO24の入力値を取得
                GPIO-->>SW: 入力値

                alt スイッチが押された
                    SW->>SW: previous_pressedを確認
                    alt 前回は未押下
                        SW->>SW: 30ms待機（チャタリング対策）
                        SW->>GPIO: GPIO24の入力値を再取得
                        GPIO-->>SW: 入力値

                        alt 再確認でも押下中
                            SW->>Mutex: pthread_mutex_lock()
                            SW->>SW: blink_intervalを切り替え
                            Note over SW: 1.0秒 ⇔ 0.2秒
                            SW->>Mutex: pthread_mutex_unlock()
                            SW->>SW: 点滅間隔を表示
                            SW->>SW: previous_pressed = 1
                        end
                    end
                else スイッチが離された
                    SW->>SW: previous_pressed = 0
                end

                SW->>SW: 50ms待機
            end
        end
    end

    Main->>Mutex: pthread_mutex_lock()
    Main->>Main: stop_flag = 1
    Main->>Mutex: pthread_mutex_unlock()

    Main->>LED: pthread_join()
    LED->>GPIO: GPIO17をLOW
    LED->>GPIO: gpiod_line_request_release()
    LED->>GPIO: gpiod_chip_close()
    LED-->>Main: スレッド終了
    deactivate LED

    Main->>SW: pthread_join()
    SW->>GPIO: gpiod_line_request_release()
    SW->>GPIO: gpiod_chip_close()
    SW-->>Main: スレッド終了
    deactivate SW

    Main->>Mutex: pthread_mutex_destroy()
    Main->>Main: プログラム終了
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
gcc -pthread -o thread_test thread_test.c -lgpiod
```

コンパイル後、以下のコマンドでプログラムを実行する。

```bash
./thread_test
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

<!-- ボード独自の追加章がある場合は、ここから「## 9. 〜」として追加する -->

<!--
  記述ルール(書きぶりを全章でそろえる)
  1. 見出し:章は「## 1. 開発環境構築」、節は「### 1.1 ツールのインストール」の形にする。
  2. 手順:番号付きリストで書き、1つの番号に1つの操作。文末は「〜する。」にそろえる。
  3. 確認:手順のあとに「> 確認:〜れば成功」と、成功したかどうかの判断方法を書く。
  4. 注意:まちがえやすい所は「> 注意:〜」と書く。
  5. 画像:画面操作にはスクリーンショットを付ける。images/ に置き、名前は担当番号から始める(例:p1_ext.png)。
  6. 図:処理の流れやタスクの関係は Mermaid で書く。配線は図か写真で、ピン番号と信号名を書く。
  7. プログラム:手順書の中にコードブロックで書き、日本語のコメントを付ける。
  8. バージョン:ツールと SDK のバージョンを書く。
  9. 用語:同じものは同じことばで書く。英数字は半角で、日本語との間に半角スペースを入れる。
  10. つまずいた点:第7章に必ず記録する。
-->