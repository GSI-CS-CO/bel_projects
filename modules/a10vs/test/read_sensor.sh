#!/bin/bash

dev="dev/wbm0"            # default timing receiver device
vendor_id="0x00000651"    # vendor ID of the voltage sensor
device_id="0xa1076000"    # device ID of the voltage sensor

addr=""                   # voltage sensor address (will be detected by EB tools with the given IDs)

vs_cmd_reg=10             # offset to the sensor controller register
vs_irqs_reg=9             # offset to the sensor interrupt status
vs_cyclic_mode=0x103      # cyclic ADC operation for all channels
vs_irq_eop=0x01           # end-of-packet (complete block of samples is received)
channels=(                # channels monitored by the voltage sensor
    "VSIG_0" "VSIG_1" "Vcc" "Vccp"
    "Vccpt" "Vcceram" "Vccl_hps" "ADCGND")

samples=()                # sensor samples
adc_res=64                # ADC resolution: 6-bit
adc_ref=1250              # ADC reference:  1250 mV (0x03f)

usage() {
    echo "usage: $0 [device]"
    echo
    echo "  device  Optional TR device (dev/wbm0 by default)"
}

check_sensor_address() {

    if [ "$addr" = "" ]; then
        echo "Error: Sensor address is unknown: $addr"
        exit 1
    fi
}

check_arguments() {

    if [ $# -ne 0 ]; then

        test -e /$1
        if [ $? -ne 0 ]; then
            echo "Error: Device $1 does not exist!"
            usage
            exit 1
        fi

        dev=$1  # update the target device
    fi
}

get_sensor_address() {

    # get the base address of the voltage sensor
    addr=$(eb-find $dev $vendor_id $device_id)
    if [ $? -ne 0 ]; then
        echo "Error: Can't find the voltage sensor: vendor_id=$vendor_id  device_id=$device_id!"
        exit 1
    else
        echo "Info: Altera voltage sensor found at: $addr"
    fi
}

enable_sensor_operation() {

    check_sensor_address # exit on failure

    # get the controller status
    offset=$(( addr + vs_cmd_reg * 4 ))
    status=$(eb-read $dev $(printf "0x%x/4" $offset))

    # verify if the desired cyclic/continuous operation mode is already set
    if [ "0x$((16#$status))" = "$sensor_cyclic_mod" ]; then
        echo "Info: Conversion is on: cmd_reg=0x$((16#$status))"
        return
    fi

    # if the operation mode is not set, then enable it: 2 steps are required
    echo "Info: Conversion is off: cmd_reg=0x$((16#$status))"

    # first step: clear the end-of-packet status if it was set
    offset=$(( addr + vs_irqs_reg * 4 ))
    eb-write $dev $(printf "0x%x/4" $offset) $vs_irq_eop

    # second step: enable the cyclic/continuous conversion mode
    offset=$(( addr + vs_cmd_reg * 4 ))
    eb-write $dev $(printf "0x%x/4" $offset) $vs_cyclic_mode

    if [ $? -eq 0 ]; then
        echo "Info: Cyclic conversion mode: on"
    fi

    # check the interrupt status again
    offset=$(( addr + vs_irqs_reg * 4 ))
    status=$(eb-read $dev $(printf "0x%x/4" $offset))

    # wait until complete block of samples is received: status=1
    while [ "$((16#$status))" = "0" ]; do
        status=$(eb-read $dev $(printf "0x%x/4" $offset))
        sleep 0.5
    done
}

get_samples() {

    check_sensor_address # exit on failure

    echo "Info: Getting samples ..."

    samples=()

    # read all channels (defined in the 'channels' array)
    ch_cnt=$(( ${#channels[@]} - 1 ))
    for i in $(seq 0 $ch_cnt); do
        offset=$((addr + i * 4))
        sample=$(eb-read $dev $(printf "0x%x/4" $offset))
        samples+=("$sample")
    done
}

print_samples() {

    # expect 8 samples
    if [ ${#samples[@]} -ne ${#channels[@]} ]; then
        echo "Error: Bad samples: ${samples[@]}"
        return
    fi

    # print heading
    printf "=%.0s" {1..30}; printf "\n"
    printf "%-8s %-10s %-5s %s\n" Channel Offset Sample mV
    printf "=%.0s" {1..30}; printf "\n"

    # print raw and human-readable sample values
    for i in $(seq 0 7); do
        sample_hex=$((16#${samples[$i]}))
        sample_mV=$(( sample_hex * adc_ref / adc_res ))
        offset=$((addr + i * 4))
        printf "%-8s 0x%.8x 0x%.3x  %d\n" ${channels[$i]} $offset $sample_hex $sample_mV
    done
}

check_arguments
get_sensor_address
enable_sensor_operation
get_samples
print_samples
