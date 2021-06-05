//
// Created by pjs on 2021/5/30.
//

#ifndef QUARKOS_MM_VMALLOC_H
#define QUARKOS_MM_VMALLOC_H

#include <types.h>
#include <lib/list.h>
#include <mm/kvm.h>


typedef struct mm_struct {
    struct vm_area {
        list_head_t head;
        ptr_t addr;
        u32_t size;
        u32_t flag;
    } text, rodata, dataBss, brk, stack;
    list_head_t area;
    ptr_t size;           // 已经使用的虚拟内存大小
    pte_t *pgdir;         // 页目录物理地址
} mm_struct_t;


void vm_area_init();

void vm_map(struct vm_area *area, pde_t *pgdir, u32_t flag);

void vm_struct_destroy(struct mm_struct *mm);

ptr_t vm_vm2pm(void *addr, pte_t *pgdir);

void vm_unmap(struct vm_area *area, pte_t *pgdir);

struct mm_struct *vm_struct_copy(struct mm_struct *src);

#endif //QUARKOS_MM_VMALLOC_H
