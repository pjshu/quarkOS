//
// Created by pjs on 2021/4/8.
//

#ifndef QUARKOS_BUF_H
#define QUARKOS_BUF_H

#include "types.h"
#include "sched/sleeplock.h"
#include "lib/list.h"
#include "mm/mm.h"
#include "lib/rwlock.h"

/*
 * BUF_DIRTY: 数据需要更新
 * BUF_VALID: 缓冲块数据为有效数据
 * timestamp: 上次访问该页的时间
 * no_secs:   需要读写的扇区起始 lba 值
 * list:      等待读写队列
 * ref_cnt:   引用次数
 */
typedef struct page_cache {
#define BUF_DIRTY   1
#define BUF_VALID   (1<<1)
#define SECTOR_SIZE 512
#define BUF_SIZE    PAGE_SIZE
    u32_t timestamp;

    void *data;

    uint16_t flag;
//    uint16_t ref_cnt;
    uint32_t no_secs;

    list_head_t list;
    rwlock_t rwlock;
} buf_t;

#define buf_entry(ptr) list_entry(ptr,buf_t,list)

#endif //QUARKOS_BUF_H
