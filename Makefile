FILES=main.c
CC=cc
CFLAGS=-lcairo -lfreetype -lzip `pkg-config --cflags freetype2 cairo`

main : $(FILES)
			$(CC) $(FILES) $(CFLAGS) -o scratchfont
			mv scratchfont /usr/local/bin/