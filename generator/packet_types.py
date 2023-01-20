# Based on the data at https://github.com/Articdive/ArticData
# TODO: Look into https://github.com/Articdive/ArticDataGenerator

import json
import re
import os
from pathlib import PurePath
from typing import Callable, Dict, List, TextIO
from functools import partial

from utils import camel_to_snake, eprint

_INDENT = " "

class Direction:
    def __init__(self, name: str):
        self.upper = name.upper()
        self.lower = name.lower()
        self.title = name.title()

class Packet:
    def __init__(self, name: str, id: int, direction: Direction, state: 'State'):
        self.name_original = name
        self.name_class = camel_to_snake(name.removeprefix(direction.title))
        self.id = id
        self.direction = direction
        self.state = state
        self.name_full = f"{direction.lower}::{self.state.lower}::{self.name_class}"

class State:
    def __init__(self, name: str, id: int):
        self.upper = name.upper()
        self.lower = name.lower()
        self.id = id
        self.packets: Dict[Direction, Packet] = {}

class Context:
    def __init__(self, data):
        self.directions: List[Direction] = [Direction("SERVERBOUND"),
                                            Direction("CLIENTBOUND")]
        self.states: List[State] = []
        for (name, data) in data.items():
            state = State(name, data["id"])
            self.states.append(state)
            for direction in self.directions:
                packets = state.packets[direction] = []
                for packet in data[direction.upper]:
                    packets.append(Packet(packet["name"], packet["id"], direction, state))

    def write_directions(self, stream: TextIO, indent = 0) -> None:
        indent = _INDENT * indent
        stream.write("\n")
        stream.write(indent + "enum class packet_direction {\n")
        for direction in self.directions:
            stream.write(f"{indent}{_INDENT}{direction.upper},\n")
        stream.write(indent + "};\n")

    def write_states(self, stream: TextIO, indent = 0) -> None:
        indent = _INDENT * indent
        stream.write("\n")
        stream.write(indent + "enum class connection_state {\n")
        for state in self.states:
            stream.write(f"{indent}{_INDENT}{state.upper} = {state.id},\n")
        stream.write(indent + "};\n")

    def write_formatters(self, stream: TextIO, namespace: str, indent = 0):
        indent0 = _INDENT * indent
        indent1 = indent0 + _INDENT * 1
        indent2 = indent0 + _INDENT * 2

        formatter_for = {
            "packet_direction": self.directions,
            "connection_state": self.states,
        }

        for (name, values) in formatter_for.items():
            stream.write("\n")
            stream.write(f"{indent0}template<>\n")
            stream.write(f"{indent0}struct fmt::formatter<{namespace}::{name}> : public fmt::formatter<string_view> {{\n")
            stream.write(f"{indent1}using type = {namespace}::{name};\n")
            stream.write(f"{indent1}template<typename FormatContext>\n")
            stream.write(f"{indent1}auto format(const type &value, FormatContext &ctx) const -> decltype(ctx.out()) {{\n")
            stream.write(f"{indent2}switch (value) {{\n")
            for value in values:
                stream.write(f'{indent2}case type::{value.upper}: return formatter<string_view>::format("{value.lower}", ctx);\n')
            stream.write(f"{indent2}}}\n")
            stream.write(f'{indent2}throw format_error("invalid {name}");\n')
            stream.write(f"{indent1}}}\n")
            stream.write(f"{indent0}}};\n")

    def write_types(self, stream: TextIO, direction: Direction, indent = 0) -> None:
        indent = _INDENT * indent

        stream.write(f"\n{indent}namespace {direction.lower} {{\n")
        for state in self.states:
            stream.write(f"{indent}{_INDENT}namespace {state.lower} {{\n")
            for packet in state.packets[direction]:
                stream.write(f"{indent}{_INDENT*2}struct {packet.name_class} {{ static constexpr int32_t id = {packet.id}; }};\n")
            stream.write(indent + _INDENT + "}\n")
        stream.write("}\n")

    def write_traits(self, stream: TextIO, direction: Direction, indent = 0) -> None:
        indent = _INDENT * indent
        for state in self.states:
            packets = state.packets[direction]
            for packet in packets:
                stream.write(f'\n')
                stream.write(f'{indent}template<>\n')
                stream.write(f'{indent}struct packet_traits<{packet.name_full}> {{\n')
                stream.write(f'{indent}{_INDENT}static constexpr auto name = "{packet.name_original}";\n')
                stream.write(f'{indent}{_INDENT}static constexpr auto direction = packet_direction::{direction.upper};\n')
                stream.write(f'{indent}{_INDENT}static constexpr auto state = connection_state::{state.upper};\n')
                stream.write(f'{indent}}};\n')

    def write_iterators(self, stream: TextIO, direction: Direction) -> None:
        for state in self.states:
            packets = state.packets[direction]
            stream.write(f"\n#define MCCPP_ITERATE_{direction.upper}_{state.upper}")
            for packet in packets:
                stream.write(f" \\\n{_INDENT}_MCCPP_ITER_PACKET({packet.name_full})")
            stream.write("\n")

        stream.write(f"\n#define MCCPP_ITERATE_{direction.upper}")
        for state in self.states:
            stream.write(f" \\\n{_INDENT}_MCCPP_ITER_STATE_START({state.upper})")
            stream.write(f" \\\n{_INDENT*2}MCCPP_ITERATE_{direction.upper}_{state.upper}")
            stream.write(f" \\\n{_INDENT}_MCCPP_ITER_STATE_END({state.upper})")
        stream.write("\n")

