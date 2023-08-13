#! /bin/sh
gcc -o client client.c -lpthread
gcc -o server server.c -lpthread
./client
./server
