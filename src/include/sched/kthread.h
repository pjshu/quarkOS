//
// Created by pjs on 2021/2/18.
//
// 内核线程

#ifndef QUARKOS_SCHED_KTHREAD_H
#define QUARKOS_SCHED_KTHREAD_H

#include "types.h"
#include "lib/list.h"
#include "x86.h"
#include "mm/mm.h"
#include "sched/klock.h"

/*
 * 线程切换时需要保存的上下文
 * 根据x86 systemV ABI eax, ecx, edx 是临时寄存器,使用(schedule)函数手动切换时无需保存,
 * 使用中断切换时, __attribute__((interrupt)) 会保存 eax ecx, edx
 * 段选择子固定, 不使用 ldt, 都无需保存
 * 内核线程不需要保存 cr3, call 指令保存 eip
 */
typedef struct context {
    uint32_t esp;
    uint32_t eflags;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} context_t;


typedef uint32_t kthread_state_t;
#define TASK_RUNNING  0    //线程可运行或正在运行
#define TASK_SLEEPING 1
#define TASK_ZOMBIE   2     //线程终止等待回收

typedef uint32_t kthread_t; //线程id

typedef struct tcb {
#define KTHREAD_NAME_LEN   16
#define KTHREAD_STACK_SIZE PAGE_SIZE
#define KTHREAD_NUM        65536
    list_head_t run_list;    //运行队列
    kthread_t tid;
    kthread_state_t state;
    char name[KTHREAD_NAME_LEN];
    void *stack;             //指向栈首地址,用于回收
    context_t context;       //上下文信息
} tcb_t;

int kthread_create(kthread_t *tid, void *(worker)(void *), void *args);

int kt_create(list_head_t **_thread, kthread_t *tid, void *(worker)(void *), void *args);

void schedule();

void kthread_exit();

int8_t block_thread(list_head_t *_block_list, spinlock_t *lock);

void unblock_thread(list_head_t *head);

void unblock_threads(list_head_t *head);

// 通用阻塞列表
extern list_head_t block_list;

// 可运行进程队列
extern list_head_t *init_task;

// i r esp / ~(uint32_t)(4096 - 1)
INLINE tcb_t *cur_tcb() {
    tcb_t *tcb;
    asm("andl %%esp,%0; ":"=r" (tcb): "0" (~PAGE_MASK));
    return tcb;
}

#define CUR_TCB  cur_tcb()
#define CUR_HEAD (CUR_TCB->run_list)
#define tcb_entry(ptr) list_entry(ptr,tcb_t,run_list)

#endif //QUARKOS_SCHED_KTHREAD_H
