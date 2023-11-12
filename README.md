# Excercises

- [1A MyCompress](#1a-mycompress)
- [1B Closest Pair](#1b-closest-pair)
- [contributing](#contributing)
- [license](#license)

---

# 1A MyCompress
## Introduction
mycompress.c is a C program developed by Ivan Cankov for compressing files. It is designed to read data from an input file, compress it, and write the compressed data to an output file.

## Usage
To use this program, execute it with the following command-line structure:

```bash
mycompress [-o outfile] [file...]
```

This allows you to specify an optional output file (outfile) and one or more input files to be compressed.

## How to Compile
To compile the program, use the provided Makefile with the following command:
```bash
make
```

## Main Functionality
The main function orchestrates the program's operation. It processes command-line arguments, opens and closes files, and manages error handling. The compression task is delegated to the compress function.

## Error Handling
The program robustly handles various errors, including:

- Multiple -o flags in the command line.
- Failure to open input or output files.
- Errors encountered during the compression process.

## Compression Logic
The compress function is the heart of the program. It reads the input file line by line, applies compression, and writes the output. It also carefully updates counters for the number of characters read and written, and manages end-of-line scenarios.

## Output
Upon successful execution, the program outputs the number of characters read and written. This information is displayed on stderr.

## Exit Codes
The program uses the following exit codes:

- `EXIT_SUCCESS` (0): Indicates successful execution.
- `EXIT_FAILURE` (1): Indicates an error occurred.

## Example Run
Here is an example of how to run the program:
```bash
./mycompress -o output.txt input.txt
```
This command will compress input.txt and write the compressed data to output.txt.


---



# 1B Closest Pair
## Overview
The Closest Pair Finder is a C program designed to efficiently find the closest pair of points from a given set. It utilizes advanced algorithms to handle large datasets with optimized performance.

## Features
- Efficient computation of the closest pair of points.
- Handling of large datasets.
- Error handling for malformed inputs and system calls.
## Requirements
- C Compiler (GCC recommended)
- Standard C Libraries
## Installation
1. Clone the repository.
2. Navigate to the project directory.
3. Compile the program using the provided Makefile:

```bash
make
```
## Usage
Run the program with the following command:
```bash
./cpair < input.txt
```
Or: 
```bash
echo -e "4 90\n4 89\n4 200\n4 -100" | ./cpair
```

Input the points in the following format, one pair per line:
```bash
x1 y1
x2 y2
...
```

The program will output the closest pair of points.

---

# Contributing
Contributions to the Closest Pair Finder are welcome. Please follow the standard GitHub pull request process to propose changes.

---

# License
This project is licensed under the MIT License.

---

# Author
Ivan Cankov e12219400@student.tuwien.ac.at

---

# Special Thanks
Creating the README.md: Christopher Scherling