def write_header(argv0: str, stream: TextIO) -> None:
    stream.write(f"// Automatically generated by {argv0}\n")
    stream.write("#pragma once\n")

def write_namespace_start(stream: TextIO) -> None:
    stream.write("\nnamespace mccpp::proto::generated {\n")

def write_namespace_end(stream: TextIO) -> None:
    stream.write("\n}\n")

def main(argv: List[str]) -> int:
    if len(argv) != 3:
        eprint(f"Usage: {argv[0]} packets.json out_dir")
        return 1

    argv0 = argv[0]
    json_path = PurePath(argv[1])
    out_path = PurePath(argv[2])

    with open(json_path, "r") as file:
        data = json.load(file)

    ctx = Context(data)

    def _write_misc_hh(stream: TextIO) -> None:
        write_header(argv0, stream)
        write_namespace_start(stream)
        ctx.write_directions(stream)
        ctx.write_states(stream)
        stream.write("\n")
        stream.write("template<typename T>\n")
        stream.write("struct packet_traits;\n")
        write_namespace_end(stream)

    def _write_format_hh(stream: TextIO) -> None:
        write_header(argv0, stream)
        stream.write("\n")
        stream.write("#include <fmt/format.h>\n")
        stream.write('#include "misc.hh"\n')
        ctx.write_formatters(stream, "mccpp::proto::generated")

    def _write_types_hh(direction: Direction, stream: TextIO) -> None:
        write_header(argv0, stream)
        stream.write("\n")
        stream.write("#include <cstdint>\n")
        write_namespace_start(stream)
        ctx.write_types(stream, direction)
        write_namespace_end(stream)

    def _write_traits_hh(direction: Direction, stream: TextIO) -> None:
        write_header(argv0, stream)
        stream.write("\n")
        stream.write('#include "../misc.hh"\n')
        stream.write('#include "types.hh"\n')
        write_namespace_start(stream)
        ctx.write_traits(stream, direction)
        write_namespace_end(stream)

    def _write_iterators_hh(direction: Direction, stream: TextIO) -> None:
        write_header(argv0, stream)
        stream.write("\n")
        stream.write('#include "types.hh"\n')
        ctx.write_iterators(stream, direction)

    files: Dict[str, Callable[None, [TextIO]]] = {
        "misc.hh": _write_misc_hh,
        "format.hh": _write_format_hh,
    }

    for direction in ctx.directions:
        d = PurePath(direction.lower)
        files[d / "types.hh"] = partial(_write_types_hh, direction)
        files[d / "traits.hh"] = partial(_write_traits_hh, direction)
        files[d / "iterators.hh"] = partial(_write_iterators_hh, direction)

    for (path, writer) in files.items():
        full_path = out_path / path
        os.makedirs(full_path.parent, exist_ok = True)
        with open(full_path, "w") as stream:
            writer(stream)

    return 0

if __name__ == '__main__':
    import sys
    exit(main(sys.argv))
