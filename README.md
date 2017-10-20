# hello_sockets
Simple POSIX sockets hello world program

This program demonstrates *very* basic use of a POSIX socket to send a single number from a client process to a server process across a POSIX socket.

To keep things simple, both processes execute on the same computer.

Example output:

```
$ clang -std=c++11 demo.cpp -lstdc++
$ ./a.out 
Reader listening for a connection...
Reader connected.
Writer connected.
Parent process received 13
OK

```

