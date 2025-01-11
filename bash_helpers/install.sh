#!/bin/bash

if [ ! -d "~/.restaurant" ]; then
  mkdir ~/.restaurant
fi

if [ -f "/usr/bin/restaurant" ]; then
  sudo rm /usr/bin/restaurant
fi

sudo cp ../restaurant /usr/bin/restaurant
sudo cp -r ../* ~/.restaurant

echo "Installation complete"
echo "Type 'restaurant' to run the program"

