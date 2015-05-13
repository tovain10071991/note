/*
 *	Hello! I'm child!
 *	Hello! I'm parent!
 *	%eip : 0xb7fdf0d0	-> ld.so��ڵ�ַ��0x10d0��gdb�鿴װ�ػ�ַ0xb7fe000������Ӧ����ͣ��ld.so��ڴ�
 *	a.out  main.c  main.c~
 *	
 */

#include <sys/ptrace.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/user.h>

int main()
{
	int pid;
	struct user_regs_struct regs;
	if((pid=fork())==0)
	{
		ptrace(PTRACE_TRACEME, 0, 0, 0);
		printf("Hello! I'm child!\n");
		execl("/bin/ls", "ls", NULL);
	}
	else
	{
		wait(NULL);
		printf("Hello! I'm parent!\n");
		ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		printf("%%eip : 0x%.8lx\n", regs.eip);
		ptrace(PTRACE_CONT, pid, 0, 0);
		wait(NULL);
	}
	return 0;
}