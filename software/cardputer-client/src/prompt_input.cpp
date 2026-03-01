#include "prompt_input.h"

// Romaji to Hiragana conversion table
// Longer sequences must come before shorter ones (e.g., "sha" before "sh")
const PromptInput::RomajiEntry PromptInput::kRomajiTable[] = {
    // double consonant (っ)
    {"bb",  "っb"}, {"cc",  "っc"}, {"dd",  "っd"}, {"ff",  "っf"},
    {"gg",  "っg"}, {"hh",  "っh"}, {"jj",  "っj"}, {"kk",  "っk"},
    {"mm",  "っm"}, {"pp",  "っp"}, {"rr",  "っr"}, {"ss",  "っs"},
    {"tt",  "っt"}, {"ww",  "っw"}, {"yy",  "っy"}, {"zz",  "っz"},
    // 3-char sequences
    {"sha", "しゃ"}, {"shi", "し"},   {"shu", "しゅ"}, {"sho", "しょ"},
    {"chi", "ち"},   {"tsu", "つ"},
    {"cha", "ちゃ"}, {"chu", "ちゅ"}, {"cho", "ちょ"},
    {"tya", "ちゃ"}, {"tyi", "ちぃ"}, {"tyu", "ちゅ"}, {"tyo", "ちょ"},
    {"nya", "にゃ"}, {"nyi", "にぃ"}, {"nyu", "にゅ"}, {"nyo", "にょ"},
    {"hya", "ひゃ"}, {"hyi", "ひぃ"}, {"hyu", "ひゅ"}, {"hyo", "ひょ"},
    {"mya", "みゃ"}, {"myi", "みぃ"}, {"myu", "みゅ"}, {"myo", "みょ"},
    {"rya", "りゃ"}, {"ryi", "りぃ"}, {"ryu", "りゅ"}, {"ryo", "りょ"},
    {"gya", "ぎゃ"}, {"gyi", "ぎぃ"}, {"gyu", "ぎゅ"}, {"gyo", "ぎょ"},
    {"bya", "びゃ"}, {"byi", "びぃ"}, {"byu", "びゅ"}, {"byo", "びょ"},
    {"pya", "ぴゃ"}, {"pyi", "ぴぃ"}, {"pyu", "ぴゅ"}, {"pyo", "ぴょ"},
    {"kya", "きゃ"}, {"kyi", "きぃ"}, {"kyu", "きゅ"}, {"kyo", "きょ"},
    {"jya", "じゃ"}, {"jyu", "じゅ"}, {"jyo", "じょ"},
    // 2-char sequences
    {"ka", "か"}, {"ki", "き"}, {"ku", "く"}, {"ke", "け"}, {"ko", "こ"},
    {"sa", "さ"}, {"si", "し"}, {"su", "す"}, {"se", "せ"}, {"so", "そ"},
    {"ta", "た"}, {"ti", "ち"}, {"tu", "つ"}, {"te", "て"}, {"to", "と"},
    {"na", "な"}, {"ni", "に"}, {"nu", "ぬ"}, {"ne", "ね"}, {"no", "の"},
    {"ha", "は"}, {"hi", "ひ"}, {"hu", "ふ"}, {"he", "へ"}, {"ho", "ほ"},
    {"ma", "ま"}, {"mi", "み"}, {"mu", "む"}, {"me", "め"}, {"mo", "も"},
    {"ya", "や"},                {"yu", "ゆ"},                {"yo", "よ"},
    {"ra", "ら"}, {"ri", "り"}, {"ru", "る"}, {"re", "れ"}, {"ro", "ろ"},
    {"wa", "わ"}, {"wi", "ゐ"},                {"we", "ゑ"}, {"wo", "を"},
    {"ga", "が"}, {"gi", "ぎ"}, {"gu", "ぐ"}, {"ge", "げ"}, {"go", "ご"},
    {"za", "ざ"}, {"zi", "じ"}, {"zu", "ず"}, {"ze", "ぜ"}, {"zo", "ぞ"},
    {"da", "だ"}, {"di", "ぢ"}, {"du", "づ"}, {"de", "で"}, {"do", "ど"},
    {"ba", "ば"}, {"bi", "び"}, {"bu", "ぶ"}, {"be", "べ"}, {"bo", "ぼ"},
    {"pa", "ぱ"}, {"pi", "ぴ"}, {"pu", "ぷ"}, {"pe", "ぺ"}, {"po", "ぽ"},
    {"fa", "ふぁ"}, {"fi", "ふぃ"}, {"fu", "ふ"}, {"fe", "ふぇ"}, {"fo", "ふぉ"},
    {"ja", "じゃ"}, {"ji", "じ"}, {"ju", "じゅ"}, {"je", "じぇ"}, {"jo", "じょ"},
    {"nn", "ん"},
    // 1-char vowels
    {"a", "あ"}, {"i", "い"}, {"u", "う"}, {"e", "え"}, {"o", "お"},
    {"n", "ん"},  // handled specially below
};

