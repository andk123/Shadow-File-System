#include "sfs_api.h"
#include "disk_emu.h"

// Global variables which represent the memory cache
superblock_t sb;
uint8_t freeBitMap[BLOCK_NUM];
uint8_t writeMask[BLOCK_NUM];
file_descriptor_t file_desc[INODE_NUM];

int inodes_per_block = BLOCK_SIZE/INODE_SIZE;
int inodes_per_directory = BLOCK_SIZE/DIRECTORY_ENTRY;

int main(int argc, char **argv){
	printf("First value %d\n", (int)file_desc[0].inode.size);
	printf("%d\n", (int)sizeof(directory_entry_t));


	mkssfs(0);
	ssfs_fopen("lol");

	inode_t test1[inodes_per_block];
	directory_entry_t test2[inodes_per_directory];

	printf("File system size: %d\n", sb.fs_size);
	printf("Root size: %d\n", sb.root.size);

/*
	for (int i = 0; i < BLOCK_NUM; i++){
		printf("%d", checkBit(freeBitMap[i]));
	}

	printf("\n");

	read_blocks(1, 1, &test1);
	read_blocks(1024, 1, &test2);

	for (int i = 0; i<10; i++){
	printf("Root size: %d\n", test1[i].size);
	printf("Root pointer: %d\n", test1[i].direct[0]);
	}

	for (int i = 0; i<10; i++){
	printf("Root inode name: %s\n", test2[i].name);
	printf("Root inode1 pointer: %d\n", test2[i].direct);
	}

	*/

	close_disk();

	return 0;
}


/*
 * Check whether the bit corresponding to the current shadow root
 * is a 1.
 * For the bitmap reference: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
 */
int checkBit(uint8_t bit){

	return (bit & 1 == 1);
}


//Method to find a file in the root director. Returns 0 if no specified file
int find_file(char *name){

	int position = 0;
	int counter = sb.root.size-1;

	//Find the root directory


	inode_t first_jnode = sb.root;

	inode_t first_inode_file[inodes_per_block];    //Default value should be 1024/64 = 16


	read_blocks(first_jnode.direct[0], 1, first_inode_file);


	//The root directory should be pointed by the first inode in the file
	inode_t root_directory_inode = first_inode_file[0];



	while(counter > 0){

		//Start by scanning the last block of the root directory (with default values there should be 4 block max)
		int directory_block = counter/inodes_per_directory;

		int root_directory_ptr = root_directory_inode.direct[directory_block];


		directory_entry_t root_directory[inodes_per_directory];

		read_blocks(root_directory_ptr, 1, root_directory);

		for (int i = 0; i < inodes_per_directory; i++){

			if (!strcmp(name, root_directory[i].name)){
				position = root_directory[i].direct;
				counter = 0;
				break;
			}
		}

		counter -= inodes_per_directory;

	}
	return position;
}


int find_next_available_directory(){

	sb.first_free_directory++;

	if(sb.root.size == sb.first_free_directory){
		return sb.first_free_directory;
	}

	int directory_number = sb.first_free_directory / inodes_per_directory;
	int directory_position = sb.root.direct[0];

	//Select the inode file containing the pointer to the root directory blocks
	inode_t inode_file_root_directory[inodes_per_block];
	read_blocks(directory_position, 1, &inode_file_root_directory);


	//Retrieve which directory (out of the 4) the file will be in
	directory_entry_t root_directory[inodes_per_directory];


	for (int i = directory_number; i < inode_file_root_directory[0].size; i++){ //by default there can only be a max of 4 root directory

		directory_position = inode_file_root_directory[0].direct[i];
		read_blocks(directory_position, 1, &root_directory);

		for (int j = sb.first_free_directory % inodes_per_directory; j < inodes_per_directory; j++){

			if (root_directory[j].direct == -1 ){
				return sb.first_free_directory;
			}
			sb.first_free_directory++;
		}
	}

	return -1;
}


int find_next_available_inode(){
	sb.first_free_inode++;

	if(sb.root.size == sb.first_free_inode-1){
		return sb.first_free_inode;
	}

	int inode_number = sb.first_free_inode / inodes_per_block;

	//Select the inode file that will contain the inode
	inode_t inode_file[inodes_per_block];


	for (int i = inode_number; i < (sb.fs_size-OVERHEAD/inodes_per_block)+1; i++){

		int inode_position = sb.root.direct[inode_number];
		read_blocks(inode_position, 1, &inode_file);


		for (int j = sb.first_free_inode % inodes_per_block; j < inodes_per_block; j++){

			if (inode_file[j].size == -1 ){
				return sb.first_free_inode;
			}
			sb.first_free_inode++;
		}
	}

	return -1;
}


