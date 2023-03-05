#!/bin/env python3

# Usage: regenerate.py source_files... timestamp_file -- command args...

# TODO: Maybe add checking for command change
# Write command separated by NUL to timestamp_file

import os
from os import path
import sys
from pathlib import Path
import subprocess
from typing import List
from dataclasses import dataclass
import re

TIMESTAMP_NAME_REGEX = re.compile(r"^[A-Z_]+$")

@dataclass
class Args:
    arg0: str
    source_files: List[str]
    timestamp_file: str
    command: List[str]

def parse_args(argv: List[str]) -> Args:
    arg0 = argv[0]
    argv = argv[1:]
    sep_index = argv.index("--")
    files = argv[:sep_index]
    source_files = files[:-1]
    timestamp_file = files[-1]
    command = argv[sep_index+1:]
    return Args(arg0, source_files, timestamp_file, command)

def is_out_of_date(timestamp_file: str, source_files: List[str]) -> bool:
    if not path.exists(timestamp_file):
        return True
    timestamp = path.getmtime(timestamp_file)
    for source_file in source_files:
        if path.getmtime(source_file) > timestamp:
            return True
    return False

def main(argv) -> None:
    args = parse_args(argv)
    # Require all caps to prevent mistakes from happening
    if not TIMESTAMP_NAME_REGEX.match(path.basename(args.timestamp_file)):
        print(f"Unacceptable timestamp file name {args.timestamp_file!r}:", file=sys.stderr)
        print("Basename can only contains uppercase letters and underscores", file=sys.stderr)
        exit(1)
    if is_out_of_date(args.timestamp_file, args.source_files):
        process = subprocess.run(args.command)
        if process.returncode != 0:
            exit(process.returncode)
        Path(args.timestamp_file).touch()

if __name__ == '__main__':
    main(sys.argv)
