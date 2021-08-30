// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  uint amount_of_free_pages;
  uint page_refs_num [PHYSTOP >> PGSHIFT]; // in size (last physical address / page size) = max amount of pages
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  kmem.amount_of_free_pages = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE){
    kfree(p); // no need of kmem.amount-- because of this call
    kmem.page_refs_num[V2P(p) >> PGSHIFT] = 0;
  }
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  if(kmem.use_lock)
      acquire(&kmem.lock);
 
  // release page only if there is no reference to it
  if (kmem.page_refs_num[V2P(v) >> PGSHIFT] == 0){
    // Fill with junk to catch dangling refs.
    //memset(v, 1, PGSIZE);

    kmem.amount_of_free_pages++;
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    
    
  }else  if (kmem.page_refs_num[V2P(v) >> PGSHIFT] > 0){
    --kmem.page_refs_num[V2P(v) >> PGSHIFT];
  }
  if(kmem.use_lock)
      release(&kmem.lock);
  // if there is some references to this page - decrease the ref's counter of this page
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.amount_of_free_pages--;
    kmem.page_refs_num[V2P(r) >> PGSHIFT] = 1; // the first allocation
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

// int
// getCurrentCapacity()
// {
//   struct run *temp;
//   int counter = 0;
//   if(kmem.use_lock)
//     acquire(&kmem.lock);
//   temp = kmem.freelist;
//   while(temp){
//     temp = temp->next;
//     counter++;
//   }
//   if(kmem.use_lock)
//     release(&kmem.lock);
//   return counter;
// }

void dec_ref_count(uint pa)
{
  if( pa < V2P (end) || pa >= PHYSTOP) // end = first address of physical addresses
    panic("dec_ref_count");
  acquire(&kmem.lock);

  //   if (kmem.page_refs_num[pa >> PGSHIFT] == 0)
  // {
  //   kmem.amount_of_free_pages++;
  // }
  // if (kmem.page_refs_num[pa >> PGSHIFT] > 0){ // NEW
    --kmem.page_refs_num[pa >> PGSHIFT]; // ref num. (physical addr / page size) counter --
  // }

  
  release(&kmem.lock);
}

void inc_ref_count(uint pa)
{
  if( pa < V2P (end) || pa >= PHYSTOP) // end = first virtual address of physical addresses
    panic("inc_ref_count");
  acquire(&kmem.lock);
  // if (kmem.page_refs_num[pa >> PGSHIFT] == 0) // NEW
  //   kmem.amount_of_free_pages --;
  kmem.page_refs_num[pa >> PGSHIFT]++;
  release(&kmem.lock);
}

uint get_ref_count(uint pa)
{
  if( pa < V2P (end) || pa >= PHYSTOP) // end = first address of physical addresses
    panic("get_ref_count");
  uint count;
  acquire(&kmem.lock);
  count = kmem.page_refs_num[pa >> PGSHIFT];
  release(&kmem.lock);
  return count;
}

int numberOfFreePages(void)
{
  acquire(&kmem.lock);
  int num_of_free_pages = kmem.amount_of_free_pages;
  release(&kmem.lock);
  return num_of_free_pages;
}


