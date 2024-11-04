#include "filesystem.hpp"
#include <iostream>

using namespace cs251;

filesystem::filesystem(const size_t sizeLimit)
{
	m_sizeLimit = sizeLimit;
}

handle filesystem::create_file(const size_t fileSize, const std::string& fileName)
{
	for (int i = 0; i < fileName.size(); i++) {
		if (fileName[i] == '/') {
			throw invalid_name();
		}
	}

	if (fileSize + m_currSize > m_sizeLimit) {
		throw exceeds_size();
	}

	if (directorySearch(0, fileName)) {
		throw file_exists();
	}

	handle spot = m_fileSystemNodes.allocate(0);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::File;
	curr.ref_data().m_name = fileName;
	curr.ref_data().m_fileSize = fileSize;

	maxHeap.push(fileSize, spot);
	m_currSize += fileSize;
	return spot;
}

handle filesystem::create_directory(const std::string& directoryName)
{
	for (int i = 0; i < directoryName.size(); i++) {
		if (directoryName[i] == '/') {
			throw invalid_name();
		}
	}

	if (directorySearch(0, directoryName)) {
		throw directory_exists();
	}

	handle spot = m_fileSystemNodes.allocate(0);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::Directory;
	curr.ref_data().m_name = directoryName;

	return spot;
}

handle filesystem::create_link(const handle targetHandle, const std::string& linkName)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	for (int i = 0; i < linkName.size(); i++) {
		if (linkName[i] == '/') {
			throw invalid_name();
		}
	}

	if (directorySearch(0, linkName)) {
		throw link_exists();
	}

	handle spot = m_fileSystemNodes.allocate(0);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::Link;
	curr.ref_data().m_name = linkName;
	curr.ref_data().m_linkedHandle = targetHandle;

	return spot;
}

bool filesystem::remove(const handle targetHandle)
{
	if ((targetHandle < 0 ) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}
	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}
	if (m_fileSystemNodes.ref_node(targetHandle).ref_data().m_type == node_type::Directory) {
		if (!m_fileSystemNodes.ref_node(targetHandle).peek_children_handles().empty()) {
			return false;
		}
		m_fileSystemNodes.remove(targetHandle);
		return true;
	} else if (m_fileSystemNodes.ref_node(targetHandle).ref_data().m_type == node_type::Link) {
		m_fileSystemNodes.remove(targetHandle);
		return true;
	}
	else {
		m_currSize = m_currSize - m_fileSystemNodes.ref_node(targetHandle).ref_data().m_fileSize;
		m_fileSystemNodes.remove(targetHandle);
		maxHeap.remove(targetHandle);
		return true;
	}
	return false;
}

