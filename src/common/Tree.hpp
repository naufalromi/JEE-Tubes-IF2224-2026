#pragma once

#include <string>
#include <string>
#include <vector>
#include <memory>
#include "NodeType.hpp"

class TreeNode {
public:
	NodeType type;
	std::string value;
	std::vector<std::shared_ptr<TreeNode>> children;

	TreeNode(NodeType, const std::string& = "");
	void addChild(const std::shared_ptr<TreeNode>& child);

	std::string typeString() const {
		return nodeTypeToString(type);
	}
};
