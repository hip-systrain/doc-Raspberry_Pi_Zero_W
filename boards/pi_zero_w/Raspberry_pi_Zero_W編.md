<!--
  手順書テンプレート(全ボード共通)
  - このファイルをコピーして、ボードごとのフォルダに保存してから書き始める。
  - 章の番号と名前は変えない。該当しない章は消さずに「本ボードでは作成しない(理由:〜)」と書く。
  - ボード独自の内容は、第9章以降に追加章として書く。
  - このコメントはプレビューには表示されない。消さずに残してよい。
-->

# 【ボード名】開発手順書

| 項目 | 内容 |
|---|---|
| 対象ボード | |
| 開発言語 | |
| IDE・ツール | (名称とバージョン) |
| SDK・ライブラリ | (名称とバージョン) |
| 開発PCのOS | (例:Windows 11 24H2) |
| 最終検証日 | (yyyy/mm/dd) |

## 改版履歴

| 版 | 日付 | 担当者 | 内容 |
|---|---|---|---|
| 0.1 | (yyyy/mm/dd) | | 新規作成 |

---

## 0. 概要

### 0.1 この手順書でできること
<!-- 読者がこの手順書を最後まで進めると何ができるようになるかを書く -->

### 0.2 必要な機材

| 品名 | 数量 | 備考 |
|---|---|---|
| | | |

### 0.3 前提知識・事前準備
<!-- 既存の Git/GitHub 運用・Redmine 運用を参考に、前提とする知識と準備を書く -->

---

## 1. 開発環境構築

### 1.1 ツールのインストール
### 1.2 ドライバのインストール
### 1.3 インストールできたことの確認

## 2. シリアルターミナルの使い方

### 2.1 使用するツールと接続方法
### 2.2 通信設定(ポート、ボーレートなど)
### 2.3 動作確認

## 3. プロジェクト作成
本手順書でいう**プロジェクト**とは、プログラムの開発に使うファイルを1つの作業フォルダにまとめたものである。

### 3.1 新規プロジェクトの作成

開発 PC に作業フォルダを作成し、その中に `Makefile` と`ソースファイル(.c)`を用意する。<br>WinSCP で作業を Raspberry Pi Zero W に転送し、 WinSCP の両方のファイルが届いたことを確認する。
<br>

#### 【新規プロジェクトの作成手順】<br>
　今回の手順では、例として作業フォルダ名を `sample` 、ソースファイル名を `sample.c` とする。<br>

 > **手順前の準備**：Raspberry Pi Zero W に SSH 接続していない場合は、第 1 章の手順で開発 PC から接続する。<br>　　　　　　　※Raspberry Pi Zero W のターミナルは、転送したファイルの確認が終わるまで開いたままにする。
 <br>

#### 開発 PC に作業フォルダを作成する

1. 開発 PC のエクスプローラーを開く。<br>


2. 作業フォルダを作成する任意の場所に移動する。<br>
![任意のエクスプローラー](../pi_zero_w/images/z2_exp.png)

3. エクスプローラーのファイル一覧の空いている場所を右クリックし、タブを開く。
<br>
![タブ](../pi_zero_w/images/z2_file_tab.png)

4. 新規作成を選択する。
<br>
![新規作成](../pi_zero_w/images/z2_new_file.png)

5. フォルダーを選択する。
<br>
![フォルダー](../pi_zero_w/images/z2_new_file_create.png)

6. 名前を `sample` でフォルダを作成する。
<br>
![sampleフォルダ](../pi_zero_w/images/z2_sample_file.png)

#### VS Code で作業フォルダを開く

1. 開発 PC で VS Code を起動する。<br>

2. メニューバーの[ファイル]を選択する。<br>
![メニューバーファイル](../pi_zero_w/images/z2_vscode_file.png)

3. [フォルダーを開く]を選択する。<br>
![](../pi_zero_w/images/z2_vscode_folder.png)

3. フォルダー選択画面で `sample` フォルダを選択する。
<br>
![](../pi_zero_w/images/z2_vscode_folder_open.png)

