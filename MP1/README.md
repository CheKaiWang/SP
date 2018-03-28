all:mp1.c MD5.c md5.h list_file.c list_file.h
	gcc mp1.c MD5.c list_file.c -std=c99 -o loser
clean:
	rm -rf loser
