#!/usr/bin/env python3
"""Generate Yottasynth screen screenshots as SVG files.

The output intentionally mirrors the current LVGL firmware layout and value
formatting without requiring the Teensy target or a desktop LVGL backend.
"""

from __future__ import annotations

import argparse
import html
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, List, Tuple


SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240

ROOT_BG = "#09111F"
BAR_BG = "#111C2E"
CONTENT_BG = "#0D1628"
TAB_INACTIVE = "#1A2740"
TAB_TEXT_INACTIVE = "#C2D2EA"
TEXT_WHITE = "#FFFFFF"
TEXT_MUTED = "#D5E1F5"
TEXT_DISABLED = "#7C8AA5"
SEQUENCER_IDLE = "#1A2740"
SEQUENCER_ACTIVE = "#C2410C"
SEQUENCER_SELECTED = "#2563EB"
SEQUENCER_PLAYING = "#16A34A"

PAGE_ORDER = [
    "PLAY",
    "OSC",
    "FILT",
    "MOD",
    "FX",
    "ARP",
    "SEQ",
    "SET",
]

PAGE_COLORS = {
    "PLAY": "#2F855A",
    "OSC": "#0EA5A4",
    "FILT": "#D97706",
    "MOD": "#2563EB",
    "FX": "#7C3AED",
    "ARP": "#9333EA",
    "SEQ": "#DC2626",
    "SET": "#475569",
}

ACTION_LABELS = {
    "PLAY": ["RUN/STOP", "ARP TOG", "", "PANIC"],
    "OSC": ["W1 <", "W1 >", "W2 <", "W2 >"],
    "FILT": ["SUS -", "SUS +", "SNAP", "LONG"],
    "MOD": ["LFO OFF", "FILT LFO", "PITCH LFO", "DEPTH 0"],
    "FX": ["BYPASS", "ECHO", "REVERB", "DIRT"],
    "ARP": ["ENABLE", "LATCH", "RUN/STOP", "CLR HELD"],
    "SEQ": ["RUN/STOP", "REC ARM", "BANK", "CLEAR"],
    "SET": ["TEST PAGE", "", "", ""],
}

PAGE_TITLES = {
    "PLAY": "PLAY",
    "OSC": "OSC / MIX",
    "FILT": "FILTER / AMP",
    "MOD": "MOD",
    "FX": "FX",
    "ARP": "ARPEGGIATOR",
    "SEQ": "SEQUENCER",
    "SET": "SETTINGS",
}

OSC_WAVES = ["SAW", "SINE", "SQR", "TRI"]
LFO_TARGETS = ["OFF", "FILTER", "PITCH"]
ARP_MODES = ["UP", "DOWN", "UP/DOWN", "RANDOM"]
TUNINGS = [
    "Standard",
    "Shur",
    "Abuata",
    "Dashti",
    "Bayat-e Tork",
    "Afshari",
    "Segah",
    "Chahargah",
    "Homayun",
    "Bayat-e Esfahan",
    "Nava",
    "Mahur",
    "Rast-Panjgah",
]


@dataclass
class PatchState:
    osc1_mix: float = 0.85
    osc2_mix: float = 0.60
    osc1_wave: int = 0
    osc2_wave: int = 0
    noise_mix: float = 0.05
    octave_index: int = 2
    detune: float = 0.08
    cutoff: float = 0.72
    resonance: float = 0.20
    attack: float = 0.05
    decay: float = 0.22
    sustain: float = 0.72
    release: float = 0.24
    lfo_rate: float = 0.22
    lfo_depth: float = 0.12
    lfo_target: int = 1
    glide: float = 0.08
    bend_range: float = 0.25
    tuning: int = 0


@dataclass
class TransportState:
    bpm: int = 120
    running: bool = False
    swing: float = 0.0
    step_index: int = 0


@dataclass
class ArpState:
    enabled: bool = False
    latch: bool = False
    mode: int = 0
    octave_range: int = 1
    division: int = 1
    gate: int = 70


@dataclass
class EchoFxState:
    mix: float = 0.34
    time: float = 0.30
    feedback: float = 0.42
    ratio: float = 0.62
    smear: float = 0.18


@dataclass
class ReverbFxState:
    mix: float = 0.28
    size: float = 0.56
    damping: float = 0.42
    predelay: float = 0.12
    tone: float = 0.58


@dataclass
class DriveFxState:
    mix: float = 0.38
    drive: float = 0.44
    tone: float = 0.68
    crush: float = 0.20
    level: float = 0.60


