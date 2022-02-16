#include <iostream>
#include "Kdtree.hpp"
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
int test_kdTreeRadiusSearch(KDtree<T, Dim> &tree, typename KDtree<T, Dim>::Point &query, double radius)
{
    std::vector<double> dist2Searched;
    std::vector<typename KDtree<T, Dim>::Point> searchedPoints;
    typename KDtree<T, Dim>::Point q = query;
    tree.SearchRadius(q, radius, searchedPoints, dist2Searched);
    return searchedPoints.size();
};

template <typename T, std::size_t Dim>
int violent_methodRadiusSearch(const typename KDtree<T, Dim>::Point &query, const std::vector<typename KDtree<T, Dim>::Point> &points, double radius)
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

template <typename T, std::size_t Dim>
std::vector<std::pair<typename KDtree<T, Dim>::Point, double>> test_kdtreeKnn(KDtree<T, Dim> &tree, typename KDtree<T, Dim>::Point &query, const int knn)
{
    std::vector<std::pair<typename KDtree<T, Dim>::Point, double>> nnResult;
    tree.SearchNN(query, knn, nnResult);
    return nnResult;
}

template <typename T, std::size_t Dim>
std::vector<std::pair<double, typename KDtree<T, Dim>::Point>> violent_Knn(const typename KDtree<T, Dim>::Point &query, const std::vector<typename KDtree<T, Dim>::Point> &points, const int knn)
{
    typedef std::pair<double, typename KDtree<T, Dim>::Point> DistPointPairType;
    std::vector<DistPointPairType> dist_sq_vector;
    std::vector<typename KDtree<T, Dim>::Point> points_searched;

    int n_index = knn;
    if (points.size() < knn)
    {
        n_index = points.size();
    }

    for (int i = 0; i < points.size(); i++)
    {
        double distance2 = KDtree<T, 3>::distance_sq(query, points[i]);
        dist_sq_vector.push_back(std::make_pair(distance2, points[i]));
    }
    auto beg = dist_sq_vector.begin();
    auto nth = beg + n_index;
    auto end = dist_sq_vector.end();
    auto cmp = [](DistPointPairType p1, DistPointPairType p2)
    {
        return p1.first < p2.first;
    };
    std::nth_element(beg, nth, end, cmp);
    std::vector<DistPointPairType> val(beg, nth);
    std::sort(val.begin(), val.end(), [](DistPointPairType p1, DistPointPairType p2)
              { return p1.first < p2.first; });
    return val;
}

int main()
{

    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_int_distribution<int> distr(MIN, MAX);
    int N = 20 + 1;
    double radius = 50;
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
    std::cout << "test radius search : " << std::endl;
    for (int i = 0; i < 100; i++)
    {
        int index = distrIndex(eng);
        auto q = points[index];
        auto t = clock();
        int result_counts = test_kdTreeRadiusSearch(tree, q, radius);
        auto es_0 = clock() - t;
        elapse_0 += es_0;

        t = clock();
        int result_voliate_counts = violent_methodRadiusSearch<float, 3>(q, points, radius);
        auto es_1 = clock() - t;
        elapse_1 += es_1;
        std::cout << i << ". kdtree takes : " << es_0 << " ticks   || violent takes : " << es_1 << " ticks  | " << result_counts - result_voliate_counts << std::endl;
    }

    std::cout << "total diff : " << elapse_0 - elapse_1 << " ticks" << std::endl;
    std::cout << "------------------------------------------------------------------------------------------ " << std::endl;

    std::cout << "test nn search " << std::endl;
    std::uniform_int_distribution<int> distr_nn_random(10, 100);
    for (int i = 0; i < 10; i++)
    {
        int index = distrIndex(eng);
        int knn = distr_nn_random(eng);
        auto q = std::array<float, 3>({100, 100, 100});
        auto t = clock();
        auto nnResult = test_kdtreeKnn(tree, q, knn);
        auto es_0 = clock() - t;
        elapse_0 += es_0;

        t = clock();
        auto violent_nnResult = violent_Knn<float, 3>(q, points, knn);
        auto es_1 = clock() - t;
        elapse_1 += es_1;

        if (nnResult.size() != violent_nnResult.size())
        {
            std::cout << "results are not equal" << std::endl;
        }

        std::cout << i << "."
                  << " knn = " << knn << " kdtree takes : " << es_0 << " ticks   || violent takes : " << es_1 << " ticks  | " << nnResult.size() - violent_nnResult.size() << std::endl;
    }

    return (0);
}