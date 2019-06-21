# Differential Power Analysis attack on AES

As part of the Applied Cryptography unit at the University of Bristol, I wrote an implementation of AES in C, implemented a Differential Power Analysis attack in Python that targets software AES implementations, and finally wrote an AES implementation in C that utilises various countermeasures to Differential Power Analysis attacks.

## About

The DPA attack exploits the sbox function of the first round of the AES encryption. Using the hamming weight bus leakage...

## Usage

### AES

Compile and run the AES implementation

```
$ make aes
$ ./aes "Hello, World?"
message    : 48 65 6c 6c 6f 2c 20 57 6f 72 6c 64 3f 00 00 00
key        : d4 96 e8 8f 21 40 55 92 ed 18 62 a9 8c 68 35 e6
ciphertext : 6b 82 1f 23 72 8a 31 f3 76 03 7f 10 5b 9b 29 65
```

### AES with countermeasures

Compile and run the AES implementation as before

```
$ make aes-mask
$ ./aes-mask "Hello, World?"
message    : 48 65 6c 6c 6f 2c 20 57 6f 72 6c 64 3f 00 00 00
key        : d4 96 e8 8f 21 40 55 92 ed 18 62 a9 8c 68 35 e6
ciphertext : 6b 82 1f 23 72 8a 31 f3 76 03 7f 10 5b 9b 29 65
```

### DPA

Run the DPA implementation, providing it with a trace file to attack

```
$ make dpa
python dpa.py traces-one.dat
Loading traces...
...
```

The Makefile provided runs the DPA against the `traces-one.dat` trace file. To run the DPA against another trace file, simply run it with Python directly

```
$ python dpa.py traces-two.dat
Loading traces...
...
```

Two sample trace files are provided. `traces-one.dat` contains a trace set recorded from the AES implementation with no countermeasures implemented (`aes.c`). `traces-two.dat` contains a trace set recorded from the software AES implementation that utilises masking (`aes-mask.c`).
