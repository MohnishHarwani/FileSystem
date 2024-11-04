#include "file_size_max_heap.hpp"
#include <iostream>
using namespace cs251;

void file_size_max_heap::push(const size_t fileSize, const handle handle)
{
	file_size_max_heap_node a{};
	a.m_handle = handle;
	a.m_value = fileSize;

	int i = m_nodes.size();
	m_nodes.push_back(a);
	int p = (i - 1) / 2;

	while ((i > 0) && (m_nodes[i].m_value > m_nodes[p].m_value)) {
		a = m_nodes[i];
		m_nodes[i] = m_nodes[p];
		m_nodes[p] = a;
		i = p;
		p = (p - 1) / 2;
	}

	m_nodeSize++;
}

handle file_size_max_heap::top() const
{
	if (m_nodes.size() == 0) {
		throw heap_empty();
	}
	return m_nodes[0].m_handle;

}

void file_size_max_heap::remove(const handle handleRem) {
	if (m_nodes.size() == 0) {
		throw heap_empty();
	}

	int slotToRemove = -1;

	for (int i = 0; i < m_nodes.size(); i++) {
		if (m_nodes[i].m_handle == handleRem) {
			slotToRemove = i;
		}
	}

	if (slotToRemove == -1) {
		throw invalid_handle();
	}

	file_size_max_heap_node a = m_nodes[m_nodes.size() - 1];
	m_nodes[m_nodes.size() - 1] = m_nodes[slotToRemove];
	m_nodes[slotToRemove] = a;

	m_nodes.pop_back();

	siftdown(slotToRemove);

	m_nodeSize--;

	
}


void file_size_max_heap::siftdown(int i) {
	int m = maxChild(i);
	file_size_max_heap_node a{};
	while ((m < m_nodes.size()) && (m_nodes[i].m_value < m_nodes[m].m_value)) {
		a = m_nodes[i];
		m_nodes[i] = m_nodes[m];
		m_nodes[m] = a;
		i = m;
		m = maxChild(i);
	}
}

int file_size_max_heap::maxChild(int i) {
	int left_child = (2 * i) + 1;
	if (left_child >= m_nodes.size()) {
		return m_nodes.size();
	}
	int right_child = (2 * i) + 2;
	if (right_child >= m_nodes.size()) {
		return left_child;
	}
	if (m_nodes[left_child].m_value > m_nodes[right_child].m_value) {
		return left_child;
	} else {
		return right_child;
	}
}
