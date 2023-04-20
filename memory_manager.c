#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>

int pid = 0;
module_param(pid, int, 0);

void calc_RSS_SWAP_size(void);
void calc_WSS_size(void);

int RSS_size;
int SWAP_size;
int WSS_size;
static struct task_struct *curr_process;
void calc_RSS_SWAP_size(){
	
	struct vm_area_struct *it = curr_process->mm->mmap;
	while(it != NULL){
		
		for(unsigned long i = it->vm_start; i < it->vm_end; i += PAGE_SIZE){
		pgd_t *pgd = pgd_offset(curr_process->mm, i);
		p4d_t *p4d = p4d_offset(pgd, i);
		pud_t *pud = pud_offset(p4d, i);
		pmd_t *pmd = pmd_offset(pud, i);
		pte_t *pte = pte_offset_map(pmd, i);
		if(pte){
			if(pte_present(*pte)){
				RSS_size++;
			}
			else{
				SWAP_size++;
			}
		}	
		}
		it = it->vm_next;
	}
	
}

void calc_WSS_size(){


}

int init_func(void){

	curr_process = pid_task(find_vpid(pid), PIDTYPE_PID);

	return 0;
}

void exit_func(void){
}

module_init(init_func);
module_exit(exit_func);
MODULE_LICENSE("GPL");
