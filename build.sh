#!/usr/bin/env sh

BIN=backlight-control

LDFLAGS=-lm CFLAGS=-O2 make $BIN
sudo chown root $BIN
sudo chmod 4550 $BIN

