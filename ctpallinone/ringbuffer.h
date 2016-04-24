#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <QObject>
#include <atomic>

//每个合约在内存中使用ringbuffer保留(256)个tick，ui上要延迟1分钟使用请自己复制=
class RingBuffer {
public:
    void init(int item_len, int item_count)
    {
        int buflen = item_len * item_count;
        if (buflen > 4 * 1024 * 1024) {
            qFatal("ringbuffer > 4M");
            return;
        }
        buffer_ = new char[item_len * item_count];
        items_ = new void*[item_count];
        for (int i = 0; i < item_count; i++) {
            items_[i] = 0;
        }
        head_ = -1;
        item_count_ = item_count;
        item_len_ = item_len;
    }

    void free()
    {
        delete[] buffer_;
        delete[] items_;
    }

    void* put(void* item)
    {
        int index = head_ + 1;
        index = index % item_count_;

        char* buf = buffer_ + index * item_len_;
        items_[index] = buf;
        memcpy(buf, item, item_len_);
        //原子操作=
        head_ = index;

        return buf;
    }

    void* put(void* item, int& index)
    {
        index = head_ + 1;
        index = index % item_count_;

        char* buf = buffer_ + index * item_len_;
        items_[index] = buf;
        memcpy(buf, item, item_len_);
        //原子操作=
        head_ = index;

        return buf;
    }

    int count() { return item_count_; }

    //需要判断返回值哦=
    void* get(int index)
    {
        if (index < 0)
            return nullptr;

        return items_[index % item_count_];
    }

    int head()
    {
        //原子操作=
        return head_;
    }

private:
    int item_count_;
    int item_len_;
    void** items_;
    char* buffer_;
    std::atomic_int32_t head_; //最近有效数据的下标=
};

#endif // RINGBUFFER_H
