/**
* @file FixedContainer.hpp
* @brief 基于大数组的节点分配类，相当于内存池
*/

template<typename DataType>
FixedContainer<DataType>::FixedContainer(unsigned int MaxNodes)
        :
        MAX_NODES(MaxNodes), m_Data(NULL), m_FreeNodes(MaxNodes + 1)
//m_UsedNodes(MaxNodes)
{
    assert(MaxNodes > 0);
    m_Data = new DataType[MaxNodes];
    if (NULL == m_Data)  //对于内存不足的问题不予考虑
    {
        assert(false);
        return;
    }
    //把空节点的下标放入队列
    unsigned int i;
    for (i = 0; i < MaxNodes; i++)
    {
        m_FreeNodes.EnQueue(i);
    }
    assert(m_FreeNodes.IsFull());
}

template<typename DataType>
FixedContainer<DataType>::~FixedContainer()
{
    if (NULL != m_Data)
    {
        delete[] m_Data;
        m_Data = NULL;
    }
}

template<typename DataType>
bool FixedContainer<DataType>::IsEmpty() const
{
    return m_FreeNodes.IsFull();
}

template<typename DataType>
bool FixedContainer<DataType>::IsFull() const
{
    return m_FreeNodes.IsEmpty();
}

template<typename DataType>
unsigned int FixedContainer<DataType>::FreeNodesCount() const
{
    return m_FreeNodes.Count();
}

template<typename DataType>
unsigned int FixedContainer<DataType>::UsedNodesCount() const
{
    return MAX_NODES - m_FreeNodes.Count();
}

template<typename DataType>
unsigned int FixedContainer<DataType>::NodesCount() const
{
    return MAX_NODES;
}

template<typename DataType>
DataType* FixedContainer<DataType>::GetNode()
{
    unsigned int nIndex;
    if (!m_FreeNodes.DeQueue(nIndex))
    {
        return NULL;
    }
    //m_UsedNodes.EnQueue(nIndex);
    return m_Data + nIndex;
}

template<typename DataType>
bool FixedContainer<DataType>::FreeNode(DataType& Node)
{
    DataType* p = &Node;
    if (p < m_Data || p > m_Data + (MAX_NODES - 1))  //下标越界
    {
        return false;
    }
    unsigned int nIndex = p - m_Data;
    assert(nIndex < MAX_NODES);
    if (nIndex >= MAX_NODES)
    {
        return false;
    }
    if (!m_FreeNodes.EnQueue(nIndex))  //队满，说明把两个节点重复入队了
    {
        return false;
    }
    return true;
}

template<typename DataType>
void FixedContainer<DataType>::SetZero()
{
    memset(m_Data, 0, sizeof(DataType) * MAX_NODES);
}

#ifndef NDEBUG

template<typename DataType>
void FixedContainer<DataType>::Print()
{
    fprintf(stderr, "FixedContainer: start address=%08x\n", (unsigned int) m_Data);
    fprintf(stderr, "FixedContainer: end address=%08x\n", (unsigned int) m_Data + sizeof(DataType) * MAX_NODES - 1);
    fprintf(stderr, "FixedContainer: maxnode=%u\n", MAX_NODES);
    fprintf(stderr, "FixedContainer: use node=%u\n", UsedNodesCount());
}

#endif

template<typename DataType>
DataType& FixedContainer<DataType>::operator[](int Index)
{
    assert(Index >= 0 && Index < (int) MAX_NODES);
    return m_Data[Index];
}

template<typename DataType>
void FixedContainer<DataType>::Clear()
{
    m_FreeNodes.Clear();
    for (unsigned int i = 0; i < MAX_NODES; i++)
    {
        m_FreeNodes.EnQueue(i);
    }
}