const size_t PromptInput::kRomajiTableSize =
    sizeof(kRomajiTable) / sizeof(kRomajiTable[0]);

PromptInput::PromptInput()
    : accumulated(""), romaji(""), japaneseMode(true) {}

void PromptInput::inputChar(char c) {
    if (!japaneseMode) {
        accumulated += c;
        return;
    }

    // Convert to lowercase for romaji matching
    if (c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
    }

    // Non-alpha characters: flush pending romaji, then append directly
    if (c < 'a' || c > 'z') {
        // Flush pending 'n' as ん if followed by non-alpha
        if (romaji == "n") {
            accumulated += "ん";
            romaji = "";
        } else if (!romaji.isEmpty()) {
            // Can't convert, flush as-is
            accumulated += romaji;
            romaji = "";
        }
        accumulated += c;
        return;
    }

    romaji += c;
    tryConvertRomaji();
}

void PromptInput::tryConvertRomaji() {
    // Special case: single 'n' followed by a consonant (not a/i/u/e/o/y/n)
    if (romaji.length() >= 2 && romaji[0] == 'n') {
        char second = romaji[1];
        if (second != 'a' && second != 'i' && second != 'u' &&
            second != 'e' && second != 'o' && second != 'y' && second != 'n') {
            accumulated += "ん";
            romaji = romaji.substring(1);
            // Continue to try converting the rest
        }
    }

    // Try to find exact match (longest first since table is ordered that way)
    for (size_t i = 0; i < kRomajiTableSize; i++) {
        const char* pattern = kRomajiTable[i].romaji;
        size_t plen = strlen(pattern);

        if (romaji.length() >= plen && romaji.startsWith(pattern)) {
            const char* result = kRomajiTable[i].hiragana;
            // Check if result contains a trailing consonant (for っ entries)
            size_t rlen = strlen(result);
            if (rlen > 0 && result[rlen - 1] >= 'a' && result[rlen - 1] <= 'z') {
                // e.g., "っk" -> append っ and keep 'k' in romaji
                String hira = String(result).substring(0, rlen - 1);
                accumulated += hira;
                romaji = String(result[rlen - 1]) + romaji.substring(plen);
            } else {
                accumulated += result;
                romaji = romaji.substring(plen);
            }

            // Recursively try to convert remaining romaji
            if (!romaji.isEmpty()) {
                tryConvertRomaji();
            }
            return;
        }
    }

    // Check if current romaji could be a prefix of a valid entry
    bool couldMatch = false;
    for (size_t i = 0; i < kRomajiTableSize; i++) {
        const char* pattern = kRomajiTable[i].romaji;
        if (String(pattern).startsWith(romaji)) {
            couldMatch = true;
            break;
        }
    }

    // If no possible match, flush the first character and retry
    if (!couldMatch && !romaji.isEmpty()) {
        accumulated += romaji[0];
        romaji = romaji.substring(1);
        if (!romaji.isEmpty()) {
            tryConvertRomaji();
        }
    }
}

void PromptInput::backspace() {
    // First remove from pending romaji
    if (!romaji.isEmpty()) {
        romaji.remove(romaji.length() - 1);
        return;
    }
    if (accumulated.isEmpty()) {
        return;
    }
    // Remove last UTF-8 character
    int idx = accumulated.length() - 1;
    while (idx > 0 && (accumulated[idx] & 0xC0) == 0x80) {
        idx--;
    }
    accumulated.remove(idx);
}

void PromptInput::clear() {
    accumulated.clear();
    romaji.clear();
}

void PromptInput::toggleMode() {
    // Flush pending romaji before switching
    if (!romaji.isEmpty()) {
        if (romaji == "n") {
            accumulated += "ん";
        } else {
            accumulated += romaji;
        }
        romaji = "";
    }
    japaneseMode = !japaneseMode;
}

const String& PromptInput::buffer() const {
    return accumulated;
}

const String& PromptInput::romajiPending() const {
    return romaji;
}

bool PromptInput::isJapaneseMode() const {
    return japaneseMode;
}

bool PromptInput::hasContent() const {
    return !accumulated.isEmpty() || !romaji.isEmpty();
}
