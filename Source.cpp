#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>

//#include "cycle.h"


struct Point
{
	int x, y;

	bool operator<(const Point &rhs) const
	{
		return x < rhs.x
			|| x == rhs.x && y < rhs.y;
	}
};

std::ostream &operator<<(std::ostream &out, const Point &p)
{
	return out << "[x:" << p.x << " y:" << p.y << "]";
}

class Bounds
{
	int a, b, c, multiplier;
public:
	Bounds(const Point &p1, const Point p2)
		: a(p1.y - p2.y)
		, b(p2.x - p1.x)
		, c(-a*p1.x - b*p1.y)
		, multiplier(p1.y < p2.y ? -1 : 1)
	{}

	bool operator==(const Bounds &rhs) const
	{
		return a == rhs.a
			&& b == rhs.b
			&& c == rhs.c;
	}

	bool matches(const Point &p) const
	{
		return multiplier*(p.x*a + p.y*b + c) >= 0;
	}

	friend std::ostream &operator<<(std::ostream &out, const Bounds &b);
};

std::ostream &operator<<(std::ostream &out, const Bounds &b)
{
	return out << "[a:" << b.a << ", b:" << b.b << ", c:" << b.c << ", m:" << b.multiplier << "]";
}

class BoundsCollection
{
	std::vector<Bounds> bounds;
	const std::vector<Point> &curve;
public:
	BoundsCollection(const std::vector<Point> &curve)
		: curve(curve)
	{}

	void update(const Point &p)
	{
		updateNeighbour(p, prev(p));
		updateNeighbour(p, next(p));
	}
	bool matches(const Point &p)
	{
		return std::all_of(bounds.begin(), bounds.end(), [&](const Bounds &b) 
		{
			return b.matches(p); 
		});
	}

	inline const Point &prev(const Point &p) const
	{
		return &p == &curve.front() ? curve.back() : (&p)[-1];
	}
	inline const Point &next(const Point &p) const
	{
		return &p == &curve.back() ? curve.front() : (&p)[1];
	}
private:
	void updateNeighbour(const Point &p, const Point &neighbour)
	{
		if (p < neighbour)
			bounds.push_back({ p, neighbour });
		else
			std::remove(bounds.begin(), bounds.end(), Bounds{ neighbour, p });
	}
};

template<typename FwdIt>
std::vector<Point> pointsInPolygon(FwdIt pBegin, FwdIt pEnd, FwdIt cBegin, FwdIt cEnd)
{
	if (pBegin == pEnd || cBegin == cEnd)
		return{};

	std::vector<Point> points(pBegin, pEnd), curve(cBegin, cEnd);

	std::sort(points.begin(), points.end());
	std::vector<Point *> curveSorted(curve.size());
	std::iota(curveSorted.begin(), curveSorted.end(), &curve.front());
	std::sort(curveSorted.begin(), curveSorted.end(), [&](const auto &a, const auto &b)
	{
		return *a < *b;
	});

	std::vector<Point> res;
	BoundsCollection bc(curve);
	auto points_it = std::lower_bound(points.begin(), points.end(), *curveSorted.front());
	for (auto curve_it = curveSorted.begin(); curve_it != curveSorted.end(); ++curve_it)
	{
		bc.update(**curve_it);
		for (; points_it != points.end() && *points_it < bc.next(**curve_it); ++points_it)
		{
			const auto &p = *points_it;
			if (bc.matches(p))
				res.push_back(p);
		}
	}

	return res;
}