handle filesystem::create_file(const size_t fileSize, const std::string& fileName, const handle parentHandle)
{
	if ((parentHandle < 0) || (parentHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	tree_node<filesystem_node_data>& parent = m_fileSystemNodes.ref_node(parentHandle);

	if (m_fileSystemNodes.ref_node(parentHandle).is_recycled()) {
		throw invalid_handle();
	}
	
	if (parent.ref_data().m_type == node_type::Link) {
		handle tempHandle = follow(parentHandle);
		parent = m_fileSystemNodes.ref_node(tempHandle);
	}
	

	if (parent.ref_data().m_type != node_type::Directory) {
		throw invalid_handle();
	}

	for (int i = 0; i < fileName.size(); i++) {
		if (fileName[i] == '/') {
			throw invalid_name();
		}
	}

	if ((fileSize + m_currSize) > m_sizeLimit) {
		throw exceeds_size();
	}

	if (directorySearch(parentHandle, fileName)) {
		throw file_exists();
	}

	handle spot = m_fileSystemNodes.allocate(parentHandle);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::File;
	curr.ref_data().m_name = fileName;
	curr.ref_data().m_fileSize = fileSize;

	maxHeap.push(fileSize, spot);
	m_currSize += fileSize;
	return spot;
}

handle filesystem::create_directory(const std::string& directoryName, const handle parentHandle)
{
	if ((parentHandle < 0) || (parentHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(parentHandle).is_recycled()) {
		throw invalid_handle();
	}

	tree_node<filesystem_node_data>& parent = m_fileSystemNodes.ref_node(parentHandle);
	
	if (parent.ref_data().m_type == node_type::Link) {
		handle tempHandle = follow(parentHandle);
		parent = m_fileSystemNodes.ref_node(tempHandle);
	}
	

	if (parent.ref_data().m_type != node_type::Directory) {
		throw invalid_handle();
	}

	for (int i = 0; i < directoryName.size(); i++) {
		if (directoryName[i] == '/') {
			throw invalid_name();
		}
	}

	if (directorySearch(parentHandle, directoryName)) {
		throw directory_exists();
	}

	handle spot = m_fileSystemNodes.allocate(parentHandle);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::Directory;
	curr.ref_data().m_name = directoryName;

	return spot;

}

handle filesystem::create_link(const handle targetHandle, const std::string& linkName, const handle parentHandle)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}

	if ((parentHandle < 0) || (parentHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(parentHandle).is_recycled()) {
		throw invalid_handle();
	}

	tree_node<filesystem_node_data>& parent = m_fileSystemNodes.ref_node(parentHandle);
	
	if (parent.ref_data().m_type == node_type::Link) {
		handle tempHandle = follow(parentHandle);
		parent = m_fileSystemNodes.ref_node(tempHandle);
	}
	

	if (parent.ref_data().m_type != node_type::Directory) {
		throw invalid_handle();
	}

	for (int i = 0; i < linkName.size(); i++) {
		if (linkName[i] == '/') {
			throw invalid_name();
		}
	}

	if (directorySearch(parentHandle, linkName)) {
		throw link_exists();
	}

	handle spot = m_fileSystemNodes.allocate(parentHandle);

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(spot);

	curr.ref_data().m_type = node_type::Link;
	curr.ref_data().m_name = linkName;
	curr.ref_data().m_linkedHandle = targetHandle;


	
	return spot;
}

std::string filesystem::get_absolute_path(const handle targetHandle)
{
	handle root = 0;

	handle curr = targetHandle;

	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}

	pathway.clear();

	while (curr != root) {
		pathway.push_back(m_fileSystemNodes.ref_node(curr).ref_data().m_name);
		curr = m_fileSystemNodes.ref_node(curr).get_parent_handle();
	}

	std::string finalPath = "/";


	for (int i = pathway.size() - 1; i >= 0; i--) {
		finalPath += pathway[i] + "/";
	}
	
	finalPath = finalPath.substr(0, finalPath.length() - 1);

	return finalPath;

}

std::string filesystem::get_name(const handle targetHandle)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}
	
	return m_fileSystemNodes.ref_node(targetHandle).ref_data().m_name;
}

void filesystem::rename(const handle targetHandle, const std::string& newName)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}

	for (int i = 0; i < newName.size(); i++) {
		if (newName[i] == '/') {
			throw invalid_name();
		}
	}

	handle parent = m_fileSystemNodes.ref_node(targetHandle).get_parent_handle();

	if (directorySearch(parent, newName)) {
		throw name_exists();
	}

	m_fileSystemNodes.ref_node(targetHandle).ref_data().m_name = newName;
}

bool filesystem::exist(const handle targetHandle)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		return false;
	}

	tree_node<filesystem_node_data>& curr = m_fileSystemNodes.ref_node(targetHandle);

	if (curr.is_recycled()) {
		return false;
	}

	return true;
}

handle filesystem::get_handle(const std::string& absolutePath)
{
	pathway.clear();
	std::string temp;

	for (char c : absolutePath) {
		if (c == '/') {
			if (!temp.empty()) {
				pathway.push_back(temp);
				temp.clear();
			}
		}
		else {
			temp += c;
		}
		
	}
	if (!temp.empty()) {
		pathway.push_back(temp);
	}

	return getHandle(0);
}

