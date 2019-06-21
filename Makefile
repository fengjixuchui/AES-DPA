.PHONY: aes
aes:
	gcc -std=c99 -Wall -O3 aes.c -o aes

.PHONY: aes-mask
aes-mask:
	gcc -std=c99 -Wall -O3 aes-mask.c -o aes-mask

.PHONY: dpa
dpa:
	python dpa.py traces-one.dat

.PHONY: clean
clean:
	rm -f aes aes-mask
