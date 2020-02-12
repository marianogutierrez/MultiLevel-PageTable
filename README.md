# Work done:
This program was developed in tandem with another student. My work was the entirety of the pagetableDS.c and majority of the pagetable.c file excluding the getopt logic at the beginning. The byu_traderreader.c and byutr.h were provided.

## Overview:
The goal of this program was to implement a 32-bit working page table. The purpose of the page table is so that the OS can quickly and efficiently look up physical addresses based off given logical virtual memory ones i.e. the pages. In this program (particularly pagetableDS.c), the program assembles the page table, and initializes all offsets. The first few bits determine the number of pages and is used for the look up in the virtual table, while was is left is the offset, which is appended to the physical address.

## How to run the code:
Calling the make file and running "make pagetable" will create the executable page table. Note, the program requires at least 3 parameters. In C, the program counts for the argc value, and thus all that remains is a mandatory file (given via table_4_8_8.txt), and the desired number of bits per page. For example, 
```
./pagetable table_4_8_8.txt 5
```  
or 
```
./pagetable table_4_8_8.txt 4 5 6 
```
 i.e. first page is four bits (one hex digit) and so on. The program will report the bytes used for the table, and the percentages of hits and misses that vary depending on the decided size of the page table. There are some options too, -n # allows you to process only a few addresses, -p lets you write out output to another file, and -t shows you the logical to physical translation for the addresses (best done with a limited amount via -n to avoid too much output).
