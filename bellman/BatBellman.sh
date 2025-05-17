#!/bin/bash

# Binário
bellman=./bellman.bin

echo "Testando algoritmo de Bellman"
for i in entradas/*.mtx
do
	echo -e "\e[33mInstância $i\e[0m";
	$bellman -f $i
done