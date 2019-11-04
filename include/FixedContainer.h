/**
* @file FixedContainer.h
* @brief 基于大数组的节点分配类，相当于内存池
*/

#ifndef _FIXED_CONTAINER_H_
#define _FIXED_CONTAINER_H_

#include <stdio.h>
#include "Queue.h"

namespace Algorithm
{

/// 固定大小的容器，简易的专用内存池
template<typename DataType>
class FixedContainer
{
public:
	/// 构造函数
	/// @param MaxNodes 最大节点数
	FixedContainer(unsigned int MaxNodes);

	/// 析构函数
	~FixedContainer();

	/// 容器是否为空
	bool IsEmpty() const;

	/// 容器是否满
	bool IsFull() const;

	/// 空闲节点的数量
	unsigned int FreeNodesCount() const;

	/// 已使用节点的数量
	unsigned int UsedNodesCount() const;

	/// 总节点的数量
	unsigned int NodesCount() const;

	/// 得到空闲节点
	DataType* GetNode();

	/// 释放节点
	bool FreeNode(DataType& Node);

	/// 清空整个容器
	void SetZero();  //清零

#ifndef NDEBUG
	/// 打印关键信息，用于调试
	void Print();
#endif

	/// 根据下标访问内容
	DataType& operator[](int Index);

	/// 清空所有节点
	void Clear();
private:
	FixedContainer();
	FixedContainer(const FixedContainer& rsh);
	FixedContainer& operator=(const FixedContainer& rsh);
//-----------------------------------------------------------------------------
protected:

private:
	const unsigned int MAX_NODES;
	DataType* m_Data;   ///< 存放数据的指针
	Queue<unsigned int> m_FreeNodes;   ///< 空闲节点的下标
};

#include "../src/FixedContainer.hpp"

};  //end namespace Algorithm

#endif

/*
内存池第二版的开发思路：
1、对外的节点引用为数字ID，不是具体的指针；（通过下标返回节点的引用）
2、可以大块的分配，也可以单个节点分配；
3、允许动态增长；
4、允许释放节点；
*/
