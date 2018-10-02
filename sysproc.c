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

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

// Calee Fulling ISU-f2018

int sys_system_load(void) {
  struct proc *p;
  struct system_info *u;

  if (argptr(0, (char **) &u, sizeof(struct system_info)) < 0)
    return -1;

  acquire(&ptable.lock);

  u->num_procs = 0;
  u->uvm_used = 0;

  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (p->state != UNUSED) {
      u->num_procs++;   // counting number of processes as it goes through
      u->uvm_used += p->sz; // adding amount of memory as it goes through
    }
  }

  u->num_cpus = ncpu;

  release(&ptable.lock);

  return 0;
}

struct {
  int zz_kern;
  int kern_kernext;
  int kernext_kernphys;
  int kernphys_dev;
  int dev_zF;
} test;

void page_dir_dump_p(void) {
  cprintf("*printing in dir_dump ...*\n");
}

int sys_page_dir_dump(void) {
  int flags;
  char * x;

  struct proc *curproc = myproc();
  pde_t *pd = curproc->pgdir; 		// getting page table of current process
  char * k = curproc->kstack;			// getting bottom of kernel stack for this process

  if (argptr(0, (char **)&x, sizeof(*x) < 0))	// checking parameters
    return -1;
  if (argint(1, &flags) < 0)
    return -1;

  int pagedir = PDX(x); // getting first 10 bits for page dir
  int pagetbl = PTX(x); // getting next 10 bits for page table
  int pageoff = (uint)x & 0xfff;  // getting next 12 bits, the offset

  int vadd = PGADDR(pagedir, pagetbl, pageoff); // constructing virtual address

  pde_t pde_entry = pd[pagedir];
  pde_t pg_table_addr = PTE_ADDR(pde_entry); 		// getting page table address
  pde_t pdeflags = PTE_FLAGS(pde_entry);		 		// getting flags from pt address

  if (flags == PAGE_DIR_DUMP_POINTER) { 				// -p case
    cprintf("-- tracing lookup of pointer --\n");
    cprintf("virtual address:      0x%x\n..\n", vadd);

    cprintf("page directory index: 0x%x\n", pagedir);
    cprintf("page table index:     0x%x\n", pagetbl);
    cprintf("offset in page:       0x%x\n..\n", pageoff);

    cprintf("page directory entry: 0x%x\n", pde_entry);
    cprintf("pde flags:            0x%x\n", pdeflags);
    cprintf("  pde flags: ");						// iterating through the pd's flags
      if (pde_entry & PTE_P)		  			// checking for present flag
        cprintf("present, ");
      if (pde_entry & PTE_W)	  				// checking for writeable flag
        cprintf("writeable, ");
      if (pde_entry & PTE_U)  					// checking for user flag
        cprintf("user, ");
      if (pde_entry & PTE_PWT)	  			// checking for write-through flag
        cprintf("write-through, ");
      if (pde_entry & PTE_PCD)  				// checking for cache-disable flag
        cprintf("cache-disable, ");
      if (pde_entry & PTE_A)  					// checking for accessed flag
        cprintf("accessed, ");
      if (pde_entry & PTE_D)					  // checking for dirty flag
        cprintf("dirty, ");
      if (pde_entry & PTE_PS)					  // checking for page size flag
        cprintf("page size, ");
      if (pde_entry & PTE_MBZ)				  // checking for bits must be zero flag
        cprintf("bits must be zero, ");
      cprintf("\n");
    cprintf("page table address:   0x%x\n..\n", pg_table_addr);

    pde_t * pte_entry = P2V(pg_table_addr);
    pde_t pte_entry2 = pte_entry[pagetbl];

    cprintf("page table entry:     0x%x\n", pte_entry2);

    pde_t pteflags = PTE_FLAGS(pte_entry2);
    cprintf("pte flags:            0x%x\n", pteflags);
    cprintf(" pde flags: ");			// checking more flags, this time the pt's
      if (pte_entry2 & PTE_P)
        cprintf("present, ");
      if (pte_entry2 & PTE_W)
        cprintf("writeable, ");
      if (pte_entry2 & PTE_U)
        cprintf("user, ");
      if (pte_entry2 & PTE_PWT)
        cprintf("write-through, ");
      if (pte_entry2 & PTE_PCD)
        cprintf("cache-disable, ");
      if (pte_entry2 & PTE_A)
        cprintf("accessed, ");
      if (pte_entry2 & PTE_D)
        cprintf("dirty, ");
      if (pte_entry2 & PTE_PS)
        cprintf("page size, ");
      if (pte_entry2 & PTE_MBZ)
        cprintf("bits must be zero, ");
      cprintf("\n");
    pde_t pg_table_addr2 = PTE_ADDR(pte_entry2);		// getting page table address
    cprintf("page address:         0x%x\n..\n", pg_table_addr2);
    cprintf("physical location:    0x%x\n", pg_table_addr2 | pageoff); // not sure if this is correct
    cprintf("value at location:    0x%x\n\n",*x);
    exit();
  }

  if (flags == PAGE_DIR_DUMP_TABLES) { // -t case
    int countPTP = 0;
    int countPT = 0;

    int countPDE1 = 0;
    int countPDE2 = 0;
    int countPDE3 = 0;
    int countPDE4 = 0;
    int countPDE5 = 0;

    cprintf("PDE's for virtual addresses below KERNBASE -\n");
    for (int i = 0; i < NPDENTRIES; i++) {   							// going through 1024 times
      pde_entry = pd[i];																	// getting a new pd entry each time
      pde_t pde_entry_flags = PTE_FLAGS(pde_entry);  			// getting entry's flags
      if (pde_entry_flags & PTE_P) {											// checking if present flag is there
	      countPTP++;
	      if (i < 512) {																		// checking if less than KERNBASE
	        cprintf(" PDE: row 0x%x (so virtual address 0x%x000), ", i, i);
	        cprintf("pde 0x%x, ", pde_entry);
	        cprintf("page at 0x%x\n", (pde_entry - pde_entry_flags) + KERNBASE); // not sure if this is how to do it
	        for (int j = 0; j < NPTENTRIES; j++) { 					// going through 1024 times
	          pde_t pg_table_addr = PTE_ADDR(pde_entry);		// getting pt address from pd entry each time
	          pde_t * pte_entry = P2V(pg_table_addr);
	          pde_t pte_entry2 = pte_entry[j];							// getting a new pt entry each time
	          pde_t pteflags = PTE_FLAGS(pte_entry2);       // getting the pt's flags
	          if (pteflags & PTE_P) {                       // if there is a present flag,
	            cprintf("  PTE: row 0x%x (so virtual address 0x%x000), ", j, j);
	            cprintf("pte 0x%x, ", pte_entry2);
	            cprintf("physical page at 0x%x\n", pte_entry2 - pteflags);
	            countPT++;
	          }
	        }
	      }
	      if (i >= 0 && i <= 511)							// 0x00000000 - KERNBASE
          countPDE1++;
	      if (i == 512)												// KERNBASE - KERNBASE+EXTMEM
          countPDE2++;
        if (i >= 513 && i <= 567)						// KERNBASE+EXTMEM - KERNBASE+PHYSTOP
          countPDE3++;
        if (i >= 568 && i <= 1015)					// KERNBASE+PHYSTOP - DEVSPACE
          countPDE4++;
        if (i >= 1016 && i <= NPDENTRIES)		// DEVSPACE - 0xFFFFFFFF
          countPDE5++;
      }
    }
    cprintf("  - %d pages present in that PT\n", countPT);

    cprintf("\nVirtual memory summary ...\n");
    cprintf("%d page tables present, %d pages total.\n", countPTP, countPT);

    cprintf("0x0 (0x00000000) - 0x%x (KERNBASE), ", KERNBASE);
    cprintf("#pdes = %d (out of %d total possible)\n", countPDE1, 512);

    cprintf("0x%x (KERNBASE) - 0x%x (KERNBASE+EXTMEM), ", KERNBASE, KERNBASE+EXTMEM);
    cprintf("#pdes = %d (out of %d total possible)\n", countPDE2, 0);

    cprintf("0x%x (KERNBASE+EXTMEM) - 0x%x (KERNBASE+PHYSTOP), ", KERNBASE+EXTMEM, KERNBASE+PHYSTOP);
    cprintf("#pdes = %d (out of %d total possible)\n", countPDE3, 55);

    cprintf("0x%x (KERNBASE+PHYSTOP) - 0x%x (DEVSPACE), ", KERNBASE+PHYSTOP, DEVSPACE);
    cprintf("#pdes = %d (out of %d total possible)\n", countPDE4, 448);

    cprintf("0x%x (DEVSPACE) - 0xfffffffff (0xFFFFFFFF), ", DEVSPACE);
    cprintf("#pdes = %d (out of %d total possible)\n", countPDE5, 7);

    cprintf("\nBottom of kernel stack for process: 0x%p\n", k);  // k is from struct myproc
  }

  if (flags == PAGE_DIR_DUMP_FREE) { // f case
    cprintf("printing in -f case\n");
  }
  return 0;
}

