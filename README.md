# TMG-WALL

Generate lua table of base16 color from the picture it gets.

It integrate with [tmg](https://github.com/commrade-goad/tmg) just use the output
of this program as a template for `tmg` and use it to gen another template that
will feed again to `tmg`.

## Build

```sh
make all -j$(nproc)
```

## Usage

```sh
./tmg-wall [infile] [outfile]
```
