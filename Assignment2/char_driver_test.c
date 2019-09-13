#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main() {
	int file = open("/dev/simple_character_device", O_RDWR);
	if (file < 0) {
		printf("Could not open device.\n");
		return 0;
	}

	char user_buffer[1024];
	int offset = 0;
	int whence = 0;
	int bytes = 0;

	char key = 'a';
	while (key != 'e') {
		printf("Press r to read from device\n");
		printf("Press w to write to the device\n");
		printf("Press s to seek into the device\n");
		printf("Press e to exit from the device\n");
		printf("Press anything else to keep reading or writing from the device\n");
		printf("Enter command: ");
		scanf("%c", &key);
		while(getchar() != '\n');
		switch(key){
			case 'r':
				printf("Enter the number of bytes you want to read: ");
				scanf("%d", &bytes);
				while(getchar() != '\n');
				char *read_buffer = malloc(bytes*sizeof(char));
				read(file, read_buffer, bytes);
				printf("Data read from the device: %s\n", read_buffer);
				free(read_buffer);
				break;

			case 'w':
				printf("Enter data you want to write to the device: ");
				scanf("%s", user_buffer);
				while(getchar() != '\n');
				write(file, user_buffer, strlen(user_buffer));
				break;

			case 's':
				printf("Enter an offset value: ");
				scanf("%d", &offset);
				while(getchar() != '\n');
				printf("Enter a value for whence (third parameter): ");
				scanf("%d", &whence);
				while(getchar() != '\n');
				llseek(file, offset, whence);
				break;

			default:
				break;
		}
		printf("\n");
	}
	close(file);
}