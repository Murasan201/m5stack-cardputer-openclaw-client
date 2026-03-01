#include <M5Cardputer.h>

#include "display_manager.h"

void DisplayManager::begin() {
    M5.Lcd.begin();
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setFont(&fonts::efontJA_12);
}

void DisplayManager::showIdle() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(10, 15);
    M5.Lcd.println("待機中 - Aキーで対話");
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.println("日本語入力対応");
    M5.Lcd.println("送信: Cキー");
}

void DisplayManager::showPromptMode(const String& buffer, const String& candidate) {
    M5.Lcd.fillScreen(TFT_NAVY);
    M5.Lcd.setTextColor(TFT_CYAN, TFT_NAVY);
    M5.Lcd.setCursor(10, 5);
    M5.Lcd.println("OpenClaw 対話モード");

    M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.print("候補: ");
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_NAVY);
    M5.Lcd.println(candidate);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY);
    M5.Lcd.setCursor(10, 55);
    M5.Lcd.println("入力文字列:");
    M5.Lcd.setCursor(10, 75);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_NAVY);
    M5.Lcd.println(buffer);

    M5.Lcd.setTextColor(TFT_WHITE, TFT_NAVY);
    M5.Lcd.setCursor(10, 110);
    M5.Lcd.println("Bキー:確定  Cキー:送信");
    M5.Lcd.println("A+B同時:削除");
}

void DisplayManager::showSending() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
    M5.Lcd.setCursor(10, 60);
    M5.Lcd.println("OpenClaw に送信中...");
}

static void printWrapped(const String& text) {
    int line = 0;
    int idx = 0;
    while (idx < (int)text.length()) {
        int end = text.indexOf('\n', idx);
        String toPrint;
        if (end == -1) {
            toPrint = text.substring(idx);
            idx = text.length();
        } else {
            toPrint = text.substring(idx, end);
            idx = end + 1;
        }
        M5.Lcd.setCursor(10, 20 + line * 16);
        M5.Lcd.println(toPrint);
        line++;
        if (line > 7) {
            break;
        }
    }
}

void DisplayManager::showResponse(const String& response) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_CYAN, TFT_BLACK);
    M5.Lcd.setCursor(10, 5);
    M5.Lcd.println("OpenClaw 応答");
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    printWrapped(response);
    M5.Lcd.setCursor(10, 125);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.println("Aキー:再入力");
}

void DisplayManager::showError(const String& message) {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("エラー発生");
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 75);
    M5.Lcd.println(message);
    M5.Lcd.setCursor(10, 115);
    M5.Lcd.println("Aキーで再試行");
}

void DisplayManager::showStatusLine(const String& text) {
    clearRegion(10, 16);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println(text);
}

void DisplayManager::clearRegion(uint16_t y, uint16_t height) {
    M5.Lcd.fillRect(0, y, 320, height, TFT_BLACK);
}
