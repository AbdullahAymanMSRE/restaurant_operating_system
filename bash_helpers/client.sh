#!/bin/bash

validate_number() {
    local input=$1
    if ! [[ "$input" =~ ^[0-9]+$ ]]; then
        echo "Invalid input, please enter a number"
        return 1
    fi
    return 0
}

menu_size=$(c_output/client --size)

while true;
do
    clear
    echo -e "\n===== MENU ====="
    c_output/client --menu
    echo "==============="

    items=()
    quantities=()
    
    while true; do
        read -p "Enter item number (0 to finish | q to exit): " item
        if [ "$item" == "q" ]; then
            exit 0
        fi

        if ! validate_number "$item_id"; then
            continue
        fi

        if [ "$item" == "0" ]; then
            break
        fi

        if [ "$item" -lt 1 ] || [ "$item" -gt "$menu_size" ]; then
            echo "Invalid item ID. Please choose between 1 and $menu_size"
            continue
        fi

        read -p "Enter quantity: " quantity

        if ! validate_number "$quantity" ; then
            continue
        fi

        items+=($item)
        quantities+=($quantity)
    done

    if [ ! -z "$items" ]; then   

      c_output/client --order "${items[@]}" "${quantities[@]}"

      sleep 2
    fi
done

