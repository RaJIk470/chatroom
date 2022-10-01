final:
		gcc -lncursesw -lformw myLib/based.c client.c -o client 
		gcc myLib/based.c server.c -o server 