@dataclass
class FxState:
    enabled: bool = False
    mode: int = 0
    echo: EchoFxState = field(default_factory=EchoFxState)
    reverb: ReverbFxState = field(default_factory=ReverbFxState)
    drive: DriveFxState = field(default_factory=DriveFxState)


@dataclass
class SequenceStep:
    active: bool
    tie: bool
    note: int
    gate: int


@dataclass
class SequencerState:
    enabled: bool = False
    record_armed: bool = False
    length: int = 16
    playhead: int = 0
    selected_step: int = 0
    visible_bank: int = 0
    steps: List[SequenceStep] = field(default_factory=list)


@dataclass
class AudioStatus:
    output_volume: float = 0.50


@dataclass
class AppState:
    page: str
    patch: PatchState = field(default_factory=PatchState)
    transport: TransportState = field(default_factory=TransportState)
    arp: ArpState = field(default_factory=ArpState)
    fx: FxState = field(default_factory=FxState)
    sequencer: SequencerState = field(default_factory=lambda: SequencerState(steps=default_sequence()))
    audio: AudioStatus = field(default_factory=AudioStatus)
    confirm_clear_sequence: bool = False


def default_sequence() -> List[SequenceStep]:
    return [
        SequenceStep(True, False, 60, 78),
        SequenceStep(True, False, 62, 72),
        SequenceStep(True, False, 67, 82),
        SequenceStep(False, False, 67, 70),
        SequenceStep(True, False, 60, 78),
        SequenceStep(True, False, 64, 72),
        SequenceStep(True, False, 69, 82),
        SequenceStep(False, False, 69, 70),
        SequenceStep(True, False, 60, 78),
        SequenceStep(True, False, 62, 72),
        SequenceStep(True, False, 67, 82),
        SequenceStep(False, False, 67, 70),
        SequenceStep(True, False, 72, 78),
        SequenceStep(True, False, 69, 72),
        SequenceStep(True, False, 64, 82),
        SequenceStep(False, False, 60, 70),
    ]


def clamp(value: float, low: float, high: float) -> float:
    return max(low, min(high, value))


def note_name(note: int) -> str:
    names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    octave = note // 12 - 1
    return f"{names[note % 12]}{octave}"


def yes_no(value: bool) -> str:
    return "ON" if value else "OFF"


def ms_from_normalized(normalized: float, min_ms: float, max_ms: float) -> float:
    return min_ms + clamp(normalized, 0.0, 1.0) * (max_ms - min_ms)


def percent(value: float) -> str:
    return f"{round(value * 100):d}%"


def tuning_label(index: int) -> str:
    return TUNINGS[index]


def format_status(state: AppState) -> str:
    if state.page == "SET":
        return f"VOL {round(state.audio.output_volume * 100):d}%  TUNE {tuning_label(state.patch.tuning)}"
    if state.page == "OSC":
        return (
            f"W1 {OSC_WAVES[state.patch.osc1_wave]}  "
            f"W2 {OSC_WAVES[state.patch.osc2_wave]}  "
            f"N {round(state.patch.noise_mix * 100):d}%"
        )
    if state.page == "FX":
        return f"FX {['ECHO', 'REVERB', 'DRIVE'][state.fx.mode]} {yes_no(state.fx.enabled)}"
    return f"{state.transport.bpm}BPM {'RUN' if state.transport.running else 'STOP'} A:{yes_no(state.arp.enabled)} S:{yes_no(state.sequencer.enabled)}"


def fx_pot_names(mode: int) -> List[str]:
    if mode == 0:
        return ["MIX", "TIME", "FDBK", "RATIO", "SMEAR"]
    if mode == 1:
        return ["MIX", "SIZE", "DAMP", "PRE", "TONE"]
    return ["MIX", "DRIVE", "TONE", "CRUSH", "LEVEL"]


