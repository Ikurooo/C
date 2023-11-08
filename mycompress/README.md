<div align="center">
███╗   ███╗██╗   ██╗ ██████╗ ██████╗ ███╗   ███╗██████╗ ██████╗ ███████╗███████╗███████╗
████╗ ████║╚██╗ ██╔╝██╔════╝██╔═══██╗████╗ ████║██╔══██╗██╔══██╗██╔════╝██╔════╝██╔════╝
██╔████╔██║ ╚████╔╝ ██║     ██║   ██║██╔████╔██║██████╔╝██████╔╝█████╗  ███████╗███████╗
██║╚██╔╝██║  ╚██╔╝  ██║     ██║   ██║██║╚██╔╝██║██╔═══╝ ██╔══██╗██╔══╝  ╚════██║╚════██║
██║ ╚═╝ ██║   ██║   ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║     ██║  ██║███████╗███████║███████║
╚═╝     ╚═╝   ╚═╝    ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝

========================================================================================

a simple compression algorithm to compress files including stdin
</div>

## Compilation

```sh
gcc -o mycompress mycompress.c
# or
gcc -o mycompress chris.c
# or
gcc -o mycompress eliyazar.c
```

## Usage example

### To get help with commandline arguments

(have yet to implement it)

### Using Command-line Arguments

```sh
./mycompress [-o INPUT FILE] [OUTPUT FILES...]
# leave blank for stdin and stdout
```

### Examples

```sh
./mycompress.c infile1 infile2
```

(or)

```sh
echo -e "some text" | ./mycompress -o outfile
```

(or)

```sh
./mycompress -o outfile infile1 infile2
```

### Disclaimer

-o flag should always appear before any input files
