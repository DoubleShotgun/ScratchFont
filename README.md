# ScratchFont
A tool for converting font files (.ttf) to .sprite3 for making text engine in Scratch.

### Installing / Compiling
For Linux users:
	First run `sudo apt-get install libcairo2-dev libfreetype-dev libzip-dev`. <br />
	Then just run `make`.

Windows users will have Windows Subsystem for Linux to compile and use the code.

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

### Todo
Alignment options. <br />
Add a list with character width. <br />
A website version.