def format_fx_pot_value(state: AppState, index: int) -> str:
    mode = state.fx.mode
    if mode == 0:
        if index == 0:
            return percent(state.fx.echo.mix)
        if index == 1:
            return f"{round(ms_from_normalized(state.fx.echo.time, 50.0, 360.0)):d}ms"
        if index == 2:
            return percent(state.fx.echo.feedback)
        if index == 3:
            return f"{round((0.45 + state.fx.echo.ratio * 0.50) * 100):d}%"
        return percent(state.fx.echo.smear)
    if mode == 1:
        if index == 0:
            return percent(state.fx.reverb.mix)
        if index == 1:
            return percent(state.fx.reverb.size)
        if index == 2:
            return percent(state.fx.reverb.damping)
        if index == 3:
            return f"{round(ms_from_normalized(state.fx.reverb.predelay, 8.0, 150.0)):d}ms"
        return percent(state.fx.reverb.tone)
    if index == 0:
        return percent(state.fx.drive.mix)
    if index == 1:
        return f"x{1.0 + state.fx.drive.drive * 13.0:.1f}"
    if index == 2:
        return f"{4.5 + state.fx.drive.tone * 18.0:.1f}k"
    if index == 3:
        return f"{round(16 - state.fx.drive.crush * 10.0):d}bit"
    return percent(state.fx.drive.level)


def pot_names(state: AppState) -> List[str]:
    if state.page == "PLAY":
        return ["CUT", "RES", "GLIDE", "A-GATE", "BPM"]
    if state.page == "OSC":
        return ["OSC1", "OSC2", "NOISE", "OCT", "DETUNE"]
    if state.page == "FILT":
        return ["CUT", "RES", "ATT", "DEC", "REL"]
    if state.page == "MOD":
        return ["RATE", "DEPTH", "GLIDE", "BEND", "TARGET"]
    if state.page == "FX":
        return fx_pot_names(state.fx.mode)
    if state.page == "ARP":
        return ["BPM", "DIV", "GATE", "OCT", "MODE"]
    if state.page == "SEQ":
        return ["BPM", "LEN", "SWING", "NOTE", "GATE"]
    return ["VOL", "TUNE", "", "", ""]


def pot_values(state: AppState) -> List[str]:
    if state.page == "PLAY":
        return [
            f"{round(state.patch.cutoff * 10000):d} Hz",
            f"{0.7 + state.patch.resonance * 4.3:.1f}",
            percent(state.patch.glide),
            f"{state.arp.gate:d}%",
            f"{state.transport.bpm:d} BPM",
        ]
    if state.page == "OSC":
        return [
            percent(state.patch.osc1_mix),
            percent(state.patch.osc2_mix),
            percent(state.patch.noise_mix),
            f"{(state.patch.octave_index - 2) * 12:+d} st",
            f"{0.95 + ((state.patch.detune - 0.5) * 0.10):.2f}",
        ]
    if state.page == "FILT":
        return [
            percent(state.patch.cutoff),
            percent(state.patch.resonance),
            f"{round(5.0 + state.patch.attack * 2500.0):d}ms",
            f"{round(20.0 + state.patch.decay * 3000.0):d}ms",
            f"{round(30.0 + state.patch.release * 3200.0):d}ms",
        ]
    if state.page == "MOD":
        return [
            f"{0.15 + state.patch.lfo_rate * 10.0:.1f} Hz",
            percent(state.patch.lfo_depth),
            percent(state.patch.glide),
            f"{1.0 + state.patch.bend_range * 11.0:.1f} st",
            LFO_TARGETS[state.patch.lfo_target],
        ]
    if state.page == "FX":
        return [format_fx_pot_value(state, idx) for idx in range(5)]
    if state.page == "ARP":
        return [
            f"{state.transport.bpm:d} BPM",
            f"x{state.arp.division:d}",
            f"{state.arp.gate:d}%",
            f"{state.arp.octave_range:d} oct",
            ARP_MODES[state.arp.mode],
        ]
    if state.page == "SEQ":
        step = state.sequencer.steps[state.sequencer.selected_step]
        return [
            f"{state.transport.bpm:d} BPM",
            f"{state.sequencer.length:d} steps",
            percent(state.transport.swing),
            note_name(step.note),
            f"{step.gate:d}%",
        ]
    return [
        percent(state.audio.output_volume),
        tuning_label(state.patch.tuning),
        "",
        "",
        "",
    ]


def page_filename(page: str) -> str:
    return {
        "PLAY": "play.svg",
        "OSC": "osc.svg",
        "FILT": "filt.svg",
        "MOD": "mod.svg",
        "FX": "fx.svg",
        "ARP": "arp.svg",
        "SEQ": "seq.svg",
        "SET": "set.svg",
    }[page]


def page_title(page: str) -> str:
    return PAGE_TITLES[page]


def svg_text(x: float, y: float, text: str, size: int, color: str, anchor: str = "start",
             weight: int = 500, family: str = "Arial, Helvetica, sans-serif") -> str:
    escaped = html.escape(text)
    return (
        f'<text x="{x}" y="{y}" fill="{color}" font-size="{size}" '
        f'font-family="{family}" font-weight="{weight}" text-anchor="{anchor}">{escaped}</text>'
    )


