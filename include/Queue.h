/**
* @file Queue.h
* @brief ���޹صĻ��ζ���
* @remark һ���̵߳���EnQueue��һ���̵߳���Dequeue�����ᷢ����ͻ
*/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdio.h>
#include <assert.h>

namespace Algorithm
{

    /// ���޹ػ��ζ���
    template<typename DataType>
    class Queue
    {
    public:
        /// ����
        /// @param QueueSize ��ʼ���Ķ��д�С
        Queue(unsigned int QueueSize);

        /// ����
        ~Queue();

        /// ���
        /// @param In ��Ҫ��ӵ�Ԫ�أ��ᷢ������
        /// @return ����Ƿ�ɹ�
        bool EnQueue(const DataType& In);

        /// ����
        /// @param Out �������
        /// @return �����Ƿ�ɹ�
        bool DeQueue(DataType& Out);

        /// ���Ԫ�صĸ���
        unsigned int Count() const;

        /// ��ö��еĴ�С
        unsigned int QueueSize() const;

        /// �Ƿ�ӿ�
        bool IsEmpty() const;

        /// �Ƿ����
        bool IsFull() const;

        /// ��ȡ��ͷԪ�أ����ǲ�����
        bool GetFront(DataType& Out);

        /// ��ն���
        void Clear();

        /// �������ö��еĴ�С
        /// @param NewSize �µĶ���Ԫ�ظ���
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
        volatile unsigned int m_Tail;  //ʹ��volatile�ؼ��֣��������������CPU������
    };

#include "../src/Queue.hpp"

};  //end namespace Algorithm

#endif