int find_first_available_block(){
	int i;
	for (i = 0; i < BLOCK_NUM; i++){
		//Verify that the bit is free
		if (checkBit(freeBitMap[i]) == 1){
			//If it is, and the number with 1111 1110 to make the most significant bit zero
			freeBitMap[i] = freeBitMap[i]&254;
			return i+1;
		}
	}
	return -1;
}

int find_last_available_block(){
	int i;
	for (i = BLOCK_NUM-1; i > 0; i--){
		//Verify that the bit is free starting from the end
		if (checkBit(freeBitMap[i]) == 1){
			//If it is, and the number with 1111 1110 to make the most significant bit zero
			freeBitMap[i] = freeBitMap[i]&254;
			return i+1;
		}
	}
	return -1;
}







//A REVOIR
superblock_t initialize_sb(){
	superblock_t sb;
	sb.magic_num;
	sb.bsize = BLOCK_SIZE;
	sb.fs_size = 0;

	inode_t root;
	root.direct[0] = BLOCK_SIZE;
	root.indirect = -1;

	root.size = 0;

	sb.root = root;

	sb.shadow;
	sb.lastShadow;

	return sb;
}


void initialize_root_directory(directory_entry_t entry[]){

	for (int i = 0; i < inodes_per_directory; i++){
		entry[i].direct = -1;
	}
}


void initialize_inode_file(inode_t inode[]){

	for (int i = 0; i < inodes_per_block; i++){
		inode[i].size = -1;
	}
}


void initialize_file_descriptor(){

	for (int i = 0; i < INODE_NUM; i++){
		file_desc[i].read_ptr = -1;
		file_desc[i].write_ptr = -1;
	}
}


void mkssfs(int fresh){

	int success;

	//If the flag is 0 therefore open an existing File System
	if (fresh == 0){
		success = init_disk(FSF_NAME, BLOCK_SIZE, BLOCK_NUM+OVERHEAD);

		if(success == -1){
			return;
		}

		read_blocks(0, 1, &sb);
		read_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap);
		read_blocks(BLOCK_NUM+OVERHEAD-1, 1, &writeMask);

	}
	//otherwise create a new file system
	else {
		success = init_fresh_disk(FSF_NAME, BLOCK_SIZE, BLOCK_NUM+OVERHEAD);

		if(success == -1){
			return;
		}

		//Initialize the superblock (1st block) to default values
		sb = initialize_sb();




		directory_entry_t root_directory[inodes_per_directory];
		initialize_root_directory(root_directory);

		inode_t inode_file[inodes_per_block];
		initialize_inode_file(inode_file);

		inode_file[0].direct[0] = BLOCK_NUM;
		inode_file[0].size = 1; //default max can be 4 for 4 root directory


		write_blocks(BLOCK_NUM, 1, &root_directory);
		write_blocks(1, 1, &inode_file);


		sb.fs_size = 5;

		sb.root.direct[0] = 1; //Make it point to the first inode
		sb.root.size = 1;

		sb.first_free_inode = 1;
		sb.first_free_directory = 0;

		write_blocks(0, 1, &sb);

		//Put all the flags in the FBM and in the WM to 1. (Advertise that all blocks are free)
		for (int i = 0; i < BLOCK_NUM; i++){
			freeBitMap[i] = ~freeBitMap[i];
			writeMask[i] = ~writeMask[i];
		}

		//Advertise that the first block is to the inode file and the last block to root directory
		freeBitMap[0] = 0;
		freeBitMap[BLOCK_NUM-1] = 0;

		//Wrote FBM and WM at the last and second last position in the file system
		write_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap);
		write_blocks(BLOCK_NUM+OVERHEAD-1, 1, &writeMask);

	}
}


