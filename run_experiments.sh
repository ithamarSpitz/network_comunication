#!/bin/bash

# Set the number of runs
num_runs=5

# Set the initial port number
initial_port=12345

# Output file for all receiver logs
all_output_file="all_receiver_output.txt"

# Function to run a command and append output
run_and_append() {
    title=$1
    command=$2

    echo "$title" >> $all_output_file
    script -a -c "$command" $all_output_file
    echo -e "\n" >> $all_output_file
}

# Function to send 'y' and 'n' every 2 seconds
send_y_and_n() {
    for ((i=1; i<=5; i++)); do
        sleep 1
        echo "y"
        sleep 1
        echo "y"
        sleep 1
        echo "y"
        sleep 1
        echo "y"
        sleep 1
        echo "n"
    done
}
run_expiriment(){
    # Run experiments for TCP Reno
    for ((i=1; i<=num_runs; i++)); do
        receiver_port=$((initial_port + i))
        run_and_append "TCP Reno Run $i" "./TCP_Receiver -p $receiver_port -algo reno" &
        sleep 2
        send_y_and_n | ./TCP_Sender -ip 127.0.0.1 -p $receiver_port -algo reno
        sleep 1
    done

    # Run experiments for TCP Cubic
    for ((i=1; i<=num_runs; i++)); do
        receiver_port=$((initial_port + i + num_runs))
        run_and_append "TCP Cubic Run $i" "./TCP_Receiver -p $receiver_port -algo cubic" &
        sleep 2
        send_y_and_n | ./TCP_Sender -ip 127.0.0.1 -p $receiver_port -algo cubic
        sleep 1
    done

    # Run experiments for TCP Cubic - Reno
    for ((i=1; i<=num_runs; i++)); do
        receiver_port=$((initial_port + i + (3*num_runs)))
        run_and_append "TCP sender Cubic - receiver Reno Run $i" "./TCP_Receiver -p $receiver_port -algo reno" &
        sleep 2
        send_y_and_n | ./TCP_Sender -ip 127.0.0.1 -p $receiver_port -algo cubic
        sleep 1
    done

    # Run experiments for TCP Reno - Cubic
    for ((i=1; i<=num_runs; i++)); do
        receiver_port=$((initial_port + i + (4*num_runs)))
        run_and_append "TCP sender Reno - receiver Cubic Run $i" "./TCP_Receiver -p $receiver_port -algo cubic" &
        sleep 2
        send_y_and_n | ./TCP_Sender -ip 127.0.0.1 -p $receiver_port -algo reno
        sleep 1
    done

    # Run experiments for RUDP
    for ((i=1; i<=num_runs; i++)); do
        receiver_port=$((initial_port + i + (10 * num_runs)))
        run_and_append "RUDP Run $i" "./RUDP_Receiver -p $receiver_port" &
        sleep 2
        send_y_and_n | ./RUDP_Sender -ip 127.0.0.1 -p $receiver_port
        sleep 1
    done
    }

run_expiriment

run_and_append "sudo tc qdisc add dev lo root netem loss 2%"

run_expiriment

run_and_append "sudo tc qdisc add dev lo root netem loss 5%"

run_expiriment

run_and_append "sudo tc qdisc add dev lo root netem loss 10%"

run_expiriment

run_and_append "sudo tc qdisc del dev lo root"
