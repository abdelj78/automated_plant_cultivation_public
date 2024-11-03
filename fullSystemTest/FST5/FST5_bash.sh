#!/bin/bash

#this is needed otherwise it will only kill one of the processes only seems to be the c one
# Function to handle SIGINT
handle_sigint() {
    # Kill the C program
    kill $C_PID
    # Kill the Python script
    kill $PYTHON_PID
    exit
}

# Trap SIGINT and call the handle_sigint function
trap handle_sigint SIGINT

# Compile the C code
sudo gcc -o FST5 FST5.c -lm -lbcm2835 -lfftw3

# Start the Python script in the background
python3 -u ./FST5_pump.py &

# Save the PID of the Python script
PYTHON_PID=$!

# Start the C program and pass the PID of the Python script
sudo ./FST5 $PYTHON_PID &

# Save the PID of the C program
C_PID=$!

# Wait for both processes to finish
wait $PYTHON_PID $C_PID