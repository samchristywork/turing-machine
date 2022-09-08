![Banner](https://s-christy.com/status-banner-service/turing-machine/banner-slim.svg)

## Overview

You can find an explanation of what a Turing machine is [on
Wikipedia](https://en.wikipedia.org/wiki/Turing_machine). This program takes a
description of a Turing machine as a JSON file, and runs it, printing out the
state of the machine at each step and continuing until some condition has been
reached, such as an error state, a halting state, or a set number of steps have
been reached.

## Example

Here is an example of the program running a "busy beaver" example. Here is the
output when I run `turing_machine sample/busy_beaver.json`:

```
|                   h                 | A
|                   1h                | B
|                   h1                | A
|                  h11                | C
|                 h111                | B
|                h1111                | A
|                1h111                | B
|                11h11                | B
|                111h1                | B
|                1111h                | B
|                11111h               | B
|                1111h1               | A
|                111h11               | C
```

The input file looks like this:

```json
{
  "start_offset": 40,
  "states": [
    {"state": "A", "tape_symbol": " ", "write_symbol": "1", "direction": "R", "next_state": "B"},
    {"state": "A", "tape_symbol": "1", "write_symbol": "1", "direction": "L", "next_state": "C"},
    {"state": "B", "tape_symbol": " ", "write_symbol": "1", "direction": "L", "next_state": "A"},
    {"state": "B", "tape_symbol": "1", "write_symbol": "1", "direction": "R", "next_state": "B"},
    {"state": "C", "tape_symbol": " ", "write_symbol": "1", "direction": "L", "next_state": "B"},
    {"state": "C", "tape_symbol": "1", "write_symbol": "1", "direction": "R", "next_state": "HALT"}
  ]
}
```

## Usage

## Dependencies

## License

This work is licensed under the GNU General Public License version 3 (GPLv3).

[<img src="https://s-christy.com/status-banner-service/GPLv3_Logo.svg" width="150" />](https://www.gnu.org/licenses/gpl-3.0.en.html)
