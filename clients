#!/bin/bash

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
