SRCDIR = ./
CFILELIST = quash.c

quashCompile: quash.c
	gcc quash.c -o quash

quash: quashCompile
	./quash

clean:
	rm -f quash