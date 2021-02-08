//
// Created by pjs on 2021/2/1.
//
//虚拟内存管理
#include "virtual_mm.h"
#include "types.h"
#include "mm.h"
#include "physical_mm.h"
#include "free_list.h"
#include "qstring.h"
#include "multiboot2.h"

//TODO: 使用buddy/slab

//修改/访问页目录方法: k_pde[i] = xxx;
//修改/访问页表方法:  pde_t *pte = PTE_ADDR(pde_index), pte 为页表索引对应的页目录首地址
static pde_t _Alignas(PAGE_SIZE) k_pde[] = {[0 ...PAGE_SIZE - 1]=(VM_KR | VM_NPRES)};


// 三个参数分别为:
// 需要映射的虚拟地址,物理地址, 需要映射的内存大小
static bool vmm_map(pointer_t va, pointer_t pa, uint32_t size, uint32_t flags) {
    uint32_t pdeI = PDE_INDEX(va), pteI = PTE_INDEX(va);
    bool paging = isPaging();

    for (; size > 0 && pdeI < N_PDE - 1; pdeI++, pteI = 0) {
        pde_t *pde = &k_pde[pdeI];  //页目录项
        pte_t *pte = (pte_t *) PTE_ADDR(pdeI);  //页表首地址(虚拟)

        if (!(*pde & VM_PRES)) {
            *pde = phymm_alloc() | flags;
            //未启用分页时,使用物理地址访问 pte
            if (!paging) pte = (pte_t *) PAGE_ADDR(*pde);
            q_memset(pte, 0, PAGE_SIZE);
        }

        for (; pteI < N_PTE && size > 0; pteI++) {
            pte[pteI] = pa | flags;
            size -= PAGE_SIZE;
            pa += PAGE_SIZE;
        }
    }
    return true;
}


// 初始化内核页表
void vmm_init() {
    cr3_t cr3 = CR3_CTRL | ((pointer_t) k_pde);
    free_list_init(SIZE_ALIGN(g_vmm_start));
    //留出页表的虚拟内存
    assertk(list_split(PTE_VA, PT_SIZE));
    k_pde[N_PDE - 1] = (pointer_t) k_pde | VM_KW | VM_PRES;

    // g_vmm_start 以下部分直接映射
    vmm_map(0, 0, SIZE_ALIGN(g_vmm_start), VM_KW | VM_PRES);
    cr3_set(cr3);
}


// size 为 PAGE_SIZE 的整数倍
void vmm_free(pointer_t va, uint32_t size) {
    assertk((size & ALIGN_MASK) == 0);
    uint32_t pdeI = PDE_INDEX(va), pteI = PTE_INDEX(va);
    for (; size > 0 && pdeI < N_PDE; pdeI++, pteI = 0) {
        uint32_t start = pteI;
        pte_t *pte = (pte_t *) PTE_ADDR(pdeI);
        for (; pteI < N_PTE; pteI++, size -= PAGE_SIZE) {
            pte[pteI] = 0;
            phymm_free(pte[pteI]);
        }
        //释放空页表
        if (start == 0 && pteI == N_PTE - 1) {
            phymm_free(k_pde[pdeI]);
            k_pde[pdeI] = VM_NPRES;
        }
    }
    list_free(va, size);
}


// size 为需要分配的虚拟内存内存大小
void *vmm_alloc(uint32_t size) {
    size = SIZE_ALIGN(size);
    void *addr = list_split_ff(size);
    if (addr == MM_NULL)return MM_NULL;
    pointer_t phy_addr = phymm_alloc();
    if (!vmm_map((pointer_t) addr, phy_addr, size, VM_KW | VM_PRES)) {
        phymm_free(phy_addr);
        list_free((pointer_t) addr, size);
        return MM_NULL;
    };
    return addr;
}