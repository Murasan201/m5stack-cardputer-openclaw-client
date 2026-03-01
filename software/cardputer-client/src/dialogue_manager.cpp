#include <M5Cardputer.h>

#include "dialogue_manager.h"

void DialogueManager::begin() {
    display.begin();
    network.begin();
    display.showIdle();
}

void DialogueManager::loop() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) {
        return;
    }

    Keyboard_Class::KeysState keys = M5Cardputer.Keyboard.keysState();
    char key = 0;
    if (!keys.word.empty()) {
        key = keys.word[0];
    }

    if (state == SessionState::Idle) {
        // Any key starts prompt mode
        if (key != 0 || keys.enter) {
            enterPromptMode();
        }
    } else if (state == SessionState::Prompt) {
        handlePromptInput(keys, key);
    } else if (state == SessionState::Responding || state == SessionState::Error) {
        // Any key returns to prompt mode
        if (key != 0 || keys.enter) {
            enterPromptMode();
        }
    }
}

void DialogueManager::enterPromptMode() {
    promptInput.clear();
    state = SessionState::Prompt;
    display.showPromptMode(promptInput.buffer(), promptInput.currentCandidate());
}

void DialogueManager::sendActivePrompt() {
    if (!promptInput.hasContent()) {
        display.showStatusLine("入力文字がありません");
        return;
    }

    state = SessionState::Sending;
    display.showSending();

    const PromptResponse response = network.postPrompt(promptInput.buffer());
    if (!response.success) {
        state = SessionState::Error;
        display.showError(response.error);
        return;
    }

    state = SessionState::Responding;
    promptInput.clear();
    display.showResponse(response.text);
}

void DialogueManager::handlePromptInput(const Keyboard_Class::KeysState& keys, char key) {
    bool updated = false;

    // Enter key: send prompt
    if (keys.enter) {
        sendActivePrompt();
        return;
    }

    // Backspace/Delete
    if (keys.del) {
        promptInput.backspace();
        updated = true;
    }
    // Tab: cycle candidate
    else if (keys.tab) {
        promptInput.nextCandidate();
        updated = true;
    }
    // Space: commit current candidate
    else if (key == ' ') {
        promptInput.commitCandidate();
        updated = true;
    }
    // Printable character: type directly into buffer
    else if (key >= '!' && key <= '~') {
        promptInput.appendChar(key);
        updated = true;
    }

    if (updated) {
        display.showPromptMode(promptInput.buffer(), promptInput.currentCandidate());
    }
}
