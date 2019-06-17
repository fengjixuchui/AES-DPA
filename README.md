# Differential Power Analysis attack on AES

As part of the Applied Cryptography unit at the University of Bristol, I
wrote an implementation of AES in C, implemented a DPA attack in Python
that targets software AES implementations, and finally wrote an AES
implementation in C that utilises various countermeasures to DPA attacks.

## About

The DPA attack exploits the sbox function of the first round of the AES
encryption. Using the hamming weight bus leakage...

## Usage

### AES

Compile and run the AES implementation

```
make aes
./aes "hello"
```

### AES with countermeasures

Compile and run the AES implementation as before

```
make aes-masking
./aes-masking "hello"
```

### DPA

Run the DPA implementation providing it with a trace file to attack

```
make dpa
```

A sample trace file is provided at ...
