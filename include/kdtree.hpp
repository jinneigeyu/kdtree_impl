#pragma once
#include "Node.hpp"
#include <memory>
#include <vector>
#include <iostream>
#include <array>
#include <queue>
#include <algorithm>
#include <stack>

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

    int SearchNN(const Point &query, const int knn, const bool order, std::vector<std::pair<Point, double>> &points);

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
    struct Dist2Point
    {

        Dist2Point() = default;
        Dist2Point(typename KDtree<ElemType, Dim>::Point point, double distSq)
        {
            this->point = point;
            this->distSq = distSq;
        }

        typename KDtree<ElemType, Dim>::Point point;
        double distSq;

        const bool operator<(const Dist2Point &r) const
        {
            return (distSq < r.distSq);
        }
    };

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

    int knnRecures(const Node<ElemType, Dim> *node, const Point &query, const int knn, std::priority_queue<Dist2Point> &pairs);

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
int KDtree<ElemType, Dim>::SearchNN(const Point &query, const int knn, const bool order, std::vector<std::pair<Point, double>> &points)
{
    std::priority_queue<Dist2Point> pairs;
    this->knnRecures(_root, query, knn, pairs);

    points.resize(pairs.size());
    int pos = pairs.size() - 1;
    Dist2Point temp;
    while (!pairs.empty())
    {
        temp = pairs.top();
        points[pos] = (std::make_pair(temp.point, temp.distSq));
        pairs.pop();
        pos--;
    }
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
    if (ret >= 0 && std::abs(dx * dx) <= dist_sq_limit)
    {

        ret = radiusSearchRecurse((dx <= 0.0 ? (curNode->right) : (curNode->left)), query, dist_sq_limit, points, dist_sq_serched);
        added_recs += ret;
    }

    added_recs += ret;

    return added_recs;
};

template <typename ElemType, std::size_t Dim>
int KDtree<ElemType, Dim>::knnRecures(const Node<ElemType, Dim> *node, const Point &query, const int knn, std::priority_queue<Dist2Point> &pairs)
{

    Node<ElemType, Dim> *nearTree = nullptr;
    Node<ElemType, Dim> *furtherTree = nullptr;
    int axis = node->axis;
    double dx = query[axis] - node->data[axis];
    if (dx < 0)
    {
        nearTree = node->left;
        furtherTree = node->right;
    }
    else
    {
        nearTree = node->right;
        furtherTree = node->left;
    }

    double dist_sq = this->distance_sq(query, node->data);
    if (pairs.size() < knn)
    {
        pairs.emplace(Dist2Point(node->data, dist_sq));
    }
    else
    {
        auto front = pairs.top();
        if (front.distSq > dist_sq)
        {
            pairs.pop();
            pairs.emplace(Dist2Point(node->data, dist_sq));
        }
    }

    if (nullptr != nearTree)
    {
        knnRecures(nearTree, query, knn, pairs);
    }

    if (nullptr != furtherTree)
    {
        if ((dx * dx) < pairs.top().distSq)
        {
            knnRecures(furtherTree, query, knn, pairs);
        }
    }

    return true;
};

#pragma endregion
