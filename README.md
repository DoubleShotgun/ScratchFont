# ScratchFont
A tool for converting font files (.ttf) to .sprite3 for making text engine in Scratch.

### Installing / Compiling
For Linux users, Just run `make`.
Windows users we'll have to compile the source code themselves.

### Usage
```
Usage:
   scratchfont [OPTIONS]... font.ttf font.sprite3

Options:
   -h     Print this page
   -n     To convert the numbers ([0-9])
   -l     To convert the letters ([A-Z] [a-z])
   -p     To convert the punctuations(.!?'"`:;)
   -o     To convert the other ASCII text(_@#$%&^|/\>=<()[]{})
   -w     Alias for -l
```

### Example
To better understand this tool, I recommend you to look at the example folder.
