gcc -c shared.c -pthread
gcc -c order_process.c -pthread
gcc client.c shared.o order_process.o -o client -pthread
gcc staff.c shared.o order_process.o -o staff -pthread
gcc display.c shared.o order_process.o -o display -pthread