def svg_rect(x: float, y: float, width: float, height: float, fill: str,
             radius: float = 0.0, opacity: float | None = None,
             stroke: str | None = None, stroke_width: float = 0.0) -> str:
    attrs = [
        f'x="{x}"',
        f'y="{y}"',
        f'width="{width}"',
        f'height="{height}"',
        f'fill="{fill}"',
    ]
    if radius:
        attrs.append(f'rx="{radius}"')
        attrs.append(f'ry="{radius}"')
    if opacity is not None:
        attrs.append(f'fill-opacity="{opacity:.3f}"')
    if stroke:
        attrs.append(f'stroke="{stroke}"')
    if stroke_width:
        attrs.append(f'stroke-width="{stroke_width}"')
    return f"<rect {' '.join(attrs)} />"


def svg_border_rect(x: float, y: float, width: float, height: float, stroke: str,
                    stroke_width: float = 1.0, radius: float = 0.0) -> str:
    attrs = [
        f'x="{x}"',
        f'y="{y}"',
        f'width="{width}"',
        f'height="{height}"',
        'fill="none"',
        f'stroke="{stroke}"',
        f'stroke-width="{stroke_width}"',
    ]
    if radius:
        attrs.append(f'rx="{radius}"')
        attrs.append(f'ry="{radius}"')
    return f"<rect {' '.join(attrs)} />"


def wrap_svg(body: Iterable[str], scale: int) -> str:
    width = SCREEN_WIDTH * scale
    height = SCREEN_HEIGHT * scale
    return "\n".join(
        [
            '<?xml version="1.0" encoding="UTF-8"?>',
            (
                f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
                f'viewBox="0 0 {SCREEN_WIDTH} {SCREEN_HEIGHT}">'
            ),
            *body,
            "</svg>",
        ]
    )


def wrap_custom_svg(body: Iterable[str], width: int, height: int, view_box: str) -> str:
    return "\n".join(
        [
            '<?xml version="1.0" encoding="UTF-8"?>',
            (
                f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
                f'viewBox="{view_box}">'
            ),
            *body,
            "</svg>",
        ]
    )


def render_tabs(active_page: str) -> List[str]:
    body: List[str] = [svg_rect(0, 196, 320, 44, BAR_BG)]
    for index, label in enumerate(PAGE_ORDER):
        x = 8 + index * 38
        active = label == active_page
        body.append(svg_rect(x, 6 + 196, 36, 32, PAGE_COLORS[active_page] if active else TAB_INACTIVE, 7))
        body.append(svg_text(x + 18, 26 + 196, label, 9, TEXT_WHITE if active else TAB_TEXT_INACTIVE, anchor="middle", weight=700))
    return body


def render_top_bar(state: AppState) -> List[str]:
    return [
        svg_rect(0, 0, 320, 46, BAR_BG),
        svg_text(10, 16, page_title(state.page), 14, TEXT_WHITE, weight=700),
        svg_text(10, 36, format_status(state), 10, TEXT_MUTED, weight=500),
    ]


def render_pot_cards(state: AppState) -> List[str]:
    accent = PAGE_COLORS[state.page]
    names = pot_names(state)
    values = pot_values(state)
    cards: List[Tuple[float, float, float, float, int]] = []
    if state.page == "SET":
        cards = [(0, 0, 68, 56, 0), (72, 0, 236, 56, 1)]
    else:
        cards = [(index * 64, 0, 60, 56, index) for index in range(5)]

    body = [svg_rect(0, 46, 320, 150, CONTENT_BG), svg_rect(-9, 54, 316, 56, "#000000", opacity=0.0)]
    for x, y, width, height, index in cards:
        name = names[index]
        active = bool(name)
        body.append(svg_rect(x, 54 + y, width, height, accent, radius=12, opacity=0.30 if active else 0.10))
        body.append(svg_text(x + width / 2, 72 + y, name, 10, "#E5EDF8" if active else TEXT_DISABLED, anchor="middle", weight=700))
        body.append(svg_text(x + width / 2, 101 + y, values[index], 10, TEXT_WHITE if active else TEXT_DISABLED, anchor="middle", weight=600))
    return body


