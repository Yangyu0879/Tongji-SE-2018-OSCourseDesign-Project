
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
#include "movebox.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
    disp_str("-----\"kernel_main\" begins-----\n");

    struct task* p_task;
    struct proc* p_proc= proc_table;
    char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
    // u16   selector_ldt = SELECTOR_LDT_FIRST;
    u8    privilege;
    u8    rpl;
    int   eflags;
    int   i, j;
    int   prio;
    for (i = 0; i < NR_TASKS+NR_PROCS; i++)
    {
        if (i >= NR_TASKS + NR_NATIVE_PROCS) {
            p_proc->p_flags = FREE_SLOT;
            p_proc++;
            p_task++;
            continue;
        }
        if (i < NR_TASKS)
        {     /* 任务 */
            p_task    = task_table + i;
            privilege = PRIVILEGE_TASK;
            rpl       = RPL_TASK;
            eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1   1 0010 0000 0010(2)*/
            prio      = 15;     //设定优先级为15
        }
        else
        {                  /* 用户进程 */
            p_task    = user_proc_table + (i - NR_TASKS);
            privilege = PRIVILEGE_USER;
            rpl       = RPL_USER;
            eflags    = 0x202; /* IF=1, bit 2 is always 1              0010 0000 0010(2)*/
            prio      = 5;     //设定优先级为5
        }

        strcpy(p_proc->name, p_task->name); /* 设定进程名称 */
        p_proc->p_parent = NO_TASK;

        if (strcmp(p_task->name, "Init") != 0) {
            p_proc->ldts[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CS >> 3];
            p_proc->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

            /* change the DPLs */
            p_proc->ldts[INDEX_LDT_C].attr1  = DA_C   | privilege << 5;
            p_proc->ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
        }
        else {      /* INIT process */
            unsigned int k_base;
            unsigned int k_limit;
            int ret = get_kernel_map(&k_base, &k_limit);
            assert(ret == 0);
            init_descriptor(&p_proc->ldts[INDEX_LDT_C],
                  0, /* bytes before the entry point
                      * are useless (wasted) for the
                      * INIT process, doesn't matter
                      */
                  (k_base + k_limit) >> LIMIT_4K_SHIFT,
                  DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);

            init_descriptor(&p_proc->ldts[INDEX_LDT_RW],
                  0, /* bytes before the entry point
                      * are useless (wasted) for the
                      * INIT process, doesn't matter
                      */
                  (k_base + k_limit) >> LIMIT_4K_SHIFT,
                  DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
        }

        p_proc->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;
        p_proc->regs.ds =
            p_proc->regs.es =
            p_proc->regs.fs =
            p_proc->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
        p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

        p_proc->regs.eip = (u32)p_task->initial_eip;
        p_proc->regs.esp = (u32)p_task_stack;
        p_proc->regs.eflags = eflags;

        /* p_proc->nr_tty       = 0; */

        p_proc->p_flags = 0;
        p_proc->p_msg = 0;
        p_proc->p_recvfrom = NO_TASK;
        p_proc->p_sendto = NO_TASK;
        p_proc->has_int_msg = 0;
        p_proc->q_sending = 0;
        p_proc->next_sending = 0;
        p_proc->pid = i;

        for (j = 0; j < NR_FILES; j++)
            p_proc->filp[j] = 0;

        p_proc->ticks = p_proc->priority = prio;

        p_task_stack -= p_task->stacksize;
        p_proc++;
        p_task++;
        // selector_ldt += 1 << 3;
    }

        /* proc_table[NR_TASKS + 0].nr_tty = 0; */
        /* proc_table[NR_TASKS + 1].nr_tty = 1; */
        /* proc_table[NR_TASKS + 2].nr_tty = 1; */

    k_reenter = 0;
    ticks = 0;

    p_proc_ready = proc_table;

    init_clock();
        init_keyboard();

    restart();

    while(1){}
}








/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}


/*======================================================================*
                               TestA
 *======================================================================*/

