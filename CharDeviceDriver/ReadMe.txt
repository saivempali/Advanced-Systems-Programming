NAME : Sai Vishnu Teja Vempali
UFID : 16141381
Email : vishnu24@ufl.edu
=======================================

Assignment 4
--------------

Extract the files from the Assignment4.zip and copy the files chardriver.c, Makefile and userapp.c to a 
virtual linux machine
and follow the following steps:

NOTE: Please use the userapp.c submitted by me as some changes were done to it for debugging convenience.

1) Compile driver module : $ make

2) Load module : $ sudo insmod chardriver.ko num_of_devices=<num_devices>
		 Sample : sudo insmod chardriver.ko num_of_devices=10

3) Test driver :
	1) Compile userapp : $ make app
	2) Run userapp : $ sudo ./userapp <device_number>
			 Sample : sudo ./userapp 8		

4) Unload module : $ sudo rmmod char_driver
