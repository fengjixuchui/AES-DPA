aes:
	gcc -std=c99 -Wall -O3 aes.c -o aes

aes-mask:
	gcc -std=c99 -Wall -O3 aes-masking.c -o aes-masking

dpa:
	python dpa.py traces-one.dat

clean:
	rm aes aes-masking
