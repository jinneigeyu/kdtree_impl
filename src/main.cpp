#include <iostream>
#include "kdtree.hpp"
#include <random>

constexpr double MIN = 10;
constexpr double MAX = 100;
constexpr double RAND_NUMS_TO_GENERATE = 2;

template <typename T, std::size_t Dim>
int createTree(const std::vector<typename KDtree<T, Dim>::Point> &points, KDtree<T, Dim> &tree)
{
    return tree.BuildTree(&points[0][0], points.size());
}

template <typename T, std::size_t Dim>
int testKdTree(KDtree<T, Dim> &tree, typename KDtree<T, Dim>::Point &query, double radius)
{
    std::vector<double> dist2Searched;
    std::vector<typename KDtree<T, Dim>::Point> searchedPoints;
    typename KDtree<T, Dim>::Point q = query;
    tree.SearchRadius(q, radius, true, searchedPoints, dist2Searched);
    return searchedPoints.size();
};

template <typename T, std::size_t Dim>
int volite_method(const typename KDtree<T, Dim>::Point &query, const std::vector<typename KDtree<T, Dim>::Point> &points, double radius)
{

    std::vector<double> dist2Searched_volit;
    std::vector<typename KDtree<T, Dim>::Point> searchedPoints_volit;
    auto radius2 = radius * radius;

    for (int i = 0; i < points.size(); i++)
    {
        double distance2 = KDtree<T, 3>::distance_sq(query, points[i]);
        if (distance2 <= radius2)
        {
            dist2Searched_volit.push_back(distance2);
            searchedPoints_volit.push_back(points[i]);
        }
    }
    return dist2Searched_volit.size();
};

int main()
{
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(MIN, MAX);
    int N = 640 * 480 / 2;
    double radius = 20;
    std::vector<KDtree<float, 3>::Point> points(N);
    for (int i = 0; i < points.size(); i++)
    {
        float x = distr(eng);
        float y = distr(eng);
        float z = distr(eng);
        points[i][0] = x;
        points[i][1] = y;
        points[i][2] = x;
    }

    KDtree<float, 3> tree;
    createTree(points, tree);

    clock_t elapse_0 = 0;
    clock_t elapse_1 = 0;

    std::uniform_int_distribution<int> distrIndex(0, points.size());
    for (int i = 0; i < 100; i++)
    {
        int index = distrIndex(eng);
        auto q = points[index];
        auto t = clock();
        int result_counts = testKdTree(tree, q, radius);
        auto es_0 = clock() - t;
        elapse_0 += es_0;

        t = clock();
        int result_voliate_counts = volite_method<float, 3>(q, points, radius);
        auto es_1 = clock() - t;
        elapse_1 += es_1;
        std::cout << "kdtree takes : " << es_0 << " || voliete takes : " << es_1 << " | " << result_counts - result_voliate_counts << std::endl;
    }

    std::cout << "total diff : " << elapse_0 - elapse_1 << std::endl;

    return (0);
}