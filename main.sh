#!/bin/bash

compile_system() {
    gcc -o restaurant_system restaurant_system.c -pthread
    if [ $? -ne 0 ]; then
        echo "Compilation failed"
        exit 1
    fi
}

init_system() {
    compile_system
    ./restaurant_system process_orders &
    KITCHEN_PID=$!
}

create_order() {
    echo "Menu:"
    echo "1. Burger ($9.99)"
    echo "2. Pizza ($12.99)"
    echo "3. Salad ($7.99)"
    echo "4. Drink ($2.99)"
    
    items=()
    quantities=()
    
    while true; do
        read -p "Enter item number (0 to finish): " item
        if [ "$item" == "0" ]; then
            break
        fi
        read -p "Enter quantity: " quantity
        items+=($item)
        quantities+=($quantity)
    done
    
    ./restaurant_system create_order "${items[@]}" "${quantities[@]}"
}

show_menu() {
    echo "1. Create new order"
    echo "2. Show system status"
    echo "3. Show memory usage"
    echo "4. Exit"
}

main() {
    init_system
    
    while true; do
        echo -e "\nRestaurant Management System"
        show_menu
        read -p "Enter choice: " choice
        
        case $choice in
            1)
                create_order
                ;;
            2)
                show_status
                ;;
            3)
                show_memory
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
