.PHONY: aes
aes:
	gcc -std=c99 -Wall -O3 aes.c -o aes

.PHONY: aes-mask
aes-mask:
	gcc -std=c99 -Wall -O3 aes-masking.c -o aes-masking

.PHONY: dpa
dpa:
	python dpa.py traces-one.dat

.PHONY: clean
clean:
	rm aes aes-masking
