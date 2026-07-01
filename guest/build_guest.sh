#!/bin/sh
make clean
make BASE_ADDR=0x50000000
mv guest.bin guest0.bin
mv guest.elf guest0.elf
mv guest.list guest0.list
cp guest0.bin ../
cp guest0.elf ../
cp guest0.list ../

#
make clean
make BASE_ADDR=0x60000000
mv guest.bin guest1.bin
mv guest.elf guest1.elf
mv guest.list guest1.list
cp guest1.bin ../
cp guest1.elf ../
cp guest1.list ../
