#!/bin/bash

dev="dev/wbm0"
vendor_id="0x00000651"
device_id="0xa1076000"
addr=""      # voltage sensor address (will be detected by EB tools with the given IDs)

# channels monitored by the voltage sensor
channels=(
    "VSIG_0" "VSIG_1" "Vcc" "Vccp"
    "Vccpt" "Vcceram" "Vccl_hps" "ADCGND")

samples=()   # sensor samples
adc_res=64   # 6-bit
adc_ref=1250 # 1250 mV

get_sensor_address() {

    addr=$(eb-find $dev $vendor_id $device_id)
    if [ $? -ne 0 ]; then
        echo "Error: Can't find the voltage sensor (vendor_id device_id): $vendor_id $device_id!"
        exit 1
    else
        echo "Info: Altera voltage sensor found at: $addr"
    fi
}

get_samples() {

    if [ "$addr" = "" ]; then
        echo "Error: Sensor address is unknown: $addr"
        return
    fi

    samples=()

    for i in $(seq 0 7); do
        offset=$((addr + i * 4))
        sample=$(eb-read $dev $(printf "0x%x/4" $offset))
        samples+=("$sample")
    done
}

print_samples() {

    if [ ${#samples[*]} -ne 8 ]; then
        echo "Error: Bad samples: ${samples[*]}"
        return
    fi

    printf "=%.0s" {1..30}; printf "\n"
    printf "%-8s %-10s %-5s %s\n" Channel Offset Sample mV
    printf "=%.0s" {1..30}; printf "\n"
    for i in $(seq 0 7); do
        sample_hex=$((16#${samples[$i]}))
        sample_mV=$(( sample_hex * adc_ref / adc_res ))
        offset=$((addr + i * 4))
        printf "%-8s 0x%.8x 0x%.3x  %d\n" ${channels[$i]} $offset $sample_hex $sample_mV
    done
}

get_sensor_address
get_samples
print_samples
