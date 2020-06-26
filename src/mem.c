
#include "mem.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include <stdio.h>

static BYTE _ram[RAM_SIZE];

static struct {
	uint32_t proc;	// ID of process currently uses this page
	int index;	// Index of the page in the list of pages allocated
			// to the process.
	int next;	// The next page in the list. -1 if it is the last
			// page.
} _mem_stat [NUM_PAGES];

static pthread_mutex_t mem_lock;

void init_mem(void) {
	memset(_mem_stat, 0, sizeof(*_mem_stat) * NUM_PAGES);
	memset(_ram, 0, sizeof(BYTE) * RAM_SIZE);
	pthread_mutex_init(&mem_lock, NULL);
}

/* get offset of the virtual address */
static addr_t get_offset(addr_t addr) {
	return addr & ~((~0U) << OFFSET_LEN);
}

/* get the first layer index */
static addr_t get_first_lv(addr_t addr) {
	return addr >> (OFFSET_LEN + SEGMENT_LEN);
}

/* get the second layer index */
static addr_t get_second_lv(addr_t addr) {
	return (addr >> OFFSET_LEN) - (get_first_lv(addr) << PAGE_LEN);
}

/* Search for page table table from the a segment table */
static struct page_table_t * get_page_table(
		addr_t index, 	// Segment level index
		struct seg_table_t * seg_table) { // first level table
	
	/*
	 * TODO: Given the Segment index [index], you must go through each
	 * row of the segment table [seg_table] and check if the v_index
	 * field of the row is equal to the index
	 *
	 * */

	int i;
	for (i = 0; i < seg_table->size; i++) {
		// Enter your code here
        if(seg_table->table[i].v_index == index)
            return seg_table->table[i].pages;
	}
	return NULL;

}

/* Translate virtual address to physical address. If [virtual_addr] is valid,
 * return 1 and write its physical counterpart to [physical_addr].
 * Otherwise, return 0 */
static int translate(
		addr_t virtual_addr, 	// Given virtual address
		addr_t * physical_addr, // Physical address to be returned
		struct pcb_t * proc) {  // Process uses given virtual address

	/* Offset of the virtual address */
	addr_t offset = get_offset(virtual_addr);
	/* The first layer index */
	addr_t first_lv = get_first_lv(virtual_addr);
	/* The second layer index */
	addr_t second_lv = get_second_lv(virtual_addr);
	
	/* Search in the first level */
	struct page_table_t * page_table = NULL;
	page_table = get_page_table(first_lv, proc->seg_table);
	if (page_table == NULL) {
		return 0;
	}

	int i;
	for (i = 0; i < page_table->size; i++) {
		if (page_table->table[i].v_index == second_lv) {
			/* TODO: Concatenate the offset of the virtual addess
			 * to [p_index] field of page_table->table[i] to 
			 * produce the correct physical address and save it to
			 * [*physical_addr]  */ 
            *physical_addr = (page_table->table[i].p_index << OFFSET_LEN) | offset;
			return 1;
		}
	}
	return 0;	
}

static void function1(addr_t virtual_address,addr_t p_index,struct pcb_t* proc)
{
	addr_t a1 = get_first_lv(virtual_address);
	addr_t a2 = get_second_lv(virtual_address);
	// Tim bang phan trang ung voi a1
	struct page_table_t * p_t = NULL;
	p_t = get_page_table(a1,proc->seg_table);
	// Neu chua co Page do
	if(p_t == NULL)
	{
		int size = proc->seg_table->size;
		proc->seg_table->table[size].v_index=a1;

		// Cap phat bang phan trang ung voi a1
		proc->seg_table->table[size].pages = (struct page_table_t*)malloc(sizeof(struct page_table_t));
		p_t=proc->seg_table->table[size].pages;
		p_t->size=0;

		// Tang so luong Page phan trang trong segment_table 
		proc->seg_table->size += 1;
	}
	int size1=p_t->size;
	p_t->table[size1].v_index =	a2;
	p_t->table[size1].p_index = p_index;
	p_t->size += 1;
}

