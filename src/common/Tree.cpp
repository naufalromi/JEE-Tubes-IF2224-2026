#include "Tree.hpp"
#include "NodeType.hpp"

TreeNode::TreeNode(const NodeType type_, const std::string& value_) : type(type_), value(value_) 
{

}

void TreeNode::addChild(const std::shared_ptr<TreeNode>& child) {
    if (child == nullptr) return;
    children.push_back(child);
}