def render_action_buttons(state: AppState) -> List[str]:
    accent = PAGE_COLORS[state.page]
    labels = ACTION_LABELS[state.page][:]
    if state.page == "SEQ" and state.confirm_clear_sequence:
        labels[3] = "WAIT OK"

    body: List[str] = []
    for index, label in enumerate(labels):
        if state.page == "SET" and index > 0:
            continue
        if state.page == "SEQ":
            x = 10 + index * 76
            y = 156
            width = 72
            height = 24
            text_size = 8
        else:
            x = 153 if index % 2 else 6
            y = 148 if index >= 2 else 116
            width = 145
            height = 26
            text_size = 10
        body.append(svg_rect(x, y, width, height, accent, radius=8, opacity=0.40))
        body.append(svg_text(x + width / 2, y + 16, label, text_size, TEXT_WHITE, anchor="middle", weight=700))
    return body


def render_sequencer(state: AppState) -> List[str]:
    body = [svg_rect(0, 46, 320, 150, CONTENT_BG)]
    step = state.sequencer.steps[state.sequencer.selected_step]
    info = f"STEP {state.sequencer.selected_step + 1:02d} {note_name(step.note)} G{step.gate} B{state.sequencer.visible_bank + 1}"
    body.append(svg_text(6, 58, info, 10, TEXT_MUTED, weight=500))

    for index in range(8):
        step_index = state.sequencer.visible_bank * 8 + index
        col = index % 4
        row = index // 4
        x = 6 + col * 76
        y = 80 + row * 36
        step_state = state.sequencer.steps[step_index]
        selected = step_index == state.sequencer.selected_step
        playing = step_index == state.sequencer.playhead and state.transport.running
        if playing:
            color = SEQUENCER_PLAYING
        elif selected:
            color = SEQUENCER_SELECTED
        elif step_state.active:
            color = SEQUENCER_ACTIVE
        else:
            color = SEQUENCER_IDLE
        label = f"{step_index + 1:02d} {note_name(step_state.note) if step_state.active else '--'}"
        body.append(svg_rect(x, y, 72, 34, color, radius=8))
        body.append(svg_text(x + 36, y + 21, label, 10, TEXT_WHITE, anchor="middle", weight=700))
    return body


def render_page_body(state: AppState) -> List[str]:
    body: List[str] = [svg_rect(0, 0, 320, 240, ROOT_BG)]
    body.extend(render_top_bar(state))
    if state.page == "SEQ":
        body.extend(render_sequencer(state))
    else:
        body.extend(render_pot_cards(state))
    body.extend(render_action_buttons(state))
    body.extend(render_tabs(state.page))
    body.append(svg_border_rect(0.5, 0.5, 319, 239, "#273449", stroke_width=1))
    return body


def render_page(state: AppState, scale: int) -> str:
    body = render_page_body(state)
    return wrap_svg(body, scale)


def render_overview(scale: int) -> str:
    columns = 2
    gap = 24
    padding = 24
    title_height = 28
    card_padding = 10
    label_height = 20
    card_width = SCREEN_WIDTH + card_padding * 2
    card_height = SCREEN_HEIGHT + card_padding * 2 + label_height
    width = padding * 2 + columns * card_width + gap
    rows = (len(PAGE_ORDER) + columns - 1) // columns
    height = padding * 2 + title_height + rows * card_height + (rows - 1) * gap

    body: List[str] = [svg_rect(0, 0, width, height, "#0B1220")]
    body.append(svg_text(padding, padding + 18, "Yottasynth Interface Overview", 18, TEXT_WHITE, weight=700))
    body.append(
        svg_text(
            padding,
            padding + 40,
            "Generated UI pages from the current firmware layout",
            11,
            TEXT_MUTED,
            weight=500,
        )
    )

    start_y = padding + title_height + 16
    for index, page in enumerate(PAGE_ORDER):
        column = index % columns
        row = index // columns
        x = padding + column * (card_width + gap)
        y = start_y + row * (card_height + gap)
        body.append(svg_rect(x, y, card_width, card_height, "#121B2D", radius=14, stroke="#273449", stroke_width=1))
        body.append(svg_rect(x + card_padding, y + card_padding, SCREEN_WIDTH, SCREEN_HEIGHT, "#09111F", radius=10))
        card_body = "\n".join(render_page_body(sample_state(page)))
        body.append(f'<g transform="translate({x + card_padding},{y + card_padding})">{card_body}</g>')
        body.append(
            svg_text(
                x + card_width / 2,
                y + card_padding + SCREEN_HEIGHT + 16,
                page_title(page),
                11,
                TEXT_MUTED,
                anchor="middle",
                weight=700,
            )
        )

    return wrap_custom_svg(body, width * scale, height * scale, f"0 0 {width} {height}")


