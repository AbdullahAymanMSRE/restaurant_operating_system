# EUI Restaurant Operating System

## Introduction

This is an operating system designed for restaurants. It contains three programs:

- one to serve clients
- one for staff members to maintain orders
- one to show orders' statuses

## Installation

1. Clone the repository
2. Run the following commands in the terminal:

   - ```bash
         chmod +x clients.sh staff.sh status.sh welcome.sh restaurant.sh
     ```

     to make the scripts executable

   - ```bash
          sudo mv clients.sh staff.sh status.sh welcome.sh restaurant.sh restaurant_system.c /usr/bin/
     ```
     to move the scripts to the `/usr/bin/` directory to be able to run them from anywhere in the terminal

## Usage

Run `restaurant` in the terminal to start the system.
