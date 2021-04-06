//
// Created by pjs on 2021/2/1.
//

#include "mm/mm.h"
#include "mm/free_list.h"

void mm_init() {
    phymm_init();
    vmm_init();
    heap_init();
    test_list_stack2();
}
