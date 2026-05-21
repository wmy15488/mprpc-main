#include "CurrentThread.h"

namespace CurrentThread
{
// 线程局部变量定义：每个线程独立保存自己的缓存 tid。
__thread int t_cachedTid = 0;

void cacheTid()
{
    // Linux 下通过 syscall(SYS_gettid) 获取内核线程 id。
    t_cachedTid = static_cast<int>(::syscall(SYS_gettid));
}
}  // namespace CurrentThread