def sample_state(page: str) -> AppState:
    state = AppState(page=page)
    if page == "PLAY":
        state.transport.running = True
        state.arp.enabled = True
        state.sequencer.enabled = False
        state.patch.cutoff = 0.81
        state.patch.resonance = 0.34
        state.patch.glide = 0.18
        state.arp.gate = 78
        state.transport.bpm = 126
    elif page == "OSC":
        state.patch.osc1_wave = 0
        state.patch.osc2_wave = 2
        state.patch.osc1_mix = 0.92
        state.patch.osc2_mix = 0.54
        state.patch.noise_mix = 0.12
        state.patch.octave_index = 3
        state.patch.detune = 0.62
    elif page == "FILT":
        state.patch.cutoff = 0.58
        state.patch.resonance = 0.47
        state.patch.attack = 0.12
        state.patch.decay = 0.28
        state.patch.release = 0.36
        state.patch.sustain = 0.64
    elif page == "MOD":
        state.patch.lfo_rate = 0.48
        state.patch.lfo_depth = 0.41
        state.patch.glide = 0.11
        state.patch.bend_range = 0.55
        state.patch.lfo_target = 2
    elif page == "FX":
        state.fx.enabled = True
        state.fx.mode = 1
        state.fx.reverb.mix = 0.35
        state.fx.reverb.size = 0.68
        state.fx.reverb.damping = 0.31
        state.fx.reverb.predelay = 0.26
        state.fx.reverb.tone = 0.73
    elif page == "ARP":
        state.transport.running = True
        state.arp.enabled = True
        state.arp.latch = True
        state.arp.division = 2
        state.arp.gate = 64
        state.arp.octave_range = 2
        state.arp.mode = 2
        state.transport.bpm = 132
    elif page == "SEQ":
        state.transport.running = True
        state.sequencer.enabled = True
        state.sequencer.record_armed = True
        state.transport.bpm = 118
        state.transport.swing = 0.18
        state.sequencer.length = 12
        state.sequencer.selected_step = 5
        state.sequencer.playhead = 6
        state.sequencer.visible_bank = 0
    elif page == "SET":
        state.audio.output_volume = 0.62
        state.patch.tuning = 1
    return state


def build_index(output_dir: Path) -> None:
    cards = []
    for page in PAGE_ORDER:
        filename = page_filename(page)
        cards.append(
            "\n".join(
                [
                    '<div class="card">',
                    f"<h2>{html.escape(page_title(page))}</h2>",
                    f'<img src="{filename}" alt="{html.escape(page_title(page))}" />',
                    "</div>",
                ]
            )
        )
    html_text = "\n".join(
        [
            "<!doctype html>",
            "<html><head><meta charset=\"utf-8\">",
            "<title>Yottasynth Screenshots</title>",
            "<style>",
            "body{font-family:Arial,Helvetica,sans-serif;background:#0b1220;color:#fff;margin:24px;}",
            "h1{font-size:28px;margin:0 0 16px;}",
            ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(340px,1fr));gap:20px;}",
            ".card{background:#121b2d;border:1px solid #273449;border-radius:14px;padding:16px;}",
            ".card h2{font-size:16px;margin:0 0 12px;color:#d5e1f5;}",
            "img{width:100%;height:auto;display:block;background:#09111f;border-radius:10px;}",
            "</style></head><body>",
            "<h1>Yottasynth Screenshots</h1>",
            "<div class=\"grid\">",
            *cards,
            "</div></body></html>",
        ]
    )
    (output_dir / "index.html").write_text(html_text, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate Yottasynth page screenshots.")
    parser.add_argument(
        "--output-dir",
        default="docs/screenshots",
        help="Directory where the generated SVG files will be written.",
    )
    parser.add_argument(
        "--scale",
        type=int,
        default=4,
        help="SVG export scale factor applied to the 320x240 device viewBox.",
    )
    args = parser.parse_args()

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    for page in PAGE_ORDER:
        svg = render_page(sample_state(page), max(1, args.scale))
        (output_dir / page_filename(page)).write_text(svg, encoding="utf-8")

    overview_svg = render_overview(max(1, args.scale))
    (output_dir / "overview.svg").write_text(overview_svg, encoding="utf-8")

    build_index(output_dir)
    print(f"Generated {len(PAGE_ORDER)} page screenshots plus overview.svg in {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
