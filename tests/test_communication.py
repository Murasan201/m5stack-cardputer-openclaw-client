import pathlib
import sys

TEST_DIR = pathlib.Path(__file__).resolve().parent
if str(TEST_DIR) not in sys.path:
    sys.path.insert(0, str(TEST_DIR))

from communication import build_prompt_event, build_mode_event, build_response_event


def test_build_prompt_event_contains_input():
    text = "こんにちは"
    event = build_prompt_event(text)

    assert event.type == "prompt.submitted"
    assert event.payload["input"] == text
    assert event.payload["source"] == "cardputer"
    assert "language" in event.payload
    assert event.timestamp.endswith("Z") or event.timestamp.endswith("+00:00")


def test_build_mode_event_ready_flag():
    event = build_mode_event("prompt", True)
    assert event.type == "mode.status"
    assert event.payload["mode"] == "prompt"
    assert event.payload["ready"] is True


def test_response_event_metadata_defaults_to_empty_dict():
    response = build_response_event("応答テキスト")
    assert response.text == "応答テキスト"
    assert response.metadata == {}
