# 調査報告
## 0章 概要
### 0.0節 Raspberry Pi Zero Wの概要
Raspberry Piは、基板上にコンピュータとしての機能を備えた小型コンピュータです。microSDカードにOSをインストールすることで、一般的なPCと同様にプログラムを実行できます。

今回使用するRaspberry Pi Zero Wは、Wi-Fiなどの無線通信機能を備えた小型モデルです。ネットワーク経由でPCから接続できるほか、GPIOと呼ばれる端子を利用して外部機器と通信することもできます。

参考資料
- [製品紹介：Raspberry Pi Zero W](https://www.raspberrypi.com/products/raspberry-pi-zero-w/)
- [Started Guide](https://www.raspberrypi.com/documentation/computers/getting-started.html)

### 0.1節 この手順書でできること
本手順書では、Raspberry Pi Zero Wの開発環境構築から、例題を用いたプログラムの作成・ビルド・書込みまでの一連の手順を説明します。

また、シリアルターミナルを使用したログの確認やデバッグ、トラブルシューティングについても解説し、Raspberry Pi Zero Wを使用した基本的な開発ができるようになることを目的とします。

### 0.2節 必要な機材
共有されたスライドに以下の品目を追加
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
### 1.2節 ドライバのインストール
### 1.3節 インストールできたことの確認

## 2章 シリアルターミナル
### 2.1節 使用するツールと接続方法
### 2.2節 通信設定(ポート、ボーレートなど)
### 2.3節 動作確認
