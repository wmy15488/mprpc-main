#pragma once
#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    // 线程局部缓存的线程 id，每个线程一份，初始为 0 表示未缓存。
    extern __thread int t_cachedTid;

    // 从内核获取当前线程 id 并写入线程局部缓存。
    void cacheTid();

    // 获取当前线程 id：优先走缓存，未命中时再触发一次系统调用。
    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }

        return t_cachedTid;
    }
}
