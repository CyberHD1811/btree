#ifndef BTREE_CYBERHD_H
#define BTREE_CYBERHD_H

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

template<typename T>
class Node {
public:
    using key_type = T;
    using size_type = std::size_t;
    using key_vector = std::vector<key_type>;
    using node_type = std::unique_ptr<Node>;
    using node_vector = std::vector<node_type>;

    using insert_pair = std::optional<std::pair<key_type, node_type>>;
    using key_search_pair = std::pair<bool, size_type>;

private:
    const size_type d;
    key_vector keys;
    node_vector children;

public:
    explicit Node(const size_type d) : d{d},
                                       keys{},
                                       children{} {
        assert(d >= 2 && d <= std::numeric_limits<size_type>::max() / 2 - 1);

        keys.reserve(2 * d - 1);
        children.reserve(2 * d);
    }

    insert_pair insert(key_type key) {
        auto [found, index] = key_search(key);

        if (found) return std::nullopt;

        if (is_leaf()) { keys.insert(keys.begin() + index, key); }
        else {
            insert_pair split{children[index]->insert(key)};

            if (split) {
                auto [split_key, right] = *split;

                keys.insert(keys.begin() + index, split_key);
                children.insert(children.begin() + index + 1, std::move(right));
            }
        }

        if (!is_overflowing()) return std::nullopt;

        size_type middle_index{d - 1};
        key_type middle_key{keys[middle_index]};

        node_type right = std::make_unique<Node>(d);
        right->keys.assign(keys.begin() + middle_index + 1, keys.end());
        keys.assign(keys.begin(), keys.begin() + middle_index);

        if (!is_leaf()) {
            right->children.assign(std::make_move_iterator(children.begin() + middle_index + 1),
                                   std::make_move_iterator(children.end()));
            children.assign(std::make_move_iterator(children.begin()),
                            std::make_move_iterator(children.begin() + middle_index + 1));
        }

        return std::make_optional(middle_key, std::move(right));
    }

    bool is_leaf() const { return children.empty(); }

    bool is_minimal() const { return keys.size() == d - 1; }

    bool is_overflowing() const { return keys.size() == 2 * d - 1; }

    bool is_underflowing() const { return keys.size() == d - 2; }

    key_search_pair key_search(const key_type key) const {
        auto it{std::lower_bound(keys.begin(), keys.end(), key)};

        return {it != keys.end() && *it == key, static_cast<size_type>(it - keys.begin())};
    }

    bool remove(key_type key) {
        auto [found, index] = key_search(key);

        if (is_leaf()) {
            if (!found) return false;

            keys.erase(keys.begin() + index);
            return true;
        }

        size_type offset_index{index};
        if (found) {
            ++offset_index;
            swap(keys[index], offset_index);
        }
        bool deleted{children[offset_index]->remove(key)};

        if (children[offset_index]->is_underflowing()) {
            node_type &u_child{children[offset_index]};

            if (offset_index != 0 && !children[offset_index - 1]->is_minimal()) {
                node_type &l_child{children[offset_index - 1]};
                u_child->keys.insert(u_child->keys.begin(), keys[offset_index - 1]);
                auto k_insert_it{keys.erase(keys.begin() + offset_index - 1)};

                keys.insert(k_insert_it, l_child->keys.back());
                l_child->keys.pop_back();

                if (!u_child->is_leaf()) {
                    u_child->children.insert(u_child->children.begin(), std::move(l_child->children.back()));
                    l_child->children.pop_back();
                }
            }
            else if (offset_index + 1 < children.size() && !children[offset_index + 1]->is_minimal()) {
                node_type &r_child{children[offset_index + 1]};

                u_child->keys.push_back(keys[offset_index]);
                auto k_insert_it{keys.erase(keys.begin() + offset_index)};

                keys.insert(k_insert_it, r_child->keys.front());
                r_child->keys.erase(r_child->keys.begin());

                if (!u_child->is_leaf()) {
                    u_child->children.push_back(std::move(r_child->children.front()));
                    r_child->children.erase(r_child->children.begin());
                }
            }
            else if (offset_index != 0) {
                node_type &l_child{children[offset_index - 1]};
                u_child->keys.insert(u_child->keys.begin(), keys[offset_index - 1]);
                auto k_insert_it{keys.erase(keys.begin() + offset_index - 1)};

                u_child->keys.insert(u_child->keys.begin(), l_child->keys.begin(), l_child->keys.end());

                if (!u_child->is_leaf()) {
                    u_child->children.insert(u_child->children.begin(),
                                             std::make_move_iterator(l_child->children.begin()),
                                             std::make_move_iterator(l_child->children.end()));
                }

                children.erase(children.begin() + offset_index - 1);
            }
            else {
                node_type &r_child{children[offset_index + 1]};

                u_child->keys.push_back(keys[offset_index]);
                auto k_insert_it{keys.erase(keys.begin() + offset_index)};

                u_child->keys.insert(u_child->keys.end(), r_child->keys.begin(), r_child->keys.end());

                if (!u_child->is_leaf()) {
                    u_child->children.insert(u_child->children.end(),
                                             std::make_move_iterator(r_child->children.begin()),
                                             std::make_move_iterator(r_child->children.end()));
                }
                children.erase(children.begin() + 1);
            }
        }

        return deleted;
    }

    bool search(const key_type key) const {
        auto [found, index] = key_search(key);

        return found || (!is_leaf() && children[index]->search(key));
    }

    /**
     * Swaps a specified key with the smallest key >= to itself in the right subtree of it.
     *
     * @param key TODO
     * @param index TODO
     */
    void swap(key_type &key, size_type index) {
        is_leaf() ? std::swap(key, keys[index]) : children[index]->swap(key, 0);
    }

    friend class BTree;
};

template<typename T>
class BTree {
public:
    using key_type = T;
    using size_type = std::size_t;
    using node_type = std::unique_ptr<Node<key_type>>;

private:
    size_type d;
    node_type root;

public:
    explicit BTree(const size_type d) : d{d},
                                        root{std::make_unique<Node<key_type>>(d)} {
        assert(d >= 2 && d <= std::numeric_limits<size_type>::max() / 2 - 1);
    }

    void insert(key_type key) {
        typename Node<key_type>::insert_pair split{root->insert(key)};

        if (split) {
            node_type new_root{std::make_unique<Node<key_type>>(d)};

            auto [split_key, right] = *split;

            new_root->children.push_back(std::move(root));
            new_root->keys.push_back(split_key);
            new_root->children.push_back(std::move(right));

            root = std::move(new_root);
        }
    }

    bool remove(key_type key) {
        const bool removed{root->remove(key)};

        if (root->keys.size() == 0 && root->children.size() == 1) { root = std::move(root->children.front()); }

        return removed;
    }

    bool search(const key_type key) const { return root->search(key); }
};

#endif
