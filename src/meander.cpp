#include "meander.h"

#include <map>
#include <functional>

namespace {

// Рекурсивно строит все непересекающиеся совершенные паросочетания
// на упорядоченном наборе точек. Условие непересечения: первая точка
// соединяется с точкой на чётной позиции, разбивая набор на «внутренний»
// (под дугой) и «внешний» сегменты, каждый из которых сочетается отдельно.
void genMatchings(const std::vector<int>& points, std::vector<Matching>& out) {
    if (points.empty()) {
        out.push_back({});
        return;
    }
    int first = points[0];
    for (std::size_t i = 1; i < points.size(); i += 2) {
        int partner = points[i];

        std::vector<int> inner(points.begin() + 1, points.begin() + i);
        std::vector<int> outer(points.begin() + i + 1, points.end());

        std::vector<Matching> innerM, outerM;
        genMatchings(inner, innerM);
        genMatchings(outer, outerM);

        for (const auto& im : innerM) {
            for (const auto& om : outerM) {
                Matching m;
                m.reserve(1 + im.size() + om.size());
                m.emplace_back(first, partner);
                m.insert(m.end(), im.begin(), im.end());
                m.insert(m.end(), om.begin(), om.end());
                out.push_back(std::move(m));
            }
        }
    }
}

// Каноническое нижнее паросочетание «радуга»: (1,2n),(2,2n-1),…,(n,n+1).
Matching rainbow(int n) {
    Matching m;
    m.reserve(n);
    for (int i = 1; i <= n; ++i)
        m.emplace_back(i, 2 * n + 1 - i);
    return m;
}

// Паросочетание → массив-инволюция (arr[a]==b и arr[b]==a) для O(1) доступа.
std::vector<int> toArray(const Matching& matching, int n) {
    std::vector<int> arr(2 * n + 1, 0);
    for (const auto& [a, b] : matching) {
        arr[a] = b;
        arr[b] = a;
    }
    return arr;
}

// Проверяет, образуют ли инволюции U (верх) и L (низ) единственный
// замкнутый цикл длины 2n — то есть настоящий меандр, а не несколько колец.
bool isMeander(const std::vector<int>& U, const std::vector<int>& L, int n) {
    int current = 1;
    for (int step = 0; step < 2 * n; ++step) {
        current = (step % 2 == 0) ? U[current] : L[current];
        if (step < 2 * n - 1 && current == 1)
            return false;  // вернулись в старт раньше времени → несколько циклов
    }
    return current == 1;
}

// Последовательность обхода: чередуя верхние и нижние дуги от точки 1.
std::vector<int> buildTraversal(const std::vector<int>& U,
                                const std::vector<int>& L, int n) {
    std::vector<int> path;
    path.reserve(2 * n);
    path.push_back(1);
    int current = 1;
    for (int step = 0; step < 2 * n; ++step) {
        current = (step % 2 == 0) ? U[current] : L[current];
        if (current != 1)
            path.push_back(current);
    }
    return path;
}

} // namespace

std::vector<Meander> generateMeanders(int n) {
    std::vector<Meander> result;
    if (n < 1 || n > 12)
        return result;

    std::vector<int> points;
    points.reserve(2 * n);
    for (int i = 1; i <= 2 * n; ++i)
        points.push_back(i);

    std::vector<Matching> uppers;
    genMatchings(points, uppers);

    Matching lower = rainbow(n);
    std::vector<int> L = toArray(lower, n);

    for (const auto& upper : uppers) {
        std::vector<int> U = toArray(upper, n);
        if (isMeander(U, L, n)) {
            Meander m;
            m.order = n;
            m.upper = upper;
            m.lower = lower;
            m.traversal = buildTraversal(U, L, n);
            result.push_back(std::move(m));
        }
    }
    return result;
}
