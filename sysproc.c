#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
// ISU-f2018, added so can print inode information for debugging
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// ISU-f2018
void
mem_dump(void)
{
  //struct proc *p = myproc();
  // here!
  cprintf(" -- mem dump\n");
  
  cprintf(" mem dump --\n");
}

// ISU-f2018
void
proc_dump(void)
{
  struct proc *p = myproc();
  cprintf(" -- struct proc dump\n");
  cprintf(" sz\t%d\n",p->sz);
  cprintf(" pgdir\t%p\n",p->pgdir);
  cprintf(" kstack\t%p\n",p->kstack);
  cprintf(" state\t%d\n",p->state);
  cprintf(" pid\t%d\n",p->pid);
  if (p->parent != 0)
    cprintf(" parent\t%p, %s\n",p->parent, p->parent->name);
  else
    cprintf(" parent\t0\n");
  cprintf(" tf\t%p\n",p->tf);
  cprintf(" context\t%p\n",p->context);
  cprintf(" chan\t%p\n",p->chan);
  cprintf(" killed\t%d\n",p->killed);
  cprintf(" ofile\t%p\n",p->ofile);
  if (p->cwd != 0)
    cprintf(" cwd\t%p - dev %d, inum %d\n",p->cwd,p->cwd->dev,p->cwd->inum);
  else
    cprintf(" cwd\t0\n");
  cprintf(" name\t%s\n",p->name);
  cprintf(" struct proc dump --\n");
}


// ISU-f2018
int
sys_usage(void)
{
  // cprintf("Hello World.\n");
  struct proc_usage *u;
  if (argptr(0, (char **) &u, sizeof(struct proc_usage)) < 0)
    return -1;

  u->memory_size = myproc()->sz;

  // debug print proc state...
  if (1) {
    proc_dump();
    mem_dump();
  }
  return 0;
}

/*
int sys_load(void) {
  struct system_info *u;
  if (argptr(0, (char **) &u, sizeof(struct system_info)) < 0)
    return -1;
  cprintf("Hello World.\n");
  return 0;
}

static struct proc* test(void) {
  struct proc *p;
  int count = 0;
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    count++;
}
*/

int sys_load(void) { // test
  struct proc *p = myproc();
  struct system_info *u;
  if (argptr(0, (char **) &u, sizeof(struct system_info)) < 0)
    return -1;
  cprintf(" testing, sz = %d\n", p->sz);
  return 0;
}
