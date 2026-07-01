# Tests

Host-side unit tests that run off-device, so the pure logic can be checked
without flashing a board.

## Run

```sh
cd test
make test
```

`RpnSolver` is pure logic and only needs `String` and `isDigit()` from the
Arduino core; both are provided by the minimal shim in `arduino/`. No board,
no external library, no framework.

Covered: every RPN operator from the readme grammar table, multi-digit
literals, `@id` references, the malformed-equation cases of `check()`, and the
divide-by-zero guard.
