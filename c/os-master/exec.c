#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "defs.h"
#include "x86.h"
#include "elf.h"

void
initPagesData(struct page *pagesBackup, int *countersBackup, int *queueBackup, int *offsetQueueBackup)
{
  struct proc *curproc = myproc();
  int i;
  for (i = 0; i < MAX_TOTAL_PAGES; ++i) {
    // backup proc pages structure
    pagesBackup[i].file_offset = curproc->pages[i].file_offset;
    pagesBackup[i].is_in_RAM = curproc->pages[i].is_in_RAM;
    pagesBackup[i].v_address = curproc->pages[i].v_address;
    pagesBackup[i].isAllocated = curproc->pages[i].isAllocated;

    // clear proc pages structure before exec 
    curproc->pages[i].v_address = 0;
    curproc->pages[i].file_offset = -1;
    curproc->pages[i].is_in_RAM = 0;
    curproc->pages[i].isAllocated = 0;
  }

  for (i = 0; i < MAX_PSYC_PAGES; ++i) {
    // backup the page's queues
    queueBackup[i] = curproc->RAMQueue[i];
    offsetQueueBackup[i] = curproc->offsetQueue[i];
    // clear the page's queues (actual RAM and swapping)
    curproc->RAMQueue[i] = -1;
    curproc->offsetQueue[i] = -1;
  }

  // backup proc page counters before exec (for case of failure)
  countersBackup[0] = curproc->fileOffset;
  countersBackup[1] = curproc->numberOfPagedOut;
  countersBackup[2] = curproc->numberOfPageFaults;
  countersBackup[3] = curproc->totalNumberOfPagedOut;
  countersBackup[4] = curproc->numberOfAllocatedPages;

  // clear the proc page's counters
  curproc->fileOffset = 0;
  curproc->numberOfPagedOut = 0;
  curproc->numberOfPageFaults = 0;
  curproc->totalNumberOfPagedOut = 0;
  curproc->numberOfAllocatedPages = 0;
}

// exec fail case
void
restoreFromBackup(struct page *pagesBackup, int *countersBackup, int *queueBackup, int *offsetQueueBackup)
{
  struct proc *curproc = myproc();
  int i;
  // restore proc pages 
  for (i = 0; i < MAX_TOTAL_PAGES; ++i) {
    curproc->pages[i].v_address = pagesBackup[i].v_address;
    curproc->pages[i].file_offset = pagesBackup[i].file_offset;
    curproc->pages[i].is_in_RAM = pagesBackup[i].is_in_RAM;
    curproc->pages[i].isAllocated = pagesBackup[i].isAllocated;
  }
  for (i = 0 ; i < MAX_PSYC_PAGES; ++i) {
    curproc->RAMQueue[i] = queueBackup[i];
    curproc->offsetQueue[i] = offsetQueueBackup[i];
  }

  // restore proc page indexs
  curproc->fileOffset = countersBackup[0] ;
  curproc->numberOfPagedOut = countersBackup[1];
  curproc->numberOfPageFaults = countersBackup[2];
  curproc->totalNumberOfPagedOut = countersBackup[3];
  curproc->numberOfAllocatedPages = countersBackup[4];
}

int
exec(char *path, char **argv)
{
  char *s, *last;
  int i, off;
  uint argc, sz, sp, ustack[3+MAXARG+1];
  struct elfhdr elf;
  struct inode *ip;
  struct proghdr ph;
  pde_t *pgdir, *oldpgdir;
  struct proc *curproc = myproc();

  begin_op();

  if((ip = namei(path)) == 0){
    end_op();
    cprintf("exec: fail\n");
    return -1;
  }
  ilock(ip);
  pgdir = 0;

#if (defined(SCFIFO) || defined(NFUA) || defined(AQ) || defined(LAPA))
  struct page pagesBackup[MAX_TOTAL_PAGES];
  int countersBackup[5];
  int queueBackup[MAX_PSYC_PAGES];
  int offsetQueueBackup[MAX_PSYC_PAGES];
  initPagesData(pagesBackup, countersBackup, queueBackup, offsetQueueBackup);
#endif

  // Check ELF header
  if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
    goto bad;
  if(elf.magic != ELF_MAGIC)
    goto bad;

  if((pgdir = setupkvm()) == 0)
    goto bad;

  // Load program into memory.
  sz = 0;
  for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
    if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
      goto bad;
    if(ph.type != ELF_PROG_LOAD)
      continue;
    if(ph.memsz < ph.filesz)
      goto bad;
    if(ph.vaddr + ph.memsz < ph.vaddr)
      goto bad;
    if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
      goto bad;
    if(ph.vaddr % PGSIZE != 0)
      goto bad;
    if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
      goto bad;
  }
  iunlockput(ip);
  end_op();
  ip = 0;

  // Allocate two pages at the next page boundary.
  // Make the first inaccessible.  Use the second as the user stack.
  sz = PGROUNDUP(sz);
  if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
    goto bad;
  clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
  sp = sz;

  // Push argument strings, prepare rest of stack in ustack.
  for(argc = 0; argv[argc]; argc++) {
    if(argc >= MAXARG)
      goto bad;
    sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
    if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
      goto bad;
    ustack[3+argc] = sp;
  }
  ustack[3+argc] = 0;

  ustack[0] = 0xffffffff;  // fake return PC
  ustack[1] = argc;
  ustack[2] = sp - (argc+1)*4;  // argv pointer

  sp -= (3+argc+1) * 4;
  if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
    goto bad;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(curproc->name, last, sizeof(curproc->name));

  // Commit to the user image.
  oldpgdir = curproc->pgdir;
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;

// change to the new swap file
#if (defined(SCFIFO) || defined(NFUA) || defined(AQ) || defined(LAPA))
  if(curproc-> pid > DEF_PROCS)
  {
    removeSwapFile(curproc);
    createSwapFile(curproc);
  }
#endif

  switchuvm(curproc);
  freevm(oldpgdir);
  return 0;

 bad:

#if (defined(SCFIFO) || defined(NFUA) || defined(AQ) || defined(LAPA))
  restoreFromBackup(pagesBackup , countersBackup, queueBackup, offsetQueueBackup);
#endif
  if(pgdir)
    freevm(pgdir);
  if(ip){
    iunlockput(ip);
    end_op();
  }
  return -1;
}
