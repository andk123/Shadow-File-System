//Functions you should implement. 

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FSF_NAME "FSF260638436"
#define FILENAME_LENGTH 10
#define BLOCK_SIZE 1024
#define BLOCK_NUM 1024
#define OVERHEAD 3 //The overhead is for the superblock, FBM and the WM
#define INODE_SIZE 64
#define INODE_NUM 200
#define DIRECTORY_ENTRY 16


typedef struct _inode_t{
	int size;
	int direct[14];
	int indirect;

}inode_t;


typedef struct _superblock_t{

	unsigned char magic_num[4];
	int bsize; //BLOCK_SIZE
	int fs_size;// BLOCK_NUM
	int num_inode; // INODE_NUM
	inode_t root;
	inode_t shadow[4];
	int lastShadow;

}superblock_t;


typedef struct _directory_entry_t{
	char name[FILENAME_LENGTH];
	int direct;

}directory_entry_t;


typedef struct _file_desc_t{
	inode_t inode;
	int read_ptr;
	int write_ptr;

}file_descriptor_t;


superblock_t initialize_sb();

int checkBit(uint8_t bit);

int find_file(char *name);








//Return -1 for error besides mkssfs
void mkssfs(int fresh);
int ssfs_fopen(char *name);
int ssfs_fclose(int fileID);
int ssfs_frseek(int fileID, int loc);
int ssfs_fwseek(int fileID, int loc);
int ssfs_fwrite(int fileID, char *buf, int length);
int ssfs_fread(int fileID, char *buf, int length);
int ssfs_remove(char *file);
int ssfs_commit();
int ssfs_restore(int cnum);
