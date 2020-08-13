#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

//一个简单的栈
#define BOOL int
#define TRUE 1
#define FALSE 0
typedef struct Stack
{
	char address[128];
} addrStack;
addrStack stack[10];//定义一个数组作为stack
int top = 0;//定义stack的指针
BOOL STACK_EMPTY()//判断stack是否为空
{
	if (top == 0)
		return TRUE;
	else
		return FALSE;
}
void PUSH(addrStack* S, char x[])//入栈操作
{
	
	top = top + 1;
	if (top >= 10) {
		top = top - 1;
		return;
	}
	addrStack temp;
	memcpy(temp.address,x,128);
	S[top] = temp;
}
addrStack POP(addrStack* S)//出栈操作
{
	if (!STACK_EMPTY())
	{
		top = top - 1;
		return S[top + 1];

	}//如果栈为空，则返回-1		
}


PUBLIC int do_find(const char *_FILENAME)
{

    printl("\ninode     location        \n");
    printl("============================\n");

    PUSH(stack,"/");
    while(top!=0)
    {
       
        char pathName[128];
        char* temp=POP(stack).address;
        memcpy(pathName,temp,128) ;

        // 取得message中的信息，详见lib/ls.c
        int name_len=strlen(pathName);

        assert(name_len < 128);  // 路径名称长度不得超过最大长度

        pathName[name_len] = 0;

        int i, j;

        //struct inode * dir_inode = root_inode;
        struct inode * dir_inode;  // 需要令它指向当前的目录节点
        char fileName[20];
        strip_path(fileName, pathName,&dir_inode);

        int dir_blk0_nr = dir_inode->i_start_sect;
        int nr_dir_blks = (dir_inode->i_size + SECTOR_SIZE - 1) / SECTOR_SIZE;
        int nr_dir_entries = dir_inode->i_size / DIR_ENTRY_SIZE;
        int m = 0;

        struct dir_entry * pde;
        struct inode* new_inode;  // 指向每一个被遍历到的节点



        for (i = 0; i < nr_dir_blks; i++)
        {
            RD_SECT(dir_inode->i_dev, dir_blk0_nr + i);

            pde = (struct dir_entry *)fsbuf;
            for (j = 0; j < SECTOR_SIZE / DIR_ENTRY_SIZE; j++, pde++)
            {
                if (pde->inode_nr == 0)
                    continue;
                if (pde->type == 'd')
                {
                    char t[128];
                    char tempPath[128];
                    char newPath[128];
                    memcpy(t,pathName,128);
                    int len=strlen(t);
                    t[len]=0;
                    memcpy(tempPath,strcat(t,pde->name),128);
                    len=strlen(tempPath);
                    tempPath[len]=0;
                    memcpy(newPath,strcat(tempPath,"/"),128);
                    len=strlen(newPath);
                    newPath[len]=0;

     //               printl("  %s     %s     %s    %s\n", t,tempPath,newPath , pathName);
                    PUSH(stack,newPath);
                }
                else
                {
                    if(strcmp(pde->name,_FILENAME)==0){
                        printl("  %2d     %s%s\n", pde->inode_nr ,pathName, pde->name);
                    }
                }
                if (++m >= nr_dir_entries)
                {
                    printl("\n");
                    break;
                }
            }
            if (m > nr_dir_entries) //[> all entries have been iterated <]
                break;
        }
    }


    printl("============================\n");
	return 0;
}

