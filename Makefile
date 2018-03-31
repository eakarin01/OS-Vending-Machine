all:
	gcc VendMach.c -pthread -o run
clean:
	rm run
