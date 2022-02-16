#pragma once
#include <array>
#include <cstring>
/**
 * @brief node of binary tree
 *
 */
template <typename ElemType, std::size_t Dim>
class Node
{
public:
    Node(){};

    Node(const ElemType *data, const size_t axis = 0, const size_t level = 0)
    {
        this->axis = axis;
        this->level = level;
        memcpy(&this->data[0], data, sizeof(ElemType) * Dim);
    };

    ~Node()
    {

        if (nullptr != left)
        {
            delete left;
        }

        if (nullptr != right)
        {
            delete right;
        }
    };

    ElemType &operator[](std::size_t index)
    {
        return this->data[index];
    };

    ElemType operator[](std::size_t index) const
    {
        return this->data[index];
    };

public:
    Node<ElemType, Dim> *left = nullptr;
    Node<ElemType, Dim> *right = nullptr;
    // ElemType *data = nullptr;
    std::array<ElemType, Dim> data;
    size_t axis = 0;
    size_t level = 0;
    bool isLeaf = false;
};