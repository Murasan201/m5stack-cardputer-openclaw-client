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

    // Esc: Fn + ` (backtick) or raw 0x1B → return to Idle
    if (key == 0x1B || (keys.fn && key == '`')) {
        exitToIdle();
        return;
    }

    if (state == SessionState::Idle) {
        if (key != 0 || keys.enter) {
            enterPromptMode();
        }
    } else if (state == SessionState::Prompt) {
        handlePromptInput(keys, key);
    } else if (state == SessionState::Responding || state == SessionState::Error) {
        if (key != 0 || keys.enter) {
            enterPromptMode();
        }
    }
}

void DialogueManager::exitToIdle() {
    promptInput.clear();
    state = SessionState::Idle;
    display.showIdle();
}

void DialogueManager::enterPromptMode() {
    promptInput.clear();
    state = SessionState::Prompt;
    updatePromptDisplay();
}

void DialogueManager::sendActivePrompt() {
    promptInput.flushPending();

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

void DialogueManager::updatePromptDisplay() {
    String displayBuf = promptInput.buffer() + promptInput.romajiPending();
    String modeStr = promptInput.isJapaneseMode() ? "[あ]" : "[A]";
    display.showPromptMode(displayBuf, modeStr);
}

void DialogueManager::handlePromptInput(const Keyboard_Class::KeysState& keys, char key) {
    // Enter: send prompt
    if (keys.enter) {
        sendActivePrompt();
        return;
    }

    bool updated = false;

    // Backspace/Delete
    if (keys.del) {
        promptInput.backspace();
        updated = true;
    }
    // Tab: toggle Japanese/ASCII mode
    else if (keys.tab) {
        promptInput.toggleMode();
        updated = true;
    }
    // Printable character
    else if (key >= ' ' && key <= '~') {
        promptInput.inputChar(key);
        updated = true;
    }

    if (updated) {
        updatePromptDisplay();
    }
}
