#pragma once
#include "Node.hpp"
#include <memory>
#include <vector>
#include <iostream>
#include <array>
#include <queue>
#include <algorithm>

template <typename ElemType, std::size_t Dim>
class KDtree
{
public:
    typedef typename std::array<ElemType, Dim> Point;

public:
    ~KDtree()
    {
        if (_root != nullptr)
        {
            delete _root;
            _root = nullptr;
        }
    }

    int BuildTree(const ElemType *pdata, const int nums);

    int SearchRadius(const Point &query, const double radius, const bool order, std::vector<Point> &points, std::vector<double> &dist2);

    int SearchNN(const int knn, const bool order, std::vector<Point> &points);

    static double distance_sq(const Point &p1, const Point &p2)
    {
        double sum = 0;
        for (size_t i = 0; i < Dim; i++)
        {
            sum += (p1[i] - p2[i]) * (p1[i] - p2[i]);
        }
        return sum;
    }

private:
    Node<ElemType, Dim> *createNode(typename std::vector<Point>::iterator beg, typename std::vector<Point>::iterator end, size_t axis, size_t level);

    /**
     * @brief find radius range first node
     *
     * @param curNode
     * @param query
     * @return int
     */
    int radiusSearchRecurse(const Node<ElemType, Dim> *curNode, const Point &query, const double dist_sq_limit, std::vector<Point> &points, std::vector<double> &dist_sq_serched);

    int nearestSearcher();

    int iterNode(const Node<ElemType, Dim> *node, std::priority_queue<Point> &points);

private:
    Node<ElemType, Dim> *_root = nullptr;
};

#pragma region public functions

template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::BuildTree(const ElemType *pdata, const int nums)
{

    std::vector<Point> points(nums);
    memcpy(&points[0][0], pdata, sizeof(ElemType) * nums * Dim);
    int start_axis = 0;
    _root = createNode(points.begin(), points.end(), start_axis, 0);
    return true;
};

template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::SearchRadius(const Point &query, const double radius,
                                        const bool order, std::vector<Point> &points, std::vector<double> &dist2)
{
    points.resize(0);
    dist2.resize(0);

    int found = this->radiusSearchRecurse(_root, query, radius * radius, points, dist2);
    if (!found)
    {
        return false;
    }
    return true;
};

template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::SearchNN(const int knn, const bool order, std::vector<Point> &points)
{
    return true;
};

#pragma endregion

#pragma region private functions

template <typename ElemType, std::size_t Dim>
Node<ElemType, Dim> *KDtree<ElemType, Dim>::createNode(typename std::vector<Point>::iterator beg, typename std::vector<Point>::iterator end, size_t axis, size_t level)
{
    auto cmp = [axis](const Point &p1, const Point &p2)
    {
        return p1[axis] < p2[axis];
    };

    std::size_t len = end - beg;
    auto mid = beg + len / 2;
    std::nth_element(beg, mid, end, cmp);

    if (len == 0)
    {
        return nullptr;
    }

    Node<ElemType, Dim> *node = new Node<ElemType, Dim>(mid->data(), axis, level);

    if (len == 1)
    {
        node->isLeaf = true;
        return node;
    }

    if (len == 2)
    {
        node->left = new Node<ElemType, Dim>(beg->data(), axis + 1, level + 1);
        node->left->isLeaf = true;
        return node;
    }

    int new_axis = (axis + 1) % Dim;
    node->left = createNode(beg, mid, new_axis, level + 1);
    node->right = createNode(mid + 1, end, new_axis, level + 1);

    return node;
};

template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::radiusSearchRecurse(const Node<ElemType, Dim> *curNode, const Point &query, const double dist_sq_limit, std::vector<Point> &points, std::vector<double> &dist_sq_serched)
{

    int ret = 0;
    int added_recs = 0;

    if (nullptr == curNode)
    {
        return 0;
    }

    auto dist2 = this->distance_sq(query, curNode->data);

    if (dist2 <= dist_sq_limit)
    {
        points.emplace_back(curNode->data);
        dist_sq_serched.emplace_back(dist2);
        added_recs += 1;
    }

    int axis = curNode->axis;
    double dx = query[axis] - curNode->data[axis];

    ret = radiusSearchRecurse((dx <= 0.0 ? (curNode->left) : (curNode->right)), query, dist_sq_limit, points, dist_sq_serched);
    added_recs += ret;
    if (ret >= 0 && std::abs(dx * dx) < dist_sq_limit)
    {

        ret = radiusSearchRecurse((dx <= 0.0 ? (curNode->right) : (curNode->left)), query, dist_sq_limit, points, dist_sq_serched);
        added_recs += ret;
    }

    added_recs += ret;

    return added_recs;
};

/*
template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::iterNode(const Node<ElemType, Dim> *node, std::priority_queue<Point> &points)
{
    if (node == nullptr)
    {
        return false;
    }

    points.push(std::move(t));

    iterNode(node->left, points);
    iterNode(node->right, points);

    return true;
}
*/
#pragma endregion
