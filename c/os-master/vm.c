#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

extern char data[];  // defined by kernel.ld
pde_t *kpgdir;  // for use in scheduler()

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all those PTE_P bits are zero.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned.
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  char *a, *last;
  pte_t *pte;

  a = (char*)PGROUNDDOWN((uint)va);
  last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
  for(;;){
    if((pte = walkpgdir(pgdir, a, 1)) == 0)
      return -1;
    if(*pte & PTE_P)
      panic("remap");
    *pte = pa | perm | PTE_P;
    if(a == last)
      break;
    a += PGSIZE;
    pa += PGSIZE;
  }
  return 0;
}

// There is one page table per process, plus one that's used when
// a CPU is not running any process (kpgdir). The kernel uses the
// current process's page table during system calls and interrupts;
// page protection bits prevent user code from using the kernel's
// mappings.
//
// setupkvm() and exec() set up every page table like this:
//
//   0..KERNBASE: user memory (text+data+stack+heap), mapped to
//                phys memory allocated by the kernel
//   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
//   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
//                for the kernel's instructions and r/o data
//   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
//                                  rw data + free physical memory
//   0xfe000000..0: mapped direct (devices such as ioapic)
//
// The kernel allocates physical memory for its heap and for user memory
// between V2P(end) and the end of physical memory (PHYSTOP)
// (directly addressable from end..P2V(PHYSTOP)).

// This table defines the kernel's mappings, which are present in
// every process's page table.
static struct kmap {
  void *virt;
  uint phys_start;
  uint phys_end;
  int perm;
} kmap[] = {
 { (void*)KERNBASE, 0,             EXTMEM,    PTE_W}, // I/O space
 { (void*)KERNLINK, V2P(KERNLINK), V2P(data), 0},     // kern text+rodata
 { (void*)data,     V2P(data),     PHYSTOP,   PTE_W}, // kern data+memory
 { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W}, // more devices
};

// Set up kernel part of a page table.
pde_t*
setupkvm(void)
{
  pde_t *pgdir;
  struct kmap *k;

  if((pgdir = (pde_t*)kalloc()) == 0)
    return 0;
  memset(pgdir, 0, PGSIZE);
  if (P2V(PHYSTOP) > (void*)DEVSPACE)
    panic("PHYSTOP too high");
  for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
    if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
                (uint)k->phys_start, k->perm) < 0) {
      freevm(pgdir);
      return 0;
    }
  return pgdir;
}

// Allocate one page table for the machine for the kernel address
// space for scheduler processes.
void
kvmalloc(void)
{
  kpgdir = setupkvm();
  switchkvm();
}

// Switch h/w page table register to the kernel-only page table,
// for when no process is running.
void
switchkvm(void)
{
  lcr3(V2P(kpgdir));   // switch to the kernel page table
}

// Switch TSS and h/w page table to correspond to process p.
void
switchuvm(struct proc *p)
{
  if(p == 0)
    panic("switchuvm: no process");
  if(p->kstack == 0)
    panic("switchuvm: no kstack");
  if(p->pgdir == 0)
    panic("switchuvm: no pgdir");

  pushcli();
  mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
                                sizeof(mycpu()->ts)-1, 0);
  mycpu()->gdt[SEG_TSS].s = 0;
  mycpu()->ts.ss0 = SEG_KDATA << 3;
  mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
  // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
  // forbids I/O instructions (e.g., inb and outb) from user space
  mycpu()->ts.iomb = (ushort) 0xFFFF;
  ltr(SEG_TSS << 3);
  lcr3(V2P(p->pgdir));  // switch to process's address space
  popcli();
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;

  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if((uint) addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, P2V(pa), offset+i, n) != n)
      return -1;
  }
  return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if(newsz >= KERNBASE)
    return 0;
  if(newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE){

#if (defined(SCFIFO) || defined(NFUA) || defined(LAPA) || defined(AQ))
    if((myproc()->pid > DEF_PROCS) && ( a >= PGSIZE * MAX_PSYC_PAGES)) {
      swapToFile(pgdir);
    }
#endif

    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz, 0);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){ // perm = writeable, user
      cprintf("allocuvm out of memory (2)\n");
      deallocuvm(pgdir, newsz, oldsz, 0);
      kfree(mem);
      return 0;
    }

