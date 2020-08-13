#include "type.h"
#include "config.h"
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
#include "hd.h"



PUBLIC void convert(char* dest, char* path, char* file)
{
    int i=0, j=0,m=0;
    while (path[i])  dest[j++] = path[i++];
        
    while (file[m]) dest[j++] = file[m++]; // 写入文件名
        

    dest[j] = 0;  // 结束符
}
/**************************************************
*创建新文件
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.再根据绝对路径去创建文件
*3.a如果失败则输出失败信息
*3.b如果成功则在文件末尾写上0保证字符串的正常终止
***************************************************/
void new_file(char* path, char* file)
{   
 
    //用于存放字符串的末尾
    char buf[1] = {0};
    
    //用于存储绝对路径
    char abs_path[512];
    //将地址和文件名转换为绝对路径
    convert(abs_path, path, file);

    //新建文件
    int fd = open(abs_path, O_CREAT | O_RDWR);
  
  
    //如果创建失败输出失败信息
    if (fd == -1){printf("counldnt create a new file named %s\n", file);return;}
    
    //在文件的末尾写0
    write(fd, buf, 1);
    
    //关闭文件
    close(fd);
    
    
    if(strcmp("/bin/",path)==0)printf("File has moved to the bin\n");
    
}


/**************************************************
*创建目录
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.直接根据文件路径打开文件夹
*3.通过mkdir函数在绝对路径位置创建新目录
***************************************************/
void new_dir(char* path, char* file)
{

    //存储绝对路径
    char abs_path[512];
    //改变为绝对路径
    convert(abs_path, path, file);
    
    //打开绝对路径下的文件
    int fd = open(abs_path, O_RDWR);
    
    //如果不能正常打开输出错误信息
    if (fd != -1) { printf("Failed to create a new directory with name %s\n", file);return;    }
   
    //在对应位置创建新目录
    mkdir(abs_path);
}


/**************************************************
*删除文件
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.直接根据文件路径删除文件
***************************************************/
void delete_file(char* path, char* file)
{  
    
    //用于存储绝对路径
    char abs_path[512];
    
    //转换为绝对路径
    convert(abs_path, path, file);
    if(strcmp(abs_path, "/password") == 0)
    {
       printf("you cant delete this file\n");
       return;
    }
     if(strcmp(abs_path,"/bin")==0){
       printf("you cant delete the bin\n");
       return;  
    }
    if(strcmp(path,"/bin/")==0){
    //删除文件，并根据删除是否成功输出对应信息
    	if (unlink(abs_path) == 0){printf("successfully deleted!\n");}    
    	else{printf("couldnt delete this file\n");}
    }
    else{ 
       
    char content[512];//content of the file
    //打开文件并返回文件描述符
    int fd = open(abs_path, O_RDWR);
    //如果文件不存在输出错误信息
    if (fd == -1) {printf("delete error\n");return; }
    //向buf中读取文件内容
    int m = read(fd, content, 512);  
    //如果文件读取失败输出错误信息
    if (m == -1){                      
        printf("couldnt restore the content of the file\n");   
        //关闭文件
        close(fd);
        return;
        }
      close(fd);
      
     //delete the file
    convert(abs_path, path, file);
    if (unlink(abs_path) == 0)printf("successfully deleted!\n");
    else printf("couldnt delete this file\n"); 
      
     new_file("/bin/",file);
          
    //rewrite the file
    char bin_path[512];
    convert(bin_path,"/bin/", file);
    int _fd = open(bin_path, O_RDWR);
    if (_fd == -1){printf("delete error");return;}       
    write(_fd, content, 512); 
    close(_fd);
      
    
    }     
}