addr_t alloc_mem(uint32_t size, struct pcb_t * proc) {
	pthread_mutex_lock(&mem_lock);
	addr_t ret_mem = 0;
	/* TODO: Allocate [size] byte in the memory for the
	 * process [proc] and save the address of the first
	 * byte in the allocated memory region to [ret_mem].
	 * */

	uint32_t num_pages = (size % PAGE_SIZE) ? size / PAGE_SIZE  + 1:
		size / PAGE_SIZE ; // Number of pages we will use
	int mem_avail = 0; // We could allocate new memory region or not?

	/* First we must check if the amount of free memory in
	 * virtual address space and physical address space is
	 * large enough to represent the amount of required 
	 * memory. If so, set 1 to [mem_avail].
	 * Hint: check [proc] bit in each page of _mem_stat
	 * to know whether this page has been used by a process.
	 * For virtual memory space, check bp (break pointer).
	 * */
    int num = 0;
	for (int i = 0; i < NUM_PAGES && num != num_pages; i++){
		if(_mem_stat[i].proc==0) {
			num++;
		}
    }
	if(num == num_pages && proc->bp+num_pages*PAGE_SIZE < (1 << ADDRESS_SIZE)){
		mem_avail = 1;
    }
	if (mem_avail) {
		/* We could allocate new memory region to the process */
		ret_mem = proc->bp;
		proc->bp += num_pages * PAGE_SIZE;
            
        
		/* Update status of physical pages which will be allocated
		 * to [proc] in _mem_stat. Tasks to do:
		 * 	- Update [proc], [index], and [next] field
		 * 	- Add entries to segment table page tables of [proc]
		 * 	  to ensure accesses to allocated memory slot is
		 * 	  valid. */
        int nt = 0;
        for(int p_i = 0; p_i < NUM_PAGES && num > 0; p_i++){
            if(_mem_stat[p_i].proc == 0){
                _mem_stat[p_i].proc = proc->pid;
                _mem_stat[p_i].index = num_pages - num;
                addr_t v_addr = ret_mem + _mem_stat[p_i].index * PAGE_SIZE;
		//printf("%d\t%d\t%d\t%d\t%d\n",_mem_stat[p_i].proc,_mem_stat[p_i].index,get_first_lv(v_addr),get_second_lv(v_addr),get_offset(v_addr));
                function1(v_addr,p_i,proc);

                if( num < num_pages){
                    _mem_stat[nt].next = p_i;
                }
                nt = p_i;
                num--;            
            }
        }
        _mem_stat[nt].next = -1;

	}
	pthread_mutex_unlock(&mem_lock);
	return ret_mem;
}

