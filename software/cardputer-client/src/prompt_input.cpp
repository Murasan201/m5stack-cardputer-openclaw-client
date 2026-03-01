#include "prompt_input.h"

const char* const PromptInput::kCandidates[] = {
    "あ", "い", "う", "え", "お", "か", "き", "く", "け", "こ",
    "さ", "し", "す", "せ", "そ", "た", "ち", "つ"
};

PromptInput::PromptInput()
    : candidateIndex(0), accumulated("") {}

void PromptInput::nextCandidate() {
    candidateIndex = (candidateIndex + 1) % kCandidateCount;
}

void PromptInput::previousCandidate() {
    candidateIndex = (candidateIndex + kCandidateCount - 1) % kCandidateCount;
}

void PromptInput::commitCandidate() {
    accumulated += kCandidates[candidateIndex];
}

void PromptInput::backspace() {
    if (accumulated.isEmpty()) {
        return;
    }
    int idx = accumulated.length() - 1;
    while (idx > 0 && (accumulated[idx] & 0xC0) == 0x80) {
        idx--;
    }
    accumulated.remove(idx);
}

void PromptInput::clear() {
    accumulated.clear();
    candidateIndex = 0;
}

const String& PromptInput::buffer() const {
    return accumulated;
}

String PromptInput::currentCandidate() const {
    return String(kCandidates[candidateIndex]);
}

bool PromptInput::hasContent() const {
    return !accumulated.isEmpty();
}
