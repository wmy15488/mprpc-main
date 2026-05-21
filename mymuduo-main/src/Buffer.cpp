#include "Buffer.h"
#include <sys/uio.h>

// 默认构造一个带预留区的缓冲区。
Buffer::Buffer(size_t initialSize)
    : buffer_(kCheapPrepend + initialSize),
      readerIndex_(kCheapPrepend),
      writerIndex_(kCheapPrepend)
{
}

// 返回可读数据的起始地址。
const char* Buffer::peek() const
{
    return readerIndex_+begin();
}

// 返回可读数据的长度。
size_t Buffer::readableBytes() const
{
    return writerIndex_-readerIndex_;
}

// 返回可写空间的长度。
size_t Buffer::writableBytes() const
{
    return buffer_.size()-writerIndex_;
}

// 返回头部预留区的长度。
size_t Buffer::prependableBytes() const
{
    return readerIndex_;
}

// 按长度回收已读数据。
void Buffer::retrieve(size_t len)
{
    if(len>=writerIndex_-readerIndex_)
    {
        retrieveAll();
    }
    else
    {
        readerIndex_+=len;
    }
}

// 回收全部已读数据。
void Buffer::retrieveAll()
{
    readerIndex_=writerIndex_=kCheapPrepend;
}

// 以字符串形式取出全部可读数据。
std::string Buffer::retrieveAllAsString()
{
    std::string str(peek(),readableBytes());
    retrieveAll();
    return str;
}

// 追加字符串数据。
void Buffer::append(const std::string& str)
{
    append(str.data(),str.size());
}

// 追加指定长度的数据。
void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data,data+len,beginWrite());
    hasWritten(len);
}

// 从文件描述符读取数据到缓冲区。
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrbuf[65536]={0};
    const size_t writable = writableBytes();
    struct iovec vec[2];
    vec[0].iov_base=beginWrite();
    vec[0].iov_len=writable;
    vec[1].iov_base=extrbuf;
    vec[1].iov_len=sizeof(extrbuf);
    const int iovcnt=(writable<sizeof(extrbuf))?2:1;
    ssize_t n=::readv(fd,vec,iovcnt);
    if(n<0)
    {
        *saveErrno=errno;
    }
    else if(static_cast<size_t>(n)<=writable)
    {
        writerIndex_+=static_cast<size_t>(n);
    }
    else
    {
        writerIndex_=buffer_.size();
        append(extrbuf,static_cast<size_t>(n)-writable);
    }
    return n;
}

// 确保缓冲区存在足够可写空间。
void Buffer::ensureWritableBytes(size_t len)
{
    if(writableBytes()<len)
    {
        makeSpace(len);
    }
    assert(writableBytes()>=len);
}

// 返回可写区域的起始地址。
char* Buffer::beginWrite()
{
    return begin()+writerIndex_;
}

// 返回可写区域的起始地址。
const char* Buffer::beginWrite() const
{
    return begin()+writerIndex_;
}

// 标记已经写入的数据长度。
void Buffer::hasWritten(size_t len)
{
    assert(len<=writableBytes());
    writerIndex_+=len;
}

// 返回底层容器的起始地址。
char* Buffer::begin()
{
    return &*buffer_.begin();
}

// 返回底层容器的起始地址。
const char* Buffer::begin() const
{
    return &*buffer_.begin();
}

// 扩容或整理缓冲区空间。
void Buffer::makeSpace(size_t len)
{
   if(prependableBytes()+writableBytes()<len+kCheapPrepend)
   {
    buffer_.resize(writerIndex_+len);
   }
   else
   {
    const size_t readable=readableBytes();
    std::copy(peek(),peek()+readable,begin()+kCheapPrepend);
    readerIndex_=kCheapPrepend;
    writerIndex_=kCheapPrepend+readable;
   }
}