int free_mem(addr_t address, struct pcb_t * proc) {
	/*TODO: Release memory region allocated by [proc]. The first byte of
	 * this region is indicated by [address]. Task to do:
	 * 	- Set flag [proc] of physical page use by the memory block
	 * 	  back to zero to indicate that it is free.
	 * 	- Remove unused entries in segment table and page tables of
	 * 	  the process [proc].
	 * 	- Remember to use lock to protect the memory from other
	 * 	  processes.  */
    pthread_mutex_lock(&mem_lock);

    int addr = -1;

    addr_t first_lv = get_first_lv(address);
	addr_t second_lv = get_second_lv(address);
	struct page_table_t * page_table = NULL;

	page_table = get_page_table(first_lv, proc->seg_table);

	if (page_table == NULL){
        pthread_mutex_unlock(&mem_lock);
        return 0;
    }
    int i;
    for (i = 0; i < page_table->size; i++) {
		if (page_table->table[i].v_index == second_lv) {
            //page_table->table[i].v_index = -1;
            addr = page_table->table[i].p_index;            
			break;
		}
	}
    
    if(addr == -1){
        pthread_mutex_unlock(&mem_lock);
        return 0;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    int num=0; // So page cua ma vung nho tro den

    while(_mem_stat[addr].next != -1){
        _mem_stat[addr].proc = 0;
        addr = _mem_stat[addr].next;
        num++;
    }
    num++;
	_mem_stat[addr].proc = 0;  
    
	// cap nhap page table - segtable
	int seg_idex = 0;	// Vi tri cua doan co v_addr == first_lv
	int pag_idex = 0;	// Vi tri cua doan co v_addr == second_lv



	// Tim vi tri cua doan
	while (proc->seg_table->table[seg_idex].v_index != first_lv && seg_idex < proc->seg_table->size){
		seg_idex += 1;
	}


	// Tim vi tri cua trang
	while (proc->seg_table->table[seg_idex].pages->table[pag_idex].v_index != second_lv && pag_idex < proc->seg_table->table[seg_idex].pages->size) {
		pag_idex += 1;
	}

	// Free  all page table
	if(pag_idex + num == proc->seg_table->table[seg_idex].pages->size && pag_idex == 0)
	{
		for(int i=0;i<num;i++){
				proc->seg_table->table[seg_idex].pages->table[i].v_index = -1;
				proc->seg_table->table[seg_idex].pages->table[i].p_index = 0;
				proc->seg_table->table[seg_idex].v_index = -1;
		}
		pthread_mutex_unlock(&mem_lock);
		return 0;
	} 
	// Kiem tra can phai free doan tiep theo khong
	if (pag_idex + num < 32)
	{
		// KHong can free doan tiep theo
		for (int i = 0; i < num; i++)
		{
			proc->seg_table->table[seg_idex].pages->table[pag_idex + i].v_index = -1;
			proc->seg_table->table[seg_idex].pages->table[pag_idex + i].p_index = 0;
		}	
	}
	else
	{
		// Free doan tiep theo

		int page1 = 32 - pag_idex;	// Number of pages on first segment
		int page2 = num - page1;	// Number of pages on next segment

		// Free doan 1
		for (int i = 0; i < page1; i++)
		{
			proc->seg_table->table[seg_idex].pages->table[pag_idex + i].v_index = -1;
			proc->seg_table->table[seg_idex].pages->table[pag_idex + i].p_index = 0;
		}
		
		int count=1;
		//Free cac doan con lai
		while(page2>32){
			for(int i=0;i<32;i++){
				proc->seg_table->table[seg_idex + count].pages->table[i].v_index = -1;
				proc->seg_table->table[seg_idex + count].pages->table[i].p_index = 0;
				proc->seg_table->table[seg_idex + count].v_index = -1;
			}
			count++;
			page2 = page2 - 32;
		}
		for(int i=0;i<page2;i++){
				proc->seg_table->table[seg_idex + count].pages->table[i].v_index = -1;
				proc->seg_table->table[seg_idex + count].pages->table[i].p_index = 0;
		}
		
	}


    
    pthread_mutex_unlock(&mem_lock);
	return 0;
}

int read_mem(addr_t address, struct pcb_t * proc, BYTE * data) {
	pthread_mutex_lock(&mem_lock);
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		*data = _ram[physical_addr];
		pthread_mutex_unlock(&mem_lock);
		return 0;
	}else{
		pthread_mutex_unlock(&mem_lock);
		return 1;
	}
}

int write_mem(addr_t address, struct pcb_t * proc, BYTE data) {
	pthread_mutex_lock(&mem_lock);
	addr_t physical_addr;
	if (translate(address, &physical_addr, proc)) {
		_ram[physical_addr] = data;
		pthread_mutex_unlock(&mem_lock);
		return 0;
	}else{
		pthread_mutex_unlock(&mem_lock);
		return 1;
	}
}

void dump(void) {
	pthread_mutex_lock(&mem_lock);
	int i;
	for (i = 0; i < NUM_PAGES; i++) {
		if (_mem_stat[i].proc != 0) {
			printf("%03d: ", i);
			printf("%05x-%05x - PID: %02d (idx %03d, nxt: %03d)\n",
				i << OFFSET_LEN,
				((i + 1) << OFFSET_LEN) - 1,
				_mem_stat[i].proc,
				_mem_stat[i].index,
				_mem_stat[i].next
			);
			int j;
			for (	j = i << OFFSET_LEN;
				j < ((i+1) << OFFSET_LEN) - 1;
				j++) {
				
				if (_ram[j] != 0) {
					printf("\t%05x: %02x\n", j, _ram[j]);
				}
					
			}
		}
	}
	pthread_mutex_unlock(&mem_lock);
}


