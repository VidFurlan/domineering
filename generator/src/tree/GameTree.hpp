#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "generator/Bitboard.hpp"
#include "tree/TreeNode.hpp"

template <Bitboard T>
class GameTree {
   public:
    GameTree();
    ~GameTree() = default;

    void addNode(const TreeNode<T>& node);
    TreeNode<T>& getNodeByIdx(uint32_t index);
    TreeNode<T>& getNodeByState(const T& state);
    const int32_t findNode(const T& state) const;
    const size_t size() const { return m_nodes.size(); }
    const void clear() {
        m_nodes.clear();
        m_nodeMap.clear();
    }
    const void saveToFileEdgeList(const std::string& filename) const;
    const void saveToFileJson(const std::string& filename) const;

   private:
    std::vector<TreeNode<T>> m_nodes;
    std::unordered_map<T, uint32_t> m_nodeMap;
};

template <Bitboard T>
inline GameTree<T>::GameTree() {
    m_nodes.reserve(1ll << 20);
}

template <Bitboard T>
inline void GameTree<T>::addNode(const TreeNode<T>& node) {
    if (findNode(node.state) != -1) return;
    m_nodes.push_back(node);
    m_nodeMap[node.state] = m_nodes.size() - 1;
}

template <Bitboard T>
inline TreeNode<T>& GameTree<T>::getNodeByIdx(uint32_t index) {
    return m_nodes[index];
}

template <Bitboard T>
inline TreeNode<T>& GameTree<T>::getNodeByState(const T& state) {
    auto it = m_nodeMap.find(state);
    if (it != m_nodeMap.end()) {
        return m_nodes[it->second];
    }
    throw std::out_of_range("State not found in GameTree");
}

template <Bitboard T>
inline const int32_t GameTree<T>::findNode(const T& state) const {
    auto it = m_nodeMap.find(state);
    if (it != m_nodeMap.end()) {
        return it->second;
    }
    return -1;
}

template <Bitboard T>
inline const void GameTree<T>::saveToFileEdgeList(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to save GameTree to file " + filename);
    }

    for (const auto& node : m_nodes) file << node.color << " ";
    file << "\n";

    for (const auto& node : m_nodes) file << node.state << " ";
    file << "\n";

    std::vector<std::pair<uint32_t, uint32_t>> edges;
    for (const auto& node : m_nodes) {
        for (const auto& child : node.getChildren()) {
            edges.emplace_back(&node - &m_nodes[0], child);
        }
    }
    file << edges.size() << "\n";
    for (const auto& [u, v] : edges) file << u << " " << v << "\n";

    file.close();
}

template <Bitboard T>
static std::string toHexString(T x) {
    std::ostringstream ss;
    ss << "0x" << std::hex << std::uppercase << (uint64_t)x;
    return ss.str();
}

template <Bitboard T>
inline const void GameTree<T>::saveToFileJson(const std::string& filename) const {
    using nlohmann::json;

    json root;
    root["nodes"] = json::array();
    root["edges"] = json::array();

    for (size_t i = 0; i < m_nodes.size(); ++i) {
        const auto& node = m_nodes[i];

        root["nodes"].push_back({
            {"id", i},
            {"state", toHexString(node.state)}, // string
            {"color", node.color}
        });
    }

    for (size_t u = 0; u < m_nodes.size(); ++u) {
        for (uint32_t v : m_nodes[u].getChildren()) {
            root["edges"].push_back({
                {"u", u},
                {"v", v}
            });
        }
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to save GameTree to file " + filename);
    }

    file << root.dump(2) << '\n';
    if (!file) {
        throw std::runtime_error("Failed while writing GameTree to file " + filename);
    }
}
