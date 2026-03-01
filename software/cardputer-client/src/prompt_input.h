#pragma once

#include <Arduino.h>

class PromptInput {
public:
    PromptInput();

    // Add a romaji character; auto-converts to hiragana when a valid sequence is found
    void inputChar(char c);
    void backspace();
    void clear();
    // Toggle between romaji and direct (ASCII) input mode
    void toggleMode();
    // Flush pending romaji (e.g. lone "n" → "ん") before sending
    void flushPending();

    const String& buffer() const;
    const String& romajiPending() const;
    bool isJapaneseMode() const;
    bool hasContent() const;

private:
    void tryConvertRomaji();

    String accumulated;  // confirmed text (hiragana + ASCII)
    String romaji;       // pending romaji input
    bool japaneseMode;

    struct RomajiEntry {
        const char* romaji;
        const char* hiragana;
    };
    static const RomajiEntry kRomajiTable[];
    static const size_t kRomajiTableSize;
};
