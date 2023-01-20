import re
import sys

_CAMEL_TO_SNAKE_0 = re.compile("(.)([A-Z][a-z]+)")
_CAMEL_TO_SNAKE_1 = re.compile("([a-z0-9])([A-Z])")
def camel_to_snake(name):
    name = _CAMEL_TO_SNAKE_0.sub(r"\1_\2", name)
    return _CAMEL_TO_SNAKE_1.sub(r"\1_\2", name).lower()

def eprint(*args, **kwargs):
    return print(*args, **kwargs, file=sys.stderr)
