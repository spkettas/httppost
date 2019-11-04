/**
* @file Queue.h
* @brief 锁无关的环形队列
* @remark 一个线程调用EnQueue，一个线程调用Dequeue，不会发生冲突
*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h>
#include <assert.h>

namespace Algorithm
{

    /// 锁无关环形队列
    template<typename DataType>
    class Queue
    {
    public:
        /// 构造
        /// @param QueueSize 初始化的队列大小
        Queue(unsigned int QueueSize);

        /// 析构
        ~Queue();

        /// 入队
        /// @param In 需要入队的元素，会发生拷贝
        /// @return 入队是否成功
        bool EnQueue(const DataType& In);

        /// 出队
        /// @param Out 输出参数
        /// @return 出队是否成功
        bool DeQueue(DataType& Out);

        /// 获得元素的个数
        unsigned int Count() const;

        /// 获得队列的大小
        unsigned int QueueSize() const;

        /// 是否队空
        bool IsEmpty() const;

        /// 是否队满
        bool IsFull() const;

        /// 获取队头元素，但是不出队
        bool GetFront(DataType& Out);

        /// 清空队列
        void Clear();

        /// 重新设置队列的大小
        /// @param NewSize 新的队列元素个数
        bool Resize(unsigned int NewSize);

    private:
        Queue();

        Queue(const Queue& rsh);

        Queue& operator=(const Queue& rsh);
        //-----------------------------------------------------------------------------
    protected:

    private:
        unsigned int QUEUE_SIZE;
        DataType* m_Data;
        volatile unsigned int m_Head;
        volatile unsigned int m_Tail;  //使用volatile关键字，避免变量被放在CPU缓存中
    };

#include "../src/Queue.hpp"

};  //end namespace Algorithm

#endif
