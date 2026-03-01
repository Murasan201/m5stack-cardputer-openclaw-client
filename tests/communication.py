"""Helpers for composing events exchanged between Cardputer and OpenClaw."""
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Dict, Optional, Any


@dataclass(frozen=True)
class PromptEvent:
    type: str
    payload: Dict[str, Any]
    timestamp: str


@dataclass(frozen=True)
class ResponseEvent:
    text: str
    metadata: Dict[str, Any]
    timestamp: str


def _now_iso() -> str:
    return datetime.now(tz=timezone.utc).isoformat()


def build_prompt_event(text: str, language: str = "ja-JP") -> PromptEvent:
    payload = {
        "input": text,
        "language": language,
        "source": "cardputer",
    }
    return PromptEvent(type="prompt.submitted", payload=payload, timestamp=_now_iso())


def build_mode_event(mode: str, ready: bool) -> PromptEvent:
    payload = {"mode": mode, "ready": ready}
    return PromptEvent(type="mode.status", payload=payload, timestamp=_now_iso())


def build_response_event(text: str, metadata: Optional[Dict[str, Any]] = None) -> ResponseEvent:
    return ResponseEvent(text=text, metadata=metadata or {}, timestamp=_now_iso())
