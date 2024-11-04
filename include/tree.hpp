#pragma once
#include "cstring"
#include "sstream"
#include "string"
#include "exception"
#include "vector"
#include "queue"
//NEED TO DELETE
#include <iostream>

namespace cs251
{
	typedef int handle;

	class invalid_handle : public std::runtime_error {
		public: invalid_handle() : std::runtime_error("Invalid handle!") {} };
	class recycled_node : public std::runtime_error {
		public: recycled_node() : std::runtime_error("Node is recycled!") {} };

	template<typename tree_node_data>
	class tree_node {
		//Friend class grant private access to another class.
		template<typename tnd>
		friend class tree;

		//TODO: You may add private members/methods here.

		/**
		 * The handle of current node, should be the index of current node within the vector array in tree.
		 */
		handle m_handle = -1;
		/**
		 * Whether the node is recycled.
		 */
		bool m_recycled = false;
		/**
		 * The content of the node.
		 */
		tree_node_data m_data = {};
		/**
		 * The handle of the parent node.
		 */
		handle m_parentHandle = -1;
		/**
		 * List of handles to all children.
		 */
		std::vector<handle> m_childrenHandles{};

		std::vector<handle> peek_children_handles_copy{};

	public:
		/**
		 * \brief Retrieve the data for this node.
		 * \return The modifiable reference to the node's data.
		 */
		tree_node_data& ref_data();
		/**
		 * \brief Check if the node is recycled.
		 * \return Whether this node is recycled or not.
		 */
		bool is_recycled() const;
		/**
		 * \brief Get the handle of this node.
		 * \return The handle of this node.
		 */
		handle get_handle() const;
		/**
		 * \brief Get the handle of this node's parent.
		 * \return The handle of this node's parent.
		 */
		handle get_parent_handle() const;
		/**
		 * \brief Get the list of handles of this node's children.
		 * \return The list of handles of this node's children.
		 */
		const std::vector<handle>& peek_children_handles() const;
	private: 
		void modifier();

		
	};

	template<typename tree_node_data>
	class tree
	{
	public:
		/**
		 * \brief The constructor of the tree class. You should allocate the root node here.
		 */
		tree();
		/**
		 * \brief Allocate a new node as root from pool or creating a new one.
		 * \return The handle of the new node.
		 */
		handle allocate(handle parentHandle);
		/**
		 * \brief Remove (recycle) a node. Remove all descendent nodes.
		 * \param handle The handle of the target node to be removed.
		 */
		void remove(handle handle);
		/**
		 * \brief Attach a node to another node as its child.
		 * \param targetHandle The handle of the target node as child.
		 * \param parentHandle The handle of the parent node.
		 */
		void set_parent(handle targetHandle, handle parentHandle);
		/**
		 * \brief Return the constant reference to the list of nodes.
		 * \return Constant reference to the list of nodes.
		 */
		const std::vector<tree_node<tree_node_data>>& peek_nodes() const;
		/**
		 * \brief Retrieve the node with its handle.
		 * \param handle The handle of the target node.
		 * \return The reference to the node.
		 */
		tree_node<tree_node_data>& ref_node(handle handle);
	private:
		//TODO: You may add private members/methods here.

		/**
		 * The storage for all nodes.
		 */
		std::vector<tree_node<tree_node_data>> m_nodes {};
		/**
		 * The pool that keep track of the recycled nodes.
		 */
		std::queue<handle> m_node_pool {};
	};

	template <typename tree_node_data>
	tree_node_data& tree_node<tree_node_data>::ref_data()
	{
		if (m_recycled) {
			throw recycled_node();
		}
		return m_data;
	}

	template <typename tree_node_data>
	bool tree_node<tree_node_data>::is_recycled() const
	{
		return m_recycled;
	}

	template <typename tree_node_data>
	handle tree_node<tree_node_data>::get_handle() const
	{
		return m_handle;
	}

	template <typename tree_node_data>
	handle tree_node<tree_node_data>::get_parent_handle() const
	{
		if (!m_recycled) {
			return m_parentHandle;
		}
		else {
			throw recycled_node();
		}
	}

	template <typename tree_node_data>
	const std::vector<handle>& tree_node<tree_node_data>::peek_children_handles() const
	{
		if (!m_recycled) {
			return m_childrenHandles;
		}
		else {
			throw recycled_node();
		}	
		/*
		modifier();
		if (!m_recycled) {
			return peek_children_handles_copy;
		}
		else {
			throw recycled_node();
		}	
		*/
	}

