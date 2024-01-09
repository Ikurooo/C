<div align="center">
<pre>
██╗  ██╗████████╗████████╗██████╗      ██████╗██╗     ██╗███████╗███╗   ██╗████████╗
██║  ██║╚══██╔══╝╚══██╔══╝██╔══██╗    ██╔════╝██║     ██║██╔════╝████╗  ██║╚══██╔══╝
███████║   ██║      ██║   ██████╔╝    ██║     ██║     ██║█████╗  ██╔██╗ ██║   ██║   
██╔══██║   ██║      ██║   ██╔═══╝     ██║     ██║     ██║██╔══╝  ██║╚██╗██║   ██║   
██║  ██║   ██║      ██║   ██║         ╚██████╗███████╗██║███████╗██║ ╚████║   ██║   
╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚═╝          ╚═════╝╚══════╝╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   
                                   
====================================

a simple HTTP client written in C
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
client [-p PORT] [ -o FILE | -d DIR ] URL
```

### Examples

```sh
./client http://example.com/
```

(or)

```sh
./client -d folder http://example.com/
```

(or)

```sh
./client -o index.html http://localhost/
```