handle filesystem::getHandle(handle currentHandle) {
	if (!(exist(currentHandle))) {
		throw invalid_path();
	}
	if (pathway.empty()) {
		return currentHandle;
	}
	else {
		if (m_fileSystemNodes.ref_node(currentHandle).ref_data().m_type == node_type::Directory) {
			for (handle child: m_fileSystemNodes.ref_node(currentHandle).peek_children_handles()) {
				if (m_fileSystemNodes.ref_node(child).ref_data().m_name == pathway[0]) {
					pathway.erase(pathway.begin());
					return getHandle(child);
				}
			}
		}
		else if (m_fileSystemNodes.ref_node(currentHandle).ref_data().m_type == node_type::Link) {
			handle link = follow(currentHandle);
			return getHandle(link);
		}
		throw invalid_path();
	}
	throw invalid_path();
}

handle filesystem::follow(const handle targetHandle)
{
	if (!(exist(targetHandle))) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).ref_data().m_type == node_type::Link) {
		return follow(m_fileSystemNodes.ref_node(targetHandle).ref_data().m_linkedHandle);
	}
	else {
		return targetHandle;
	}
}

handle filesystem::get_largest_file_handle() const
{
	return maxHeap.top();
}

size_t filesystem::get_available_size() const
{
	return (m_sizeLimit - m_currSize);
}

size_t filesystem::get_file_size(const handle targetHandle)
{
	if ((targetHandle < 0) || (targetHandle >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).is_recycled()) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(targetHandle).ref_data().m_type == node_type::Directory) {
		throw invalid_handle();
	}

	else if (m_fileSystemNodes.ref_node(targetHandle).ref_data().m_type == node_type::Link) {
		handle linked = m_fileSystemNodes.ref_node(targetHandle).ref_data().m_linkedHandle;
		return get_file_size(linked);
	}
	else {
		return m_fileSystemNodes.ref_node(targetHandle).ref_data().m_fileSize;
	}
}


size_t filesystem::get_file_size(const std::string& absolutePath)
{	
	handle a = get_handle(absolutePath);
	
	return get_file_size(a);
}

bool filesystem::directorySearch(handle curr, std::string targetName) 
{
	if ((curr < 0) || (curr >= m_fileSystemNodes.peek_nodes().size())) {
		throw invalid_handle();
	}

	if (m_fileSystemNodes.ref_node(curr).is_recycled()) {
		throw invalid_handle();
	}

	std::vector<handle> childs = m_fileSystemNodes.ref_node(curr).peek_children_handles();
	for (int i = 0; i < m_fileSystemNodes.ref_node(curr).peek_children_handles().size(); i++) {
		if (m_fileSystemNodes.ref_node(childs[i]).ref_data().m_name == targetName) {
			return true;
		}
	}

	return false;

}

std::string filesystem::print_layout()
{
	std::stringstream ss{};
	const auto& node = m_fileSystemNodes.ref_node(0);
	for (const auto& childHandle : node.peek_children_handles()) {
		print_traverse(0, ss, childHandle);
	}
	return ss.str();
}

void filesystem::print_traverse(const size_t level, std::stringstream& ss, const handle targetHandle)
{
	auto& node = m_fileSystemNodes.ref_node(targetHandle);
	std::stringstream indentation{};
	for (auto i = level; i > 0; i--)
	{
		indentation << "\t";
	}
	std::string type{};
	switch (node.ref_data().m_type)
	{
	case node_type::Directory: type = "[D]"; break;
	case node_type::Link: type = "[L]"; break;
	case node_type::File: type = "[F]"; break;
	} 
	ss << indentation.str() << type << node.ref_data().m_name;
	if (node.ref_data().m_type == node_type::Link)
	{
		try {
			const auto path = get_absolute_path(follow(node.get_handle()));
			ss << " [->" << path << "]";
		}
		catch (const std::exception& e)
		{
			ss << " [invalid]";
		}
	}
	else if (node.ref_data().m_type == node_type::File)
	{
		ss << " (size = " << std::to_string(node.ref_data().m_fileSize) << ")";
	}
	ss << std::endl;
	for (const auto& childHandle : node.peek_children_handles())
	{
		print_traverse(level + 1, ss, childHandle);
	}
}