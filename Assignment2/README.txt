Julia Sanford
jusa1470@colorado.edu
Programming Assignment 2

File Information:

Makefile
	This file includes the line "obj-m := simple_char_driver.o" which is tells the kbuild that this is what is to be compiled. There is a -m because this is a module. It assumes that there is a .c file with the same name, simple_char_driver.c.

simple_char_driver.c
	This is the source code for the character driver. This file has the init and exit functions that the kernel knows that this is a loadable kernel module, and it executes in the init function and runs the exit when the module is uninstalled. These functions also register and unregister the device, and they allocate and deallocate memory for the buffer. There is also a struct that links the five operations that you can do to the file to the functions: open, close, read, write, and seek.

	Open keeps track of how many times the file is opened and prints this to the kernel. Release/close does the same thing except keeps track of how many times the file is closed.

	The read function takes in 4 parameters: the file, the user buffer, the length of which is to be read (number of characters), and the offset. First, this function makes sure that the offset is not at the end and there are still characters that can be read. If there are no characters left to be read, this is printed to the kernel. Otherwise, it will copy the amount of characters starting at the offset in the device_buffer to the user buffer. The offset is then updated to be the amount of characters it read more than it was which would be where it stopped reading.

	The write function takes in the same 4 parameters as the read function. First, it finds out how many spots there are left in the buffer which is just the buffer size minus the current position or offset. Then, it checks to see if amount of characters that it wants to write would fit in the space left. If the length to be written is larger than the amount left, then we only need to write the number of spots left. If the space left in the buffer is larger than the length of the string to be written, then we can write that length of string. Finally, we copy our calculated number of bytes to be written from the user buffer to the device buffer at the current position or offset. Offset is then updated with however many bytes were written.

	The seek function takes in 3 parameters: the file, offset, and whence. Whence will tell it the option of changing offset. If whence is zero, then the file position just goes to where offset is. If whence is one, the file position is incremented by offset. If whence is two, the file position is set to offset bytes from the end of the file. If any of these options bring the file position to the front of the buffer or past the buffer size, the file position will go back to what it was before any changes were made.

char_driver_test.c
	This is the source code to test the character driver. First, the file is opened, and if it doesn't open, there will be a print statement saying this. Then, a menu prints for the user to choose an option. The user's option is registered in a switch statement.

	If the user chooses 'r', they are prompted to enter the number of bytes they want to read. A read buffer of that size is allocated. Then the read function is called with the file, the buffer, and the number of bytes. The data read is printed, and the memory for this buffer is deallocated. 

	If the user chooses 'w', the user is prompted to enter the data they want to write. The write function is called with the file, the user buffer with the data, and the length of the data.

	If the user chooses 's', the user is prompted to enter an offset value and a whence value. The llseek function is called with the file, offset, and whence.

	If the user chooses 'e', the program will stop printing the menu and exit.

	If the user chooses any other character than the options provided, nothing will happen and the menu will be printed again with the same prompt to enter a command.



Instructions for Building and Running Test Program:

1.Find a major number that is not in use by looking at the major numbers that come up when you type cat /prov/devices into the terminal.

2. Create a device file using this command:
	sudo mknod -m 777 /dev/simple_character_device c 123 0
	The 777 is to give owners, groups, and others all read, write, and execute permissions. The number 123 is the major number. 0 is the minor number. 'c' is for creating a character driver.

3. Create a Makefile containing the line obj-m := simple_char_driver.o.

4. Make the file using this command:
	make -C /lib/modules/$(uname -r)/build M=$PWD modules

5. Insert your module to the kernel using this command:
	sudo insmod simple_char_driver.ko

6. Check the syslog file or type dmesg to make sure that the init function was executed and printed.

7. Compile the test file using this command:
	gcc -o char_driver char_driver_test.c

8. Read, write, and seek using the test file as you wish.

9. Check the syslog file or type dmesg to make sure the correct print statements are happening.

10. Remove the module from the kernel using this command:
	sudo rmmod simple_char_driver