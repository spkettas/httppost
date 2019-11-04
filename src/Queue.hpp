/**
* @file Queue.hpp
* @brief 锁无关的环形队列
* @remark 一个线程调用EnQueue，一个线程调用Dequeue，不会发生冲突
*/

/// @brief 构造函数
/// @param QueueSize 队列中的元素个数
template<typename DataType>
Queue<DataType>::Queue(unsigned int QueueSize)
        :
        QUEUE_SIZE(QueueSize), m_Head(0UL), m_Tail(0UL)
{
    assert(QueueSize > 0);
    m_Data = new DataType[QueueSize];
    if (NULL == m_Data)  //对于内存不足的问题不予考虑
    {
        assert(false);
        return;
    }
}

/// @brief 析构函数
template<typename DataType>
Queue<DataType>::~Queue()
{
    if (NULL != m_Data)
    {
        delete[] m_Data;
        m_Data = NULL;
    }
}

/// @brief 入队
/// @param In 入队的元素
/// @return true/false
template<typename DataType>
bool Queue<DataType>::EnQueue(const DataType& In)
{
    //int nNext = (m_Tail==QUEUE_SIZE-1)?0 : m_Tail+1;
    //以上语句简化为：
    unsigned int nNext = (m_Tail + 1) % QUEUE_SIZE;
    if (nNext == m_Head)  //队满
    {
        return false;
    }
    m_Data[m_Tail] = In;
    m_Tail = nNext;
    return true;
}

/// @brief 出队
/// @param Out 输出参数
/// @return true/false
template<typename DataType>
bool Queue<DataType>::DeQueue(DataType& Out)
{
    if (m_Head == m_Tail)  //队空  多线程中的极端情况：队列中只有一个元素，但是出队错误
    {
        return false;
    }

    Out = m_Data[m_Head];
    //m_Head = (m_Head==QUEUE_SIZE-1) ? 0 : m_Head+1;
    //以上语句简化为：
    m_Head = (m_Head + 1) % QUEUE_SIZE;
    return true;
}

/// @brief 获得队列中元素的个数
/// @return 元素个数
template<typename DataType>
unsigned int Queue<DataType>::Count() const
{
    if (m_Head == m_Tail)
    {
        return 0;
    }
    else if (m_Head > m_Tail)
    {
        return QUEUE_SIZE - (m_Head - m_Tail);
    }
    else
    {
        return m_Tail - m_Head;
    }
}

/// 返回队列的大小
template<typename DataType>
unsigned int Queue<DataType>::QueueSize() const
{
    return QUEUE_SIZE;
}

/// @brief 判断是否队空
/// @return true/false
template<typename DataType>
bool Queue<DataType>::IsEmpty() const
{
    return m_Head == m_Tail;
}

/// @brief 判断是否队满
/// @return true/false
template<typename DataType>
bool Queue<DataType>::IsFull() const
{
    return m_Head == ((m_Tail + 1) % QUEUE_SIZE);
}

/// @brief 得到队头的元素
/// @param Out 输出指针
/// @return true/false
template<typename DataType>
bool Queue<DataType>::GetFront(DataType& Out)
{
    if (m_Head == m_Tail)  //队空
    {
        return false;
    }
    //int nLoc = (m_Head==QUEUE_SIZE-1) ? 0 : m_Head+1;
    //*Out = m_Data[nLoc];
    //以上BUG修正为：
    Out = m_Data[m_Head];
    return true;
}

/// 重新设置队列的大小
template<typename DataType>
bool Queue<DataType>::Resize(unsigned int NewSize)
{
    unsigned int nCount = Count();
    if (NewSize <= nCount)
    {
        return false;
    }
    DataType* pData = new DataType[NewSize];
    if (nCount > 0)
    {
        //对旧数据进行拷贝
        if (m_Tail < m_Head)  //数据在两头
        {
            unsigned int nTempCount = QUEUE_SIZE - m_Head;
            memcpy(pData, m_Data + m_Head, sizeof(DataType) * nTempCount);
            memcpy(pData + nTempCount, m_Data, m_Tail);
        }
        else
        {
            memcpy(pData, m_Data + m_Head, sizeof(DataType) * nCount);  //数据在中间
        }
    }
    m_Head = 0;
    m_Tail = nCount;
    QUEUE_SIZE = NewSize;
    delete[] m_Data;
    m_Data = pData;
    return true;
}

/// @brief 清空队列
template<typename DataType>
void Queue<DataType>::Clear()
{
    m_Head = m_Tail;
}

/**
* @example TestQueue.cpp 队列的使用例子
*/
