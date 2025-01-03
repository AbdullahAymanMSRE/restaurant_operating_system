#!/bin/bash

show_memory() {
    ./restaurant_system show_memory
}

init_system() {
    # Print welcome message 
    chmod +x ./welcome.sh
    ./welcome.sh

    # Compile the main C program
    gcc -o restaurant_system restaurant_system.c -pthread
    if [ $? -ne 0 ]; then
        echo "Compilation failed"
        exit 1
    fi
    
    # Start the kitchen process
    ./restaurant_system process_orders & KITCHEN_PID=$!
}

show_menu() {
    echo -e "\nPlease select the program you want to open:"
    echo "1. Serve Clients"
    echo "2. Show Status"
    echo "3. Handle Orders"
    echo "4. Exit"
}

main() {
    init_system
    
    while true; do
        show_menu
        read -p "Enter choice: " choice
        
        case $choice in
            1)
                clients
                ;;
            2)
                status
                ;;
            3)
                staff
                ;;
            4)
                echo "Shutting down system..."
                kill $KITCHEN_PID
                exit 0
                ;;
            *)
                echo "Invalid choice"
                ;;
        esac
    done
}

trap 'echo "Use option 4 to exit properly"; sleep 1' SIGINT SIGTERM

main