//1号终端
void TestA()
{	
	char tty_name[] = "/dev_tty0";//表明进程

	int fd_stdin = open(tty_name, O_RDWR);//启动进程
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	char readBuffer[256];//缓冲区大小
	char command[20];//命令大小
	char filename[128];//文件名称
	int pidPoul[10]={0};
	int login=0;

	sl();//开机动画

	clear();

	printf("                      ===============================================\n");
	printf("                       ____   ____   ____\\_ |__  __ _____  ___\n");
	printf("                      /    \\ /  _ \\ /  _ \\| __ \\|  |  \\  \\/  /\n");
	printf("                     |   |  (  <_> |  <_> ) \\_\\ \\  |  />    < \n");
	printf("                     |___|  /\\____/ \\____/|___  /____//__/\\_ \\\n");
	printf("                          \\/                  \\/            \\/\n");
	printf("                      ===============================================\n");

	//help();

	char currentFolder[512] = "/";  // 记录当前路径（其实路径字符长度上限为MAX_PATH）
    
       char abs_path[512];
    
       convert(abs_path, currentFolder, "protect");
    
       int fd = open(abs_path, O_RDWR);
    
       if(fd==-1)mkdir("/protect");
    
       convert(abs_path, currentFolder, "bin");
    
       fd = open(abs_path, O_RDWR);
    
       if(fd==-1)mkdir("/bin");
    
       convert(abs_path, currentFolder, "password");
    
       fd = open(abs_path, O_RDWR);
    
       if(fd==-1)new_file(currentFolder, "password");

	while (1)//不断循环的操作
	{
		if (login == 0) {
			int res = login11();
			while (res != 1) {
				res = login11();
			}
			login = 1;
			res = 0;
		}
		printf("[root@localhost: %s]", currentFolder);  // 打印当前路径

		int r = read(fd_stdin, readBuffer, 512);//读入 r为大小
		readBuffer[r] = '\0';


		//解析命令
		int pos = 0;
		while (readBuffer[pos] != ' ' && readBuffer[pos] != '\0')  // 读取指令这一字符串，以空格为分隔符
		{
			command[pos] = readBuffer[pos];
			pos++;
		}
		command[pos] = '\0';//末尾添加一个0作为结束符
		if (readBuffer[pos] != '\0')  // 指令还未结束
		{
			pos++;
			int len = pos;
			while (readBuffer[pos] != ' ' && readBuffer[pos] != '\0')  // 读取操作文件名
			{
				filename[pos - len] = readBuffer[pos];
				pos++;
			}
			filename[pos - len] = '\0';
		}

		if (strcmp(command, "process") == 0)//开始比较指令内容
		{
			ProcessManage(pidPoul);
		}
		else if (strcmp(command, "clear") == 0)
		{
			clear();
			printf("                      ===============================================\n");
			printf("                       ____   ____   ____\\_ |__  __ _____  ___\n");
			printf("                      /    \\ /  _ \\ /  _ \\| __ \\|  |  \\  \\/  /\n");
			printf("                     |   |  (  <_> |  <_> ) \\_\\ \\  |  />    < \n");
			printf("                     |___|  /\\____/ \\____/|___  /____//__/\\_ \\\n");
			printf("                          \\/                  \\/            \\/\n");
			printf("                      ===============================================\n");
		}
		else if (strcmp(command, "help") == 0)
		{
			help();
		}
		else if (strcmp(command, "filemanage") == 0)
		{
			printf("File Manager has already run on CONSOLE-1 ! \n");
			continue;
		}
		else if (strcmp(command, "ls") == 0)
		{
			ls(currentFolder);
		}
        else if (strcmp(command, "find") == 0)
		{
			find_file(filename);
		}
		else if (strcmp(command, "touch") == 0)  // 创建文件
		{
		    if(strcmp(currentFolder,"/bin/")==0){
                      printf("can't create dir in the bin\n");
                    }
                    else{
	               new_file(currentFolder, filename);
		     }
		}
        else if (strcmp(command, "rmfile") == 0)  // 删除文件
        {
            delete_file(currentFolder, filename);
        }
        else if (strcmp(command, "rsfile") == 0)  
        {
            restore_file(currentFolder, filename);
        }
        else if (strcmp(command, "rmdir") == 0)  // 删除目录
        {
            delete_node(currentFolder, filename);
        }
		else if (strcmp(command, "cat") == 0)  // 打印文件内容
		{
			read_file(currentFolder, filename);
		}
		else if (strcmp(command, "vi") == 0)  // 写文件
		{
			if(strcmp(currentFolder,"/bin/")==0){
                      printf("can't write file in the bin\n");
                    }
                    else{
	               write_file(currentFolder, filename);
		     }
		}
		else if (strcmp(command, "mkdir") == 0)  // 创建目录
		{
			new_dir(currentFolder, filename);
		}
		else if(strcmp(command,"clearbin")==0){
                       clear_bin();
                }
		else if (strcmp(command, "cd") == 0)
		{
			go_dir(currentFolder, filename);
		}
		else if (strcmp(command, "runminesweep") == 0)
		{
			Minesweeper1(fd_stdin);
		}
		else if (strcmp(command, "runmovebox") == 0)
		{
			MoveBox(fd_stdin);
		}
		else if (strcmp(command, "password") == 0)
        {
            password();
        }
		else if (strcmp(command, "getpid") == 0) //进程相关
		{
			printf(asm_strcat(getpid(), "\n"));
			printi(getpid());
			printf("\n");
			printi(new_getpid());
			printf("\n");
		}
		else if (strcmp(command, "fork") == 0)
		{
			int pid = fork();
		}
		else if (strcmp(command, "kill") == 0)
		{
			int pid = filename[0] - 48;
			kill_proc(pid, pidPoul);
			ProcessManage(pidPoul);
		}
		else if (strcmp(command, "block") == 0)
		{
			int pid = filename[0] - 48;
			block_proc(pid, pidPoul);
			ProcessManage(pidPoul);
		}
		else if (strcmp(command, "unblock") == 0)
		{
			int pid = filename[0] - 48;
			unblock_proc(pid, pidPoul);
			ProcessManage(pidPoul);
		}
        else if (strcmp(command, "logout") == 0)
        {
        clear();
        login = 0;
        }
		else
			printf("Command not found, please check!\n");
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
//二号终端
void TestB()
{
    char tty_name[] = "/dev_tty1";

    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin  == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);
    char rdbuf[128];
    while (1)
    {
    	printf("$ ");
    	int r = read(fd_stdin, rdbuf, 70);
    }
    assert(0); /* never arrive here */
}


void TestC()
{
    spin("TestC");
}


/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
    int i;
    char buf[256];

    /* 4 is the size of fmt in the stack */
    va_list arg = (va_list)((char*)&fmt + 4);

    i = vsprintf(buf, fmt, arg);

    printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

    /* should never arrive here */
    __asm__ __volatile__("ud2");
}

