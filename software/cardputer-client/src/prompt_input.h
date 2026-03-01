#pragma once

#include <Arduino.h>

class PromptInput {
public:
    PromptInput();

    void nextCandidate();
    void previousCandidate();
    void commitCandidate();
    void backspace();
    void appendChar(char c);
    void clear();

    const String& buffer() const;
    String currentCandidate() const;
    bool hasContent() const;

private:
    static const char* const kCandidates[];
    static constexpr size_t kCandidateCount = 18;

    size_t candidateIndex;
    String accumulated;
};
