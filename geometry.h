#pragma once

namespace pointInPolygon
{
	template<typename T>
	class Bounds;
}

template<typename T>
std::ostream &operator<<(std::ostream &out, const pointInPolygon::Bounds<T> &b);

namespace pointInPolygon
{
	template<typename T>
	struct Point
	{
		T x, y;

		bool operator<(const Point<T> &rhs) const
		{
			return x < rhs.x
				|| x == rhs.x && y < rhs.y;
		}
	};

	template<typename T>
	class Bounds
	{
		T a, b, c, multiplier;
	public:
		Bounds(const Point<T> &p1, const Point<T> p2)
			: a(p1.y - p2.y)
			, b(p2.x - p1.x)
			, c(-a*p1.x - b*p1.y)
			, multiplier(p1.y < p2.y ? -1 : 1)
		{}

		bool operator==(const Bounds<T> &rhs) const
		{
			return a == rhs.a
				&& b == rhs.b
				&& c == rhs.c;
		}

		bool matches(const Point<T> &p) const
		{
			return multiplier*(p.x*a + p.y*b + c) >= 0;
		}

		friend std::ostream & operator<<(std::ostream &out, const Bounds<T> &b);
	};

	template<typename T>
	class BoundsCollection
	{
		std::vector<Bounds<T>> bounds;
		const std::vector<Point<T>> &curve;
	public:
		BoundsCollection(const std::vector<Point<T>> &curve)
			: curve(curve)
		{}

		void update(const Point<T> &p)
		{
			updateNeighbour(p, prev(p));
			updateNeighbour(p, next(p));
		}
		bool matches(const Point<T> &p)
		{
			return std::all_of(bounds.begin(), bounds.end(), [&](const Bounds<T> &b)
			{
				return b.matches(p);
			});
		}

		inline const Point<T> &prev(const Point<T> &p) const
		{
			return &p == &curve.front() ? curve.back() : (&p)[-1];
		}
		inline const Point<T> &next(const Point<T> &p) const
		{
			return &p == &curve.back() ? curve.front() : (&p)[1];
		}
	private:
		void updateNeighbour(const Point<T> &p, const Point<T> &neighbour)
		{
			if (p < neighbour)
				bounds.push_back({ p, neighbour });
			else
				std::remove(bounds.begin(), bounds.end(), Bounds<T>{ neighbour, p });
		}
	};

	template<typename FwdIt1, typename FwdIt2>
	auto pointsInPolygon(FwdIt1 pBegin, FwdIt1 pEnd, FwdIt2 cBegin, FwdIt2 cEnd) -> std::vector<Point<decltype(pBegin->x)>>
	{
		using T = decltype(pBegin->x);

		if (pBegin == pEnd || cBegin == cEnd)
			return std::vector<Point<T>>{};

		std::vector<Point<T>> points(pBegin, pEnd), curve(cBegin, cEnd);

		std::sort(points.begin(), points.end());
		std::vector<Point<T> *> curveSorted(curve.size());
		std::iota(curveSorted.begin(), curveSorted.end(), &curve.front());
		std::sort(curveSorted.begin(), curveSorted.end(), [&](const Point<T> *a, const Point<T> *b)
		{
			return *a < *b;
		});

		std::vector<Point<T>> res;
		BoundsCollection<T> bc(curve);
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

}

template<typename T>
std::ostream &operator<<(std::ostream &out, const pointInPolygon::Bounds<T> &b)
{
	return out << "[a:" << b.a << ", b:" << b.b << ", c:" << b.c << ", m:" << b.multiplier << "]";
}