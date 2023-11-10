<div align="center">
<pre>
 ██████╗██████╗  █████╗ ██╗██████╗ 
██╔════╝██╔══██╗██╔══██╗██║██╔══██╗
██║     ██████╔╝███████║██║██████╔╝
██║     ██╔═══╝ ██╔══██║██║██╔══██╗
╚██████╗██║     ██║  ██║██║██║  ██║
 ╚═════╝╚═╝     ╚═╝  ╚═╝╚═╝╚═╝  ╚═╝
                                   
====================================

a simple closest pair algorithm
</pre>
</div>

## Compilation

```sh
make all
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
