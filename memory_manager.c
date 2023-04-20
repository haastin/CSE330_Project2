#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>

int pid = 0;
module_param(pid, int, 0);

void calc_RSS_SWAP_size(void);

unsigned long timer_interval_ns = 10e9;
static struct hrtimer hr_timer;

int RSS_size;
int SWAP_size;
int WSS_size;
static struct task_struct *curr_process;

int ptep_test_and_clear_young(struct vm_area_struct *vma, unsigned long addy, pte_t *ptep){
	int ret = 0;
	if(pte_young(*ptep))
		ret = test_and_clear_bit(_PAGE_BIT_ACCESSED, (unsigned long *) &ptep->pte);
	return ret;
}


enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart){
	
	ktime_t currtime, interval;
	currtime = ktime_get();
	interval = ktime_set(0, timer_interval_ns);
	hrtimer_forward(timer_for_restart, currtime, interval);
	
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
				if(ptep_test_and_clear_young(it, i, pte)){
						WSS_size++;
						}
                        }
                        else{
                                SWAP_size++;
                        }
                }
                }
                it = it->vm_next;
        }
	printk(KERN_INFO "PID %d: RSS=%d KB, SWAP=%d KB, WSS=%d KB\n", pid,RSS_size, SWAP_size, WSS_size); 

	return HRTIMER_RESTART;
}

int init_func(void){

	curr_process = pid_task(find_vpid(pid), PIDTYPE_PID);
	ktime_t ktime;
	ktime = ktime_set(0, timer_interval_ns);
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hr_timer.function = &timer_callback;
	printk(KERN_INFO "about to launch timer\n");
	hrtimer_start(&hr_timer,ktime,HRTIMER_MODE_REL);
	return 0;
}

void exit_func(void){
	int ret;
	ret = hrtimer_cancel(&hr_timer);
	if(ret)
		printk(KERN_INFO "Timer still in use");
	else
		printk("HR Timer module uninstalling");
}

module_init(init_func);
module_exit(exit_func);
MODULE_LICENSE("GPL");
