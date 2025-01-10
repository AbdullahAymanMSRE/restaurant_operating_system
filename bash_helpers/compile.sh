#!/bin/bash

# Create output directory if it does not exist
if [ ! -d "c_output" ]; then
  mkdir c_output
fi

gcc -c c_code/shared.c -o c_output/shared.o -pthread
gcc -c c_code/order_process.c -o c_output/order_process.o -pthread
gcc c_code/client.c c_output/shared.o c_output/order_process.o -o c_output/client -pthread
gcc c_code/staff.c c_output/shared.o c_output/order_process.o -o c_output/staff -pthread
gcc c_code/display.c c_output/shared.o c_output/order_process.o -o c_output/display -pthread
gcc c_code/init_memory.c c_output/shared.o -o c_output/init_memory -pthread