/*****************************************************************************
 *                                clear
 *      系统调用clear_screen
 *      并重置      console_table[current_console].crtc_start
 *                  console_table[current_console].cursor
 *****************************************************************************/
void clear()
{
    clear_screen(0, console_table[current_console].cursor);
    console_table[current_console].crtc_start = 0;
    console_table[current_console].cursor = 0;

}

/*****************************************************************************
 *                                help
 *      显示help内容，提示用户命令
 *****************************************************************************/
void help()
{
    printf("===============================================================================\n");
    printf("*                              Command List:                                   \n");
    printf("*                        Please Enter the command!                             \n");
    printf("===============================================================================\n");
    printf("*1. process                | A Process Manager that display process information\n");
    printf("*2. clear                  | Clear the Screen                                  \n");
    printf("*3. help                   | Show the Commmand List                            \n");
    printf("*4. filemng                | Start the File Management                         \n");
    printf("*5. password               | Check the password to access file                 \n");
    printf("*6. ls                     | List all files in the current folder              \n");
    printf("*7. touch        [filename]| Create a new file in the current directory        \n");
    printf("*8. rmfile       [filename]| Delete a file in the current folder               \n");
    printf("*9. rsfile       [filename]| restore a file in the bin folder               \n");
    printf("*10. cat         [filename]| Output the content of a file in the current folder\n");
    printf("*11. vi          [filename]| Apppend new content at the end of the file        \n");
    printf("*12. mkdir     [foldername]| Create a new floder in the current folder         \n");
    printf("*13. cd        [foldername]| Open a folder in the current folder               \n");
    printf("*14. runminesweep          | Run MineSweep game                                \n");
    printf("*15. runmovebox            | Run MoveBox game                                  \n");
    printf("*16. clearbin              | Clear the bin                                     \n");
    printf("*17. rmdir                 | Delete a directory in the current folder          \n");
    printf("*18. find        [filename]| Find the file with the input name          \n");
    printf("*19. logout                | Exit current account          \n");
    printf("===============================================================================\n");
}