int main()
{
	//std::vector<Point>
	//	points(1000000),
	//	curve(1000000);
	//	//curve = { {4, 3}, {4, 1}, {2, 7}, {5, 1} };

	//std::srand(1);
	//std::generate(points.begin(), points.end(), [] { return Point{ std::rand() & 0xF, std::rand() & 0xF }; });
	//std::generate(curve.begin(), curve.end(), [] { return Point{ std::rand()&0xF, std::rand()&0xF }; });

	std::vector<Point>
		points = { {0,3}, {0,0},{0,5},{5,5},{5,0},{3,3} },
		curve = { {0,3},{3,5},{5,3},{3,0} };

	const auto res = pointsInPolygon(points.begin(), points.end(), curve.begin(), curve.end());
	/*std::sort(points.begin(), points.end());

	std::vector<Point *> cp(curve.size());
	std::iota(cp.begin(), cp.end(), &curve.front());
	std::sort(cp.begin(), cp.end(), [&curve](const auto &a, const auto &b) { return *a < *b; });

	const auto next = [&cp](const auto &p) 
	{
		const auto &a = *p == cp.back();
		return a ?
			cp.front() : 
			p[1]; 
	};
	const auto prev = [&cp](const auto &p) 
	{
		const auto &a = *p == cp.front();
		return *p == cp.front() ? 
			cp.back() :
			p[-1];
	};
	
	std::cout << Bounds({ 2, 4 }, { 5, 13 });

	std::vector<Bounds> bounds;
	const auto update = [&](const auto &p)
	{
		const auto updateNeighbour = [&](const auto &neighbour) 
		{
			if (**p < neighbour)
				bounds.push_back({ **p, neighbour });
			else
				std::remove(bounds.begin(), bounds.end(), Bounds{ neighbour, **p });
		};

		updateNeighbour(*prev(p));
		updateNeighbour(*next(p));
	};
	
	const auto matches = [&](const Point &p)
	{
		return std::all_of(bounds.begin(), bounds.end(), [&](const Bounds &b) 
		{
			return b.multiplier*(p.x*b.a + p.y*b.b + b.c) > 0;
		});
	};

	std::vector<Point> res;
	auto pt_it = std::lower_bound(points.begin(), points.end(), *cp.front());
	for (auto p = cp.begin(); p != cp.end(); ++p)
	{
		update(p);
		for (; pt_it != points.end() && *pt_it < *next(p); ++pt_it)
			if (matches(*pt_it))
				res.push_back(*pt_it);
	}*/

	//std::for_each(cp.begin(), cp.end(), [](const auto &c) { std::cout << *c << " "; });
	//std::cout << std::endl;
	std::for_each(res.begin(), res.end(), [](const auto &c) { std::cout << c << " "; });
	return 0;
}

//using RanIt = std::vector<Point>::iterator;
//using FwdIt = RanIt;
//struct ASD
//{
//	ASD(const FwdIt &first, const FwdIt &last)
//		: curve(first, last)
//	{
//		for (auto it = first; it != last; ++it) sorted.push_back(it);
//		std::sort(sorted.begin(), sorted.end(), [](auto a, auto b) { return *a < *b; });
//	}
//
//	struct Calc
//	{
//		double a, b;
//	};
//	std::vector<Calc> calc;
//
//	std::vector<Point> curve;
//	std::vector<std::vector<Point>::iterator> sorted;
//
//	template<typename It, typename Foo>
//	std::vector<Point> asd(It begin, It end, Foo foo)
//	{
//		auto points = std::lower_bound(begin, end, *sorted.front());
//
//		return{ *sorted.front() };
//	}
//
//	void setLeftBound(const Point &p)
//	{
//
//	}
//};
//
//auto pointsInPolygon(const RanIt &first, const RanIt &last, const RanIt &polFirst, const RanIt &polLast)
//-> std::vector<std::remove_reference<decltype(*polFirst)>::type>
//{
//	if (first == last || polFirst == polLast)
//		return{};
//
//	ASD asd(polFirst, polLast);
//	std::vector<Point> points(first, last);
//
//	std::sort(points.begin(), points.end());
//
//	return asd.asd(points.begin(), points.end(), []
//	{
//	});
//	
//	auto points_it = std::lower_bound(points.begin(), points.end(), asd.curve.front());
//
//	for (auto it = asd.curve.begin(); it != asd.curve.end(); ++it)
//	{
//		asd.setLeftBound(*it);
//		for (; *points_it < *it; ++points_it)
//		{
//
//		}
//	}
//
//	return{ polFirst[0] };
//}
//
//int main()
//{
//	std::vector<Point> points{ {4, 5} }, polygon{ {2, 4}, {1, 2}, {3, 4} };
//
//	std::cout << pointsInPolygon(points.begin(), points.end(), polygon.begin(), polygon.end())[0];
//}