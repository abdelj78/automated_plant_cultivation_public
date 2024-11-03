/usr/bin/python3 -u ./librosacheck.py &

# Save the PID of the Python script
PYTHON_PID=$!

# Wait for both processes to finish
wait $PYTHON_PID $C_PID