#if (defined(SCFIFO) || defined(NFUA) || defined(LAPA) || defined(AQ))
    struct proc* currproc = myproc();
    pte_t *pte;
    if(currproc->pid > DEF_PROCS)
    {
      int pageIndex = 0;
      while((pageIndex<MAX_TOTAL_PAGES) && (currproc->pages[pageIndex].isAllocated == 1))
        pageIndex++;

      currproc->pages[pageIndex].isAllocated = 1;
      currproc->pages[pageIndex].v_address = a; //  a = PGROUNDUP(oldsz);
      currproc->pages[pageIndex].is_in_RAM = 1; // puts the new page inside the RAM istead of inside the swap file 
      currproc->pages[pageIndex].file_offset = -1;

      insert(pageIndex);

      currproc->numberOfAllocatedPages++;

      pte = walkpgdir(pgdir, (char *)a , 0);
      *pte=PTE_P_ON(*pte);
      *pte=PTE_PG_OFF(*pte);
    }
#endif
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz, int flag)
{
  pte_t *pte;
  uint a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
      a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
    else if((*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
#if (defined(SCFIFO) || defined(NFUA) || defined(AQ) || defined(LAPA))
      if(myproc()->pid > DEF_PROCS && flag == 1)
        deallocatePage(a);
#endif

      char *v = P2V(pa);
      kfree(v);
      *pte = 0;
    } else {
#if (defined(SCFIFO) || defined(NFUA) || defined(AQ) || defined(LAPA))
      if((myproc()->pid > DEF_PROCS) && ((*pte & PTE_PG) != 0)){
        pa = PTE_ADDR(*pte);
        if(pa == 0)
          panic("kfree");
        if(myproc()->pid > DEF_PROCS && flag == 1)
        deallocatePage(a);
        *pte = 0;
      }
#endif
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0, 0);
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * v = P2V(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if(pte == 0)
    panic("clearpteu");
  *pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;
  // char *mem;

  if((d = setupkvm()) == 0)
    return 0;
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P)){
      if(!(*pte & PTE_PG))
        panic("copyuvm: page not present");
      continue;
    }
    pa = PTE_ADDR(*pte);

    if (*pte & PTE_W){
      *pte |= PTE_C; // PTE_C = 1 --> while page fault change to writeable
      *pte &= ~PTE_W; // PTE_W = 0  --> read only
    }
    flags = PTE_FLAGS(*pte);
   
    // if((mem = kalloc()) == 0)
    //   goto bad;
    // memmove(mem, (char*)P2V(pa), PGSIZE);
    // if(mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0)
    //   goto bad;
    if(mappages(d, (void*)i, PGSIZE, pa, flags) < 0) { 
    //   // kfree(mem);
      goto bad;
    }
    inc_ref_count(pa);
    lcr3(V2P(myproc()->pgdir)); 
  }
  return d;

bad:
  freevm(d);
  return 0;
}

//PAGEBREAK!
// Map user virtual address to kernel address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if((*pte & PTE_P) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  return (char*)P2V(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;

  buf = (char*)p;
  while(len > 0){
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char*)va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if(n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

uint
selectPage()
{
  int index = 0;

#if defined(NFUA)
  index = removeNFUA();
#endif
#if defined(LAPA)
  index = removeLAPA();
#endif
#if defined(SCFIFO)
  index = removeSCFIFO();
#endif
#if defined(AQ)
  index = removeAQ();
#endif  

  return myproc()->pages[index].v_address;
}

char *
swapToFile(pde_t *pgdir)
{
  struct proc* curproc = myproc();
  pte_t *pte;
  uint addr = selectPage();
  int offset = getFreeFileOffset();
  writeToSwapFile(curproc, (char *) addr, offset, PGSIZE);
  int i = 0;
  while(curproc->pages[i].v_address != addr && i < MAX_TOTAL_PAGES)
    i++;
  curproc->pages[i].file_offset = offset;  
  curproc->pages[i].is_in_RAM = 0; // the page is on swap file and not on RAM

  curproc->totalNumberOfPagedOut++;
  curproc->numberOfPagedOut++;

  pte = walkpgdir(pgdir, (char *)addr, 0);
  //*pte = PTE_P_OFF(*pte);
  *pte = PTE_PG_ON(*pte);
  uint pageAddr = PTE_ADDR(*pte);
  char* v_addr = P2V(pageAddr);
  kfree(v_addr);
  lcr3(V2P(curproc->pgdir));
  return v_addr;
}

// mappages- for external use
int
global_mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
  return mappages(pgdir,va,size,pa,perm);
}


