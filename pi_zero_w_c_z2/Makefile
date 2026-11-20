# C のコンパイラ、言語規格、警告、デバッグ情報の設定
CC = gcc
CFLAGS += -std=c11 -Wall -Wextra -g

# Pi の GPIO を操作するライブラリの設定
CPPFLAGS += $(shell pkg-config --cflags libgpiod)
LDLIBS += $(shell pkg-config --libs libgpiod)

# 作業フォルダ直下の .c ファイルから実行ファイル名を求める
SOURCES := $(wildcard *.c)
PROGRAMS := $(SOURCES:.c=)

.PHONY: all
all: $(PROGRAMS)

# .c からコンパイルされた .o とライブラリをリンクして実行ファイルを作る
$(PROGRAMS): %: %.o ; $(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)