int ssfs_fopen(char *name){

	//Input validation

	//ADD Check if name is empty

	if(strlen(name) > FILENAME_LENGTH){
		fprintf(stderr, "File name too long\n");
		return -1;
	}

	int position = find_file(name);

	//If we didn't find the file, create it
	if (position == 0){


		/*** Create the directory entry	***/


		int directory_place = sb.first_free_directory % inodes_per_directory;
		int directory_number = sb.first_free_directory / inodes_per_directory;
		int directory_position = sb.root.direct[0];


		//Select the inode file containing the pointer to the root directory blocks
		inode_t inode_file_root_directory[inodes_per_block];
		read_blocks(directory_position, 1, &inode_file_root_directory);

		directory_entry_t root_directory[inodes_per_directory];


		//If the current directory is full, create a new one
		if(directory_place == 0 && directory_number > 0 && sb.root.size-1 == sb.first_free_directory){

			if(sb.fs_size == BLOCK_NUM+OVERHEAD){
				printf("File system full, can't create file\n");
				return -1;
			}



			//Find position of last available block
			int new_position = find_last_available_block();

			//Write the new root directory to disk and update the corresponding inode file
			initialize_root_directory(root_directory);
			write_blocks(new_position, 1, &root_directory);
			sb.fs_size++;

			inode_file_root_directory[0].direct[directory_number] = new_position;
			inode_file_root_directory[0].size++;
			write_blocks(directory_position, 1, &inode_file_root_directory);

		}


		//Retrieve which directory (out of the 4) the file will be in
		directory_position = inode_file_root_directory[0].direct[directory_number];

		//Find the correct pointer pointing to the root_directory where the first available position is
		read_blocks(directory_position, 1, &root_directory);


		//Create the instance in the root directory
		strcpy(root_directory[directory_place].name, name);
		root_directory[directory_place].direct = sb.first_free_inode;
		write_blocks(directory_position, 1, &root_directory);

		//Find the next available directory slot
		sb.first_free_directory = find_next_available_directory();



		/*** Create the inode ***/


		int inode_place = sb.first_free_inode % inodes_per_block;
		int inode_number = sb.first_free_inode / inodes_per_block;
		int inode_position = sb.root.direct[inode_number];

		//Select the inode file that will contain the inode
		inode_t inode_file[inodes_per_block];

		//If the current inode file is full, create a new one
		if(inode_place == 0 && sb.root.size == sb.first_free_inode){

			if(sb.fs_size == BLOCK_NUM+OVERHEAD){
				printf("File system full, can't create file\n");
				return -1;
			}


			//Find position of first available block
			int new_position = find_first_available_block();


			//Write the new inode file to disk and update the root
			initialize_inode_file(inode_file);
			write_blocks(new_position, 1, &inode_file);
			sb.fs_size++;

			sb.root.direct[inode_number] = new_position;
		}


		//Retrieve which inode file the inode will be in
		inode_position = sb.root.direct[inode_number];

		//Find the correct pointer pointing to the root_directory where the first available position is
		read_blocks(inode_position, 1, &inode_file);

		if(sb.fs_size == BLOCK_NUM+OVERHEAD){
			printf("File system full, can't create file\n");
			return -1;
		}

		//Find new data block and create it
		int new_position = find_first_available_block();
		char buffer[1024];

		write_blocks(new_position, 1, &buffer);
		sb.fs_size++;

		//Create the instance in the inode file
		inode_file[inode_place].direct[0] = new_position;
		inode_file[inode_place].size = 0;

		write_blocks(inode_position, 1, &inode_file);
		sb.root.size++;

		position = sb.first_free_inode;

		//Find the next available inode slot
		sb.first_free_inode = find_next_available_inode();


		write_blocks(0, 1, &sb);
		write_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap);
	}

	//Create file descriptor entry
	for (int i = 0; i <= INODE_NUM; i++){
		// In case the file descriptor runs out of place
		if(i == INODE_NUM){
			printf("File descriptor full, close a file before opening again.\n");
			return -1;
		}





	}


	return 0;
}


int ssfs_fclose(int fileID){
	return 0;
}
int ssfs_frseek(int fileID, int loc){
	return 0;
}
int ssfs_fwseek(int fileID, int loc){
	return 0;
}
int ssfs_fwrite(int fileID, char *buf, int length){

	//verify with write mask
	return 0;
}
int ssfs_fread(int fileID, char *buf, int length){
	//different version of file could point to same data block (from diff inode block)
	//No need write mask
	return 0;
}
int ssfs_remove(char *file){
	//put back values to -1
	return 0;
}