// walkpgdir- for external use
pte_t *
global_walkpgdir(pde_t *pgdir, void *va, int alloc)
{
  return walkpgdir(pgdir, va, alloc);
}


//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

int
handle_page_fault(uint va){
  pte_t *pte;
  struct proc *proc;
  
  if((proc = myproc()) == 0){
    panic("handle_page_fault");
  }
  proc->numberOfPageFaults++;
  (pte = walkpgdir(proc->pgdir, (void*) va, 0));
  if(va >= KERNBASE){
    cprintf("in kernbase");
    proc->killed = 1;
    return -1;
  }
  if(pte == 0  || !(*pte & PTE_P) || !(*pte & PTE_U) ){
    cprintf("Illegal virtual address. kill proc with pid %d. va = 0x%x. pte = %x. present = %x, user = %x \n", proc->pid, va, pte, (*pte & PTE_P), (*pte & PTE_U));
    proc->killed = 1;
    return -1;
  }
  if(*pte & PTE_W){
    panic("The page is already writeable\n");
  }

    uint pa = PTE_ADDR(*pte); // PTE_ADDR returns the physical address of pte
    uint amount_of_refs_to_page = get_ref_count(pa);
    char *mem;

    if(amount_of_refs_to_page > 1) { // first try to write case
        if((mem = kalloc()) == 0) {
          cprintf("out of memory, kill proc with pid %d\n", proc->pid);
          proc->killed = 1;
          return -1;
        }
        memmove(mem, (char*)P2V(pa), PGSIZE); // copy the page at pa to mem
        //if (*pte & PTE_C){ // cow flag is on --> the file is not read only 
          *pte = V2P(mem) | PTE_P | PTE_U | PTE_W;
          dec_ref_count(pa);
          inc_ref_count((uint) P2V(mem)); 
          lcr3(V2P(myproc()->pgdir));  // NEW
        //}
    }
    else if((amount_of_refs_to_page == 1) && (*pte & PTE_C)){
      *pte |= PTE_W;  // remove the read-only restriction if the page is not read-only (PTE_C is on)
      //*pte &= ~PTE_C;
      // dec_ref_count(pa); // NEW - maybe solve the BUG
      lcr3(V2P(myproc()->pgdir));  // NEW
    }
    else{
      panic("wrong ref count");
    }
    //lapiceoi();
    return 1;
}

int
get_page(uint va){
  struct proc *curproc=myproc();
  uint page_va = PGROUNDDOWN(va);
  int i;
  int counter = 0;

  for(i=0;i<MAX_TOTAL_PAGES;i++){
    if((curproc->pages[i].isAllocated == 1) && (curproc->pages[i].is_in_RAM))
        counter++;
  }
  if(counter == MAX_PSYC_PAGES)
    swapToFile(curproc->pgdir);
  char* newPageAddr = kalloc();
  if(newPageAddr == 0){
    cprintf("kalloc failed (Page Fault case\n");
    return -1;
  }
  memset(newPageAddr, 0, PGSIZE);
  i=0;
  while((i<MAX_TOTAL_PAGES) && (curproc->pages[i].v_address != page_va))
    i++; 
  if(i==MAX_TOTAL_PAGES)
    panic("can't find page");
  uint offset = curproc->pages[i].file_offset;
  readFromSwapFile(curproc, newPageAddr, offset, PGSIZE);
  pte_t *pte = global_walkpgdir(curproc->pgdir,(char *) va,  0);
  *pte = PTE_P_OFF(*pte);
  *pte = PTE_PG_ON(*pte);
  global_mappages(curproc->pgdir, (void *) page_va, PGSIZE, V2P(newPageAddr), PTE_W | PTE_U );
  *pte = PTE_P_ON(*pte);
  *pte = PTE_PG_OFF(*pte);
  curproc->pages[i].is_in_RAM = 1;
  insertOffsetQueue(curproc->pages[i].file_offset);
  curproc->pages[i].file_offset = -1;
  insert(i);
  curproc->numberOfPagedOut--;  
  return 0;
}

int
is_PG_on(uint va){
  struct proc* p = myproc();
  pte_t *pte = walkpgdir(p->pgdir, (char*)rcr2(), 0);
  return *pte & PTE_PG;
}