void restore_file(char* path, char* file)
{  
    
    //用于存储绝对路径
    char abs_path[512];
    
    //转换为绝对路径
    convert(abs_path, path, file);
 
    if(strcmp(path,"/bin/")!=0){
    //删除文件，并根据删除是否成功输出对应信息
    	printf("you cannot restore the file out of the bin!\n");
    	return;
    }
    else{ 
       
    char content[512];//content of the file
    //打开文件并返回文件描述符
    int fd = open(abs_path, O_RDWR);
    //如果文件不存在输出错误信息
    if (fd == -1) {printf("restore error\n");return; }
    //向buf中读取文件内容
    int m = read(fd, content, 512);  
    //如果文件读取失败输出错误信息
    if (m == -1){                      
        printf("couldnt restore the content of the file\n");   
        //关闭文件
        close(fd);
        return;
        }
      close(fd);
      
    //restore the file
    convert(abs_path, path, file);
    if (unlink(abs_path) == 0);
    else printf("couldnt restore this file\n");
    
    new_file("/",file);
          
    char old_path[512];
    convert(old_path,"/", file);
    int _fd = open(old_path, O_RDWR);
    if (_fd == -1){printf("delete error");return;}       
    write(_fd, content, 512); 
    close(_fd);
      
    
    }     
}

/**************************************************
*删除目录
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.直接根据文件路径删除
***************************************************/
void delete_node(char* path, char* file)
{  
    //用于存储绝对路径
    char abs_path[512];
    
    //转换为绝对路径
    convert(abs_path, path, file);
    if(strcmp(abs_path, "/password") == 0)
    {
       printf("you cant delete this directory\n");
       return;
    }
    if(strcmp(abs_path,"/bin")==0){
       printf("you cant delete the bin\n");
       return;  
    }
    //删除文件，并根据删除是否成功输出对应信息
    if (unlink(abs_path) == 0)printf("successfully deleted!\n");
    
    else printf("couldnt delete this file\n");
    
}

void clear_bin()
{  
    
    char abs_path[512];
    convert(abs_path, "/", "bin");
    if (unlink(abs_path) == 0){
    printf("successfully clear the bin!\n");
    convert(abs_path, "/", "bin");
    int fd = open(abs_path, O_RDWR); 
    if(fd==-1)mkdir("/bin");
    } 
    else printf("couldnt delete this directory\n");  
}


/**************************************************
*前往目录
*                                   
*1.如果是要返回上一级，则先求出上一级的路径，再通过和空组成新的绝对路径达到返回上一级
*2.如果是进入下一级，则将路径和文件名直接组合成新的绝对路径即可
***************************************************/
void go_dir(char* path, char* file)
{    
    
    int flag = 0;  
    
    //保存新路径，因为可能出现倒退的情况，因此需要new_path记录上一级的目录
    char new_path[512] = {0};
    
    //对于指令cd..在DOS系统中一般用于返回上一级
    if (file[0] == '.' && file[1] == '.')
    {
        flag = 1;
        
        //在路径字符串中的字符位置，用于遍历
        int pos_path = 0;
        
        //在新的路径字符串的字符位置，用于遍历
        int pos_new = 0;
        
        //i用于temp的计数
        int i = 0;
        
        //temp作为中间变量保存过程中的文件目录名
        char temp[128] = {0}; 
        
        //如果没有到达字符串的结尾
        while (path[pos_path] != 0)
        {
        
            //未到达'/'时
            if (path[pos_path] != '/')
            {   
            //使用temp保存路径
                temp[i] = path[pos_path];
                
                i++;
                
                pos_path++;
                
               
            }
            
            //如果到达'/'时
            else
            {
                //递增计数
                pos_path++;
                
                //到达结尾时停止
                if (path[pos_path] == 0)break;
                 
                //如果不是结尾   
                else
                {
                
                    //对temp进行初始化
                    temp[i] = '/';
                    
                    //末尾置零
                    temp[i + 1] = 0;
                    
                    i = 0;
                    
                    //将temp中的全部复制到newPath中再把temp清除
                    while (temp[i] != 0)
                    {
                    
                        new_path[pos_new] = temp[i];
                        
                        temp[i] = 0;  
                        
                        pos_new++;
                        
                        i++;
                    }
                    i = 0;
                }
            }
        }
    }
    
    //存储绝对路径
    char abs_path[512];
    
    //存储文件名称
    char file_name[512];
    
    int pos = 0;
    
    //将文件名称复制过来
    while (file[pos] != 0){file_name[pos] = file[pos];pos++;   }
  
    //给文件末尾添加终止符号
    file_name[pos] = '/';    
    file_name[pos + 1] = 0;
    
    //返回上一级目录
    if (flag == 1) {file_name[0] = 0;convert(abs_path, new_path, file_name); }
   
   
    //进入下一级目录
    else  convert(abs_path, path, file_name);
    
    if(strcmp(abs_path, "/protect/")==0)
    {
      char buf1[512];
      int fd = open("/password", O_RDWR);
      read(fd, buf1, 512);
  
    char tty[] = "/dev_tty0";
    int input  = open(tty, O_RDWR);
    
    char write_buf[4096];  
    
    //length为读取的字符串长度,将输入读入写缓冲区中
    
    printf("please enter your password\n");
    int length = read(input, write_buf, 4096);
    
    //将末尾置0，保证字符串正常结束
    write_buf[length] = 0;
  
  
  if(strcmp(buf1, write_buf) != 0){printf("wrong password\n");return;}
 
    }
    
    //打开文件    
    int fd = open(abs_path, O_RDWR);
    
    //如果打开失败输出错误信息
    if (fd == -1)  printf("%s is not a directory!\n", abs_path);
    
    //将当前的绝对路径复制给所在目录的路径
    else memcpy(path, abs_path, 512);
        
}
/**************************************************
*读取文件
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.直接根据文件路径打开文件
*3.读取文件
***************************************************/
void read_file(char* path, char* file)
{

    //保存绝对路径
    char abs_path[512];
    char buf[512];
    
    //转换为绝对路径
    convert(abs_path, path, file);
   
    //打开文件并返回文件描述符
    int fd = open(abs_path, O_RDWR);

    //如果文件不存在输出错误信息
    if (fd == -1) {printf("couldnt read %s!\n", file);return; }
   
    if(strcmp(abs_path, "/password") == 0)
    {
    
       printf("you cant read this file\n");
       return;
    
    }
    //向buf中读取文件内容
    int m = read(fd, buf, 512);
    
    //如果文件读取失败输出错误信息
    if (m == -1)    {                    
  
        printf("couldnt read this file\n");
        
        //关闭文件
        close(fd);
        
        return;
    }
     
     
    //输出文件内容
    printf("%s\n", buf);
    
    //关闭文件
    close(fd);
}
/**************************************************
*写文件
*                                   
*1.先将路径和新建的文件名组合起来形成一个新的绝对路径
*2.直接根据文件路径打开文件
*3.指定需要的TTY
*4.通过TTY读入写缓冲区中
*5.将写缓冲区的内容写入文件
***************************************************/
void write_file(char* path, char* file)
{
    //保存绝对路径
    char abs_path[512];
    
    //转换为绝对路径
    convert(abs_path, path, file);
    
    //根据绝对路径进行打开
    int fd = open(abs_path, O_RDWR);
    
    //如果打开失败输出错误信息
    if (fd == -1){printf("couldnt open file: %s!\n", file);return;  }       
    
  
    //指定读写需要的TTY
    char tty[] = "/dev_tty0";
    
    //打开dev_tty0，由于是在文件系统开始就已经创建了这个文件，因此这个文件一定存在不需要判断是否存在
    int input  = open(tty, O_RDWR);
    
    char write_buf[4096];  
    
    //length为读取的字符串长度,将输入读入写缓冲区中
    int length = read(input, write_buf, 4096);
    
    //将末尾置0，保证字符串正常结束
    write_buf[length] = 0;
    
    //将写缓冲区中的内容写入fd对应的文件
    write(fd, write_buf, length + 1); 
    
    //关闭文件
    close(fd);
}



void password()
{
  char buf[512];
  int fd = open("/password", O_RDWR);
  read(fd, buf, 512);
  
    char tty[] = "/dev_tty0";
    int input  = open(tty, O_RDWR);
    
    char write_buf[4096];  
    
    //length为读取的字符串长度,将输入读入写缓冲区中
    
    printf("please enter your password\n");
    int length = read(input, write_buf, 4096);
    
    //将末尾置0，保证字符串正常结束
    write_buf[length] = 0;
  
  
  if(strcmp(buf, write_buf) == 0){
  printf("please enter new password\n");
  write_file("/","password");}
  else
  printf("wrong password\n");
  
  
}

void find_file(char* filename){


    char root[128]="/";
    do_find(filename);
}

