# Multithread DNS Lookup Project

To compile:
- type "make all" in the terminal


To run: valgrind ./multi-lookup <# requester> <# resolver> <requester log> <resolver log> <data file>...<data file>

valgrind: Checks for memory leaks

<# requester>: Num of producer threads

<# resolver>: Num of consumer threads

<requester log>: Write producer status info into this file
	
<resolver log>: Write consumer status info into this file
	
<data file>: Files that contain domain names

Example: valgrind ./multi-lookup 1 1 serviced.txt results.txt names1.txt names2.txt names3.txt names4.txt names5.txt