	template <typename tree_node_data>
	void tree_node<tree_node_data>::modifier() {
		peek_children_handles_copy.clear();
		for (int i = 0; i < m_childrenHandles.size(); i++) {
			peek_children_handles_copy.push_back(m_childrenHandles[i] - 1);
		}	
	}


	template <typename tree_node_data>
	tree<tree_node_data>::tree()
	{
		tree_node<tree_node_data> root {};
		root.m_handle = 0;
		m_nodes.push_back(root);
	}

	template <typename tree_node_data>
	handle tree<tree_node_data>::allocate(handle parentHandle)
	{	

		if ((parentHandle < 0) || (parentHandle >= m_nodes.size())) {
			throw invalid_handle();
		}

		tree_node<tree_node_data> a = m_nodes[parentHandle];


		if (m_nodes[parentHandle].m_recycled) {
			throw recycled_node();
		}

		handle temp_handle;

		if (m_node_pool.empty()) {
			tree_node<tree_node_data> temp{};
			temp.m_handle = m_nodes.size();
			temp.m_parentHandle = parentHandle;
			temp.m_recycled = false;
			temp_handle = m_nodes.size();
			m_nodes.push_back(temp);
		} else {
			temp_handle = m_node_pool.front();
			m_node_pool.pop();
			tree_node<tree_node_data> temp = m_nodes[temp_handle];
			m_nodes[temp_handle].m_handle = temp_handle;
			m_nodes[temp_handle].m_parentHandle = parentHandle;
			m_nodes[temp_handle].m_recycled = false;
		}

		m_nodes[parentHandle].m_childrenHandles.push_back(temp_handle);

		return temp_handle;
		
	}

	template <typename tree_node_data>
	void tree<tree_node_data>::remove(const handle handle)
	{
		if ((handle >= m_nodes.size()) || (handle < 0)) {
			throw invalid_handle();
		}

		tree_node<tree_node_data> temp = m_nodes[handle];

		if (temp.m_recycled) {
			throw recycled_node();
		}

		int curr_child;

		while (!m_nodes[handle].m_childrenHandles.empty()) {
			curr_child = m_nodes[handle].m_childrenHandles.front();
			remove(curr_child);
			//m_nodes[handle].m_childrenHandles.erase(m_nodes[handle].m_childrenHandles.begin());
		}

		m_nodes[handle].m_recycled = true;
		
		int index = -1;

		for (int i = 0; i < m_nodes[m_nodes[handle].m_parentHandle].m_childrenHandles.size(); i++) {
			if (m_nodes[m_nodes[handle].m_parentHandle].m_childrenHandles[i] == handle) {
				index = i;
				break;
			}
		}

		if (index == -1) {
			throw invalid_handle();
		}

		m_nodes[m_nodes[handle].m_parentHandle].m_childrenHandles.erase(m_nodes[m_nodes[handle].m_parentHandle].m_childrenHandles.begin() + index);
		
		m_node_pool.push(handle);
		
	}

	template <typename tree_node_data>
	void tree<tree_node_data>::set_parent(const handle targetHandle, const handle parentHandle)
	{
		if ((targetHandle >= m_nodes.size()) || (parentHandle >= m_nodes.size())) {
			throw invalid_handle();
		}

		if ((parentHandle < 0) || (targetHandle < 0)) {
			throw invalid_handle();
		}

		tree_node<tree_node_data> temp = m_nodes[targetHandle];
		tree_node<tree_node_data> parent_temp = m_nodes[parentHandle];

		if ((temp.m_recycled) || (parent_temp.m_recycled)) {
			throw recycled_node();
		}


		for (int i = 0; i < m_nodes[temp.m_parentHandle].m_childrenHandles.size(); i++) {
			if (m_nodes[temp.m_parentHandle].m_childrenHandles[i] == targetHandle) {
				m_nodes[temp.m_parentHandle].m_childrenHandles.erase(m_nodes[temp.m_parentHandle].m_childrenHandles.begin() + i);
			}
		}
		

		m_nodes[targetHandle].m_parentHandle = parentHandle;
		m_nodes[parentHandle].m_childrenHandles.push_back(targetHandle);
		
	}

	template <typename tree_node_data>
	const std::vector<tree_node<tree_node_data>>& tree<tree_node_data>::peek_nodes() const
	{
		return m_nodes;
	}

	template <typename tree_node_data>
	tree_node<tree_node_data>& tree<tree_node_data>::ref_node(handle t_handle)
	{
		if ((t_handle >= m_nodes.size()) || (t_handle < 0)) {
			throw invalid_handle();
		}

		return m_nodes[t_handle];
	}
}
