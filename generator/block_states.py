#!/bin/env python3

import sys
import json
from typing import Dict, List, Union
from io import TextIOBase

class Property:
    id: int
    key: str
    kind: type
    values: Union[List[str], List[int], List[bool]]

class BlockState:
    block: 'Block'
    properties: Dict[str, str]
    id: int
    air: bool
    renderShape: str

class Block:
    name: str
    id: int
    translationKey: str
    defaultStateId: int
    properties: List[Property]
    states: List[BlockState]

def parse_props(data) -> Dict[str, Property]:
    props: dict = {}
    total_enum_values = 0
    for key, value in data.items():
        prop = Property()
        prop.id = len(props)
        prop.key = value['key']

        values: list = value['values']
        value_count = len(values)
        assert(value_count < 32)
        if isinstance(values[0], str):
            prop.kind = str
            prop.values = values
            total_enum_values += value_count
            for v in values:
                assert(isinstance(v, str))
        elif isinstance(values[0], bool):
            assert(value_count == 2)
            assert(values[0] is True)
            assert(values[1] is False)
            prop.kind = bool
            prop.values = [True, False]
        elif isinstance(values[0], int):
            assert(values[0] in [0, 1])
            for i in range(len(values)):
                assert(values[i] == values[0] + i)
            prop.kind = int
            prop.values = values
        else:
            raise TypeError()

        props[key] = prop
    assert(total_enum_values < 256)
    return props

def parse_blocks(data, props: Dict[str, Property]) -> List[Block]:
    blocks = []
    for key, value in data.items():
        block = Block()
        block.name = key
        block.id = value['id']
        block.translationKey = value['translationKey']
        block.defaultStateId = value['defaultStateId']
        block.properties = [props[name] for name in value['properties']]
        block.states = []
        for value in value['states']:
            state = BlockState()
            state.block = block
            state.properties = value['properties']
            state.id = value['stateId']
            state.air = value['air']
            state.renderShape = value['renderShape']
            block.states.append(state)
        blocks.append(block)
    return blocks

def write_cpp(w: TextIOBase, props: Dict[str, Property], blocks: List[Block]) -> None:
    w.write('#include "data/block_impl.hh"\n')
    w.write('\n')
    w.write('namespace mccpp::data::impl {\n')
    w.write(' namespace property {\n')
    w.write(f'  const size_t count = {len(props)};\n')
    w.write('  const uint16_t flags[] = {')
    value_offset = 0
    for prop in props.values():
        flags = 0
        assert(value_offset < 256)
        if prop.kind is str:
            flags |= 0x00
            flags |= value_offset << 8
            value_offset += len(prop.values)
        elif prop.kind is bool:
            flags |= 0x20
        elif prop.kind is int:
            if prop.values[0] == 0:
                flags |= 0x40
            else:
                flags |= 0x60
        else:
            raise ValueError()
        assert(len(prop.values) < 32)
        flags |= len(prop.values)
        w.write(f'0x{flags:04x},')
    w.write('};\n')
    w.write('  const char *const name[] = {')
    for name in props.keys():
        w.write(f'"{name}",')
    w.write('};\n')
    w.write('  const char *const key[] = {')
    for prop in props.values():
        w.write(f'"{prop.key}",')
    w.write('};\n')
    w.write('  const char *const values[] = {')
    for prop in props.values():
        if prop.kind is str:
            for value in prop.values:
                w.write(f'"{value}",')
    w.write('};\n')
    w.write(' }\n')
    w.write(' namespace block {\n')
    w.write(f'  const size_t count = {len(blocks)};\n')
    w.write('  const char *const name[] = {')
    for block in blocks:
        w.write(f'"{block.name}",')
    w.write('};\n')
    w.write('  const char *const translation_key[] = {')
    for block in blocks:
        w.write(f'"{block.translationKey}",')
    w.write('};\n')
    w.write('  const state_id first_state_id[] = {')
    for block in blocks:
        w.write(f'{block.states[0].id},')
    w.write('};\n')
    w.write('  const state_id default_state_id[] = {')
    for block in blocks:
        w.write(f'{block.defaultStateId},')
    w.write('};\n')
    prop_offset = 0
    w.write('  const uint16_t properties[] = {')
    for block in blocks:
        prop_count = len(block.properties)
        assert(prop_offset < 2048)
        assert(prop_count < 8)
        data = prop_offset | prop_count << 11
        w.write(f'0x{data:04x},')
        prop_offset += prop_count
    w.write('};\n')
    w.write('  const property_id property_map[] = {')
    for block in blocks:
        for prop in reversed(block.properties):
            w.write(f'0x{prop.id:02x},')
    w.write('};\n')
    w.write(' }\n')
    w.write(' namespace state {\n')
    w.write(f'  const size_t count = {len([s for b in blocks for s in b.states])};\n')
    w.write('  const block_id block[] = {')
    for block in blocks:
        for state in block.states:
            w.write(f'{block.id},')
    w.write('};\n')
    w.write(' }\n')
    w.write('}\n')

def main(argv) -> None:
    if len(argv) != 4:
        print(f"Usage: {argv[0]} blocks.json block_properties.json output.cc")
        exit(1)

    blocks_path = argv[1]
    props_path = argv[2]
    out_path = argv[3]

    with open(props_path, "r") as file:
        props = parse_props(json.load(file))

    with open(blocks_path, "r") as file:
        blocks = parse_blocks(json.load(file), props)

    with open(out_path, "w") as file:
        write_cpp(file, props, blocks)

if __name__ == '__main__':
    main(sys.argv)
