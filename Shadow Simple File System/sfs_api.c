#include "sfs_api.h"
#include "disk_emu.h"

// Global variables which represent the memory cache
superblock_t sb;
uint8_t freeBitMap[BLOCK_NUM];
uint8_t writeMask[BLOCK_NUM];
uint8_t freeBitMap_commit[BLOCK_NUM];
uint8_t writeMask_commit[BLOCK_NUM];
file_descriptor_t file_desc[INODE_NUM];

int inodes_per_blocks = BLOCK_SIZE/INODE_SIZE;
int inodes_per_directory = BLOCK_SIZE/DIRECTORY_ENTRY;

int main(int argc, char **argv){
	printf("First value %d\n", (int)file_desc[0].inode.size);
	printf("%d\n", (int)sizeof(directory_entry_t));

	uint8_t test2 = 9;

	printf("%d\n", 1%5);

	//mkssfs(1);
	mkssfs(0);

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
	int counter = sb.root.size;

	//Find the root directory


	inode_t first_jnode = sb.root;

	inode_t first_inode_block[BLOCK_SIZE/INODE_SIZE];    //Default value should be 1024/64 = 16


	read_blocks(first_jnode.direct[0], 1, first_inode_block);


	//The root directory should be pointed by the first inode in the file
	inode_t root_directory_inode = first_inode_block[0];



	while(counter > 0){

		//Start by scanning the last block of the root directory (with default values there should be 4 block max)
		int directory_block = counter/inodes_per_directory;

		int root_directory_ptr = root_directory_inode.direct[directory_block];


		directory_entry_t dir_entries[inodes_per_directory];

		read_blocks(root_directory_ptr, 1, dir_entries);

		for (int i = 0; i < inodes_per_directory; i++){

			if (!strcmp(name, dir_entries[i].name)){
				position = dir_entries[i].direct;
				counter = 0;
				break;
			}
		}

		counter -= inodes_per_directory;

	}

	return position;
}



superblock_t initialize_sb(){
	superblock_t sb;
	sb.magic_num;
	sb.bsize = BLOCK_SIZE;
	sb.fs_size = BLOCK_NUM;
	sb.num_inode = INODE_NUM; //A voir

	inode_t root;
	root.direct[0] = BLOCK_SIZE;
	root.indirect = -1;

	root.size = 1; // A voir

	sb.root = root;

	sb.shadow;
	sb.lastShadow;

	return sb;
}

void mkssfs(int fresh){

	int success;

	//If the flag is 0 therefore create a existing File System
	if (fresh == 0){
		success = init_disk(FSF_NAME, BLOCK_SIZE, BLOCK_NUM+OVERHEAD);

		if(success == -1){
			return;
		}

		read_blocks(0, 1, &sb);
		read_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap);
		read_blocks(BLOCK_NUM+OVERHEAD-1, 1, &writeMask);
		read_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap_commit);
		read_blocks(BLOCK_NUM+OVERHEAD-1, 1, &writeMask_commit);

	}
	//otherwise create a new file system
	else {
		success = init_fresh_disk(FSF_NAME, BLOCK_SIZE, BLOCK_NUM+OVERHEAD);

		if(success == -1){
			return;
		}

		//Initialize the superblock (1st block) to default values
		sb = initialize_sb();

		//Put all the flags in the FBM and in the WM to 1. (Advertise that all blocks are free)
		for (int i = 0; i < BLOCK_NUM; i++){
			freeBitMap[i] = ~freeBitMap[i];
			writeMask[i] = ~writeMask[i];
			freeBitMap_commit[i] = ~freeBitMap_commit[i];
			writeMask_commit[i] = ~writeMask_commit[i];
		}




		printf("Superblock size: %d\n", (int)sizeof(sb));
		printf("FreeBitMap size: %d\n", (int)sizeof(freeBitMap));
		printf("Write Mask size: %d\n", (int)sizeof(writeMask));

		write_blocks(0, 1, &sb);
		write_blocks(BLOCK_NUM+OVERHEAD-2, 1, &freeBitMap);
		write_blocks(BLOCK_NUM+OVERHEAD-1, 1, &writeMask);
	}
}


int ssfs_fopen(char *name){

	//Input validation

	//Check if name is empty

	if(strlen(name) > FILENAME_LENGTH){
		fprintf(stderr, "File name too long\n");
		return -1;
	}

	int position = find_file(name);

	//If we didn't find the file, create it
	if (position == 0){


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
	return 0;
}
int ssfs_fread(int fileID, char *buf, int length){
	return 0;
}
int ssfs_remove(char *file){
	return 0;
}
