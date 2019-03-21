#!/bin/bash
rm fpga.bin
cp /home/franciszek/Documents/fpga/firN/kfir_2.runs/impl_1/top.bit .
source ~/Xilinx/14.7/LabTools/settings64.sh
promgen -w -b -p bin -o fpga.bin -u 0 top.bit -data_width 32
rm fpga.prm fpga.cfi top.bit
