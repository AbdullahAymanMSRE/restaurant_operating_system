gcc -c ../c_code/shared.c -pthread
gcc -c ../c_code/order_process.c -pthread
gcc ../c_code/client.c shared.o order_process.o -o ../c_output/client -pthread
gcc ../c_code/staff.c shared.o order_process.o -o ../c_output/staff -pthread
gcc ../c_code/display.c shared.o order_process.o -o ../c_output/display -pthread
gcc ../c_code/init_memory.c shared.o -o ../c_output/init_memory -pthread
