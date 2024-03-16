main: main.c
	gcc main.c -o main
	./main

copy: maincopy.c
	gcc maincopy.c -o maincopy
	./maincopy

clean:
	rm -rf ./main