void ProcessManage(int* pidPoul)
{
    int i;
    printf("-------------------------------------------------------------------------------\n");//进程号，进程名，优先级，是否是系统进程，是否在运行
    printf("      PID      |    name       | spriority    | running?\n");
    printf("-------------------------------------------------------------------------------\n");
    for ( i = 0 ; i < NR_TASKS + NR_PROCS ; i++ )//逐个遍历
    {
		if (proc_table[i].p_flags != FREE_SLOT)
		{
			if (pidPoul[i] == 1)
			{
				continue;
			}
			else if (pidPoul[i] == 2)
			{
				printf("        %d           %s            %d                NO\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority);
			}
			else if (pidPoul[i] == 0)
			{
				printf("        %d           %s            %d                %s\n", proc_table[i].pid, proc_table[i].name, proc_table[i].priority, proc_table[i].p_flags == FREE_SLOT ? "NO" : "YES");
			}
		}
    }
    printf("-------------------------------------------------------------------------------\n");
	printf("===============================================================================\n");
	printf("*                          Process Command List:                               \n");
	printf("*                        Please Enter the command!                             \n");
	printf("===============================================================================\n");
	printf("*1. kill              [pid]| Kill a process by its pid                         \n");
	printf("*2. block             [pid]| Block a process by its pid                        \n");
	printf("*3. unblock           [pid]| Unblock a process by its pid                      \n");
	printf("*4. fork                   | Fork a process                                    \n");
	printf("===============================================================================\n");
}

void kill_proc(int pid, int*pidPoul)
{
	if (pid >= NR_TASKS + NR_PROCS && pid < 0)
	{
		printf("                       Warnning:This pid is out of range                       \n");
	}
	else if (pidPoul[pid] == 1)
	{
		printf("                       Warnning:This pid has been killed                       \n");
	}
	else if (pidPoul[pid] == 0 || pidPoul[pid] == 2)
	{
		pidPoul[pid] = 1;
		printf("                         Process %s has been killed                            \n", proc_table[pid].name);
	}
}

void block_proc(int pid, int*pidPoul)
{
	if (pid >= NR_TASKS + NR_PROCS && pid < 0)
	{
		printf("                       Warnning:This pid is out of range                       \n");
	}
	else if (pidPoul[pid] == 1)
	{
		printf("                       Warnning:This pid has been killed                       \n");
	}
	else if (pidPoul[pid] == 2)
	{
		printf("                      Warnning:This pid has been blocked                       \n");
	}
	else if (pidPoul[pid] == 0)
	{
		pidPoul[pid] = 2;
		printf("                         Process %s has been blocked                           \n", proc_table[pid].name);
	}
}

void unblock_proc(int pid, int*pidPoul)
{
	if (pid >= NR_TASKS + NR_PROCS && pid < 0)
	{
		printf("                       Warnning:This pid is out of range                       \n");
	}
	else if (pidPoul[pid] == 1)
	{
		printf("                       Warnning:This pid has been killed                       \n");
	}
	else if (pidPoul[pid] == 0)
	{
		printf("                       Warnning:This pid is still running                      \n");
	}
	else if (pidPoul[pid] == 2)
	{
		pidPoul[pid] = 0;
		printf("                        Process %s has been unblocked                           \n", proc_table[pid].name);
	}
}

int login11()
{
    char buf[512];
    int fd = open("/password", O_RDWR);
    read(fd, buf, 512);

    char tty[] = "/dev_tty0";
    int input = open(tty, O_RDWR);

    char write_buf[4096];

    printf("login as root@localhost!\n");
    printf("Please enter the password:\n");
    int length = read(input, write_buf, 4096);
    write_buf[length] = 0;


    if (strcmp(buf, write_buf) == 0) {
        help();
        return 1;
    }
    else
        printf("wrong password!\n");
    return 0;


}