4. [フォルダーの選択]をクリックする。
<br>
![](../pi_zero_w/images/z2_vscode_folder_open_click.png)
　
   >　確認： VS Code のエクスプローラーに `sample` フォルダが表示されれば成功。<br><br>
   ![](../pi_zero_w/images//z2_check_vscode_folder.png)

<br>

#### Makefile と ソースファイル(.c)を作成する

1. VS Code のエクスプローラーで、作業フォルダ名の横にある[新しいファイル]をクリックする。<br>
![](../pi_zero_w/images/z2_vscode_file_create.png)
<br>

2. ファイル名を `Makefile` で作成する。<br>※ファイル名は大文字の `M` で始め、拡張子は不要とする。<br>
![](../pi_zero_w/images/z2_vscode_makefile_create.png)

3.  `Makefile` の編集画面に、次の内容を記述し、保存する。

      ```makefile
      # C のコンパイラ、言語規格、警告、デバッグ情報の設定
      CC = gcc
      CFLAGS += -std=c11 -Wall -Wextra -g

      # Raspberry Pi Zero W の GRaspberry Pi Zero WO を操作するライブラリの設定
      CPPFLAGS += $(shell pkg-config --cflags libgpiod)
      LDLIBS += $(shell pkg-config --libs libgpiod)

      # 作業フォルダ直下のソースファイル(.c)から実行ファイル名を求める
      SOURCES := $(wildcard *.c)
      PROGRAMS := $(SOURCES:.c=)

      .PHONY: all
      all: $(PROGRAMS)

      # .c からコンパイルされた .o とライブラリをリンクして実行ファイルを作る
      $(PROGRAMS): %: %.o ; $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
      ```

      >　確認： `Makefile` の内容が次のようになっていれば成功。<br>
      ![](../pi_zero_w/images/z2_check_vscode_makefile.png)

<br>

4. VS Code のエクスプローラーで、再度[新しいファイル]をクリックする。<br>
<br>
![](../pi_zero_w/images/z2_vscode_cfile_create.png)

5. ファイル名を `sample.c` で作成する。<br>※プログラムは後で記述するため、この時点では空のファイルでよい。
<br>
![](../pi_zero_w/images/z2_vscode_c_file_create.png)

   >　確認： 作業フォルダの直下に `Makefile` と `sample.c` が並んで表示されていれば成功。<br><br>
   ![](../pi_zero_w/images/z2_check_vscode_files.png)


####  WinSCP で Raspberry Pi Zero W に接続する

1. 開発 PC で WinSCP を起動する。<br>

2. ログイン画面で[新しいサイト]を選択する。<br>
![](../pi_zero_w/images/z2_winscp_login.png)

3. セッションの[転送プロトコル]で `SCP` を選択する。<br>
![](../pi_zero_w/images/z2_winscp_session.png)

4. [ホスト名]に、**1.1.1 Raspberry Pi Imager のインストール**で設定した `hostname` を設定する。
<br>
![](../pi_zero_w/images/z2_winscp_hostname.png)

5. [ポート番号]に、標準のSSHポートである `22` を設定する。
<br>
![](../pi_zero_w/images/z2_winscp_poot.png)

6. [ユーザー名]に、**1.1.1 Raspberry Pi Imager のインストール**で設定した `username` を設定する。
<br>
![](../pi_zero_w/images/z2_winscp_username.png)

7. [保存]をクリックする。
<br>
![](../pi_zero_w/images/z2_winscp_save.png)

8. [セッションの保存名]を入力する。<br>※今回の手順では、 `Raspberry Pi Zero W` と入力する。
<br>
![](../pi_zero_w/images/z2_winscp_savename.png)

9. [OK]をクリックしてセッションを保存する。
<br>
![](../pi_zero_w/images/z2_winscp_session_save.png)

   >　確認： ログイン画面の左側の一覧から、保存した `Raspberry Pi Zero W` が表示されていれば成功。<br><br>
   ![](../pi_zero_w/images//z2_check_winscp_session.png)

10. ログイン画面の左側の一覧から、保存した `Raspberry Pi Zero W` を選択する。
<br>
![](../pi_zero_w/images/z2_winscp_login_raspizero.png)

11. [ログイン]をクリックする。<br>
※パスワードの入力を求められた場合は、**1.1.1 Raspberry Pi Imager のインストール**で設定したログインユーザーのパスワードを入力する。
<br>
![](../pi_zero_w/images/z2_winscp_login_click.png)

      >　確認：  WinSCP のリモート側(右側)に Raspberry Pi Zero W のファイル一覧が表示されれば接続成功。<br>
      ![](../pi_zero_w/images/z2_check_winscp.png)

####  作業フォルダを転送する
1. WinSCP のリモート側で、ログインユーザーのホームフォルダを表示する。今回の手順では、ユーザー名を `pi` 、ホームフォルダを `/home/pi` とする。<br>
 別の場所が表示されている場合は、リモート側の一覧をクリックして `Ctrl` + `O` を押し、表示されたダイアログにホームフォルダのパスを入力して開く。<br>
![](../pi_zero_w/images/z2_winscp_pi.png)

<br>

2. 開発 PC のエクスプローラーで、`sample` フォルダが見えるように、その 1 つ上のフォルダを表示する。<br>
![](../pi_zero_w/images/z2_winscp_samplefolder.png)

3. エクスプローラーから `sample` フォルダをドラッグし、WinSCP のリモート側のファイル一覧の空いている場所にドロップする。<br>
![](../pi_zero_w/images/z2_winscp_sample_folder_cd.png)
      >　注意： 別のフォルダのアイコンに重ねると、そのフォルダが転送先になる。転送先に同名のファイルがある場合は、上書きするファイルを確認する。<br>




4. アップロード画面の「OK」を選択し、転送を開始する。<br>
![](../pi_zero_w/images/z2_winscp_forward_OK.png)

   >　確認： 転送先のリモートディレクトリのパスがあっているかを確認。<br>![](../pi_zero_w/images/z2_winscp_forward.png)

5. 転送が終わったら、WinSCP のリモート側に sample フォルダが表示されたことを確認する。<br>
![](../pi_zero_w/images/z2_check_winscp_sample_folder.png)
      >　確認： Raspberry Pi Zero W 側の `sample` フォルダに `Makefile` と `sample.c` の両方が表示されれば、新規プロジェクトの作成は完了。<br><br>![](../pi_zero_w/images/z2_check_winscp_file.png)

####  Raspberry Pi Zero W 側で転送したファイルを確認する

1. WinSCP のリモート側で、転送した `sample` フォルダを開く。<br>
![](../pi_zero_w/images//z2_winscp_remote_folder.png)
<br>

2. 開発 PC のエクスプローラーで `sample` フォルダを開く。<br>
![](../pi_zero_w/images/z2_winscp_pc_folder.png)

3. 開発 PC と WinSCP のリモート側のファイル一覧を見比べる。<br>
![](../pi_zero_w/images/z2_check_winscp_sample_files.png)



### 3.2 プロジェクトの構成（フォルダ・ファイルの役割）
本プロジェクトでは、開発 PC でプログラムを作成・編集し、Raspberry Pi Zero W 上でビルドして動作を確認する。<br>`ソースファイル(.c)`とビルド手順を記述した `Makefile` を 1 つの作業フォルダにまとめ、フォルダごと Raspberry Pi Zero W に転送できるように構成している。<br>
次の表に、開発 PC と Raspberry Pi Zero W における作業フォルダと各ファイルの役割を示す。

| フォルダ　　| 開発 PC での役割 | Raspberry Pi Zero W での役割 |
| --- | --- | --- |
| 作業フォルダ | プロジェクトで使うファイルをまとめて編集する場所。| 転送したファイルを受け取り、ビルドや実行を行う場所。 |
<br>

| ファイル　| 開発 PC での役割 | Raspberry Pi Zero W での役割 |
| --- | --- | --- |
| Makefile | 実行ファイルを作るために使うファイルとコマンドを記述する。 | `make` がこのファイルを読み、ビルドに必要なコマンドを実行する。 |
| ソースファイル(.c) | プログラムの処理をC言語で記述する。 | `gcc` がオブジェクトファイル(`.o`)に変換する。この処理をコンパイルという。|
| オブジェクトファイル(.o) | ― | コンパイルで生成される中間ファイル。これをGPIO操作用のライブラリ `libgpiod` と結び付けて実行ファイルを作る処理をリンクという。 |
| 実行ファイル | ― | リンクによって作業フォルダに生成される。このファイルを起動して、プログラムの動作を確認する。 |
<br>

## 4. コーディング・ビルド・書込み(RTOSなし)

<!-- 各題材の節は「作るもの → 配線 → プログラム → ビルドと書込み → 動作確認」の順で書く -->

### 4.1 題材1:LED点滅
### 4.2 題材2:ボタン入力
### 4.3 題材3:UART出力
### 4.4 題材4:タイマ割込み

## 5. コーディング・ビルド・書込み(FreeRTOS)

### 5.1 FreeRTOSの導入・設定
### 5.2 題材5:複数タスク
### 5.3 題材6:タスク間通信

## 6. デバッグ

### 6.1 デバッガの接続(配線図・写真)
### 6.2 デバッグの開始
### 6.3 ブレークポイント・ステップ実行
### 6.4 変数・メモリ・レジスタの確認

## 7. トラブルシューティング

| 症状 | 原因 | 対処 |
|---|---|---|
| | | |

## 8. 参考資料

- 

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
