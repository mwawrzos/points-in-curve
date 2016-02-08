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
	template<typename T, typename Ref>
	struct Point
	{
		T x, y;
		Ref *ref;

		template <typename Ref2>
		bool operator<(const Point<T, Ref2> &rhs) const
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
		template<typename Ref>
		Bounds(const Point<T, Ref> &p1, const Point<T, Ref> p2)
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

		template<typename Ref>
		bool matches(const Point<T, Ref> &p) const
		{
			return multiplier*(p.x*a + p.y*b + c) >= 0;
		}

		friend std::ostream & operator<<(std::ostream &out, const Bounds<T> &b);
	};

	template<typename T, typename Ref>
	class BoundsCollection
	{
		std::vector<Bounds<T>> bounds;
		const std::vector<Point<T, Ref>> &curve;
	public:
		BoundsCollection(const std::vector<Point<T, Ref>> &curve)
			: curve(curve)
		{}

		void update(const Point<T, Ref> &p)
		{
			updateNeighbour(p, prev(p));
			updateNeighbour(p, next(p));
		}
		template<typename Ref2>
		bool matches(const Point<T, Ref2> &p)
		{
			return std::all_of(bounds.begin(), bounds.end(), [&](const Bounds<T> &b)
			{
				return b.matches(p);
			});
		}

		inline const Point<T, Ref> &prev(const Point<T, Ref> &p) const
		{
			return &p == &curve.front() ? curve.back() : (&p)[-1];
		}
		inline const Point<T, Ref> &next(const Point<T, Ref> &p) const
		{
			return &p == &curve.back() ? curve.front() : (&p)[1];
		}
	private:
		void updateNeighbour(const Point<T, Ref> &p, const Point<T, Ref> &neighbour)
		{
			if (p < neighbour)
				bounds.push_back({ p, neighbour });
			else
			{
				const auto found = std::find(bounds.begin(), bounds.end(), Bounds<T>{ neighbour, p });
				bounds.erase(found);
			}
		}
	};

	template<typename T, typename Ref1, typename Ref2>
	std::vector<Point<T, Ref1>> pointsInPolygon(std::vector<Point<T, Ref1>> &points, const std::vector<Point<T, Ref2>> &curve)
	{
		if (points.size() == 0 || curve.size() == 0)
			return{};

		std::sort(points.begin(), points.end());
		std::vector<const Point<T, Ref2> *> curveSorted(curve.size());
		std::iota(curveSorted.begin(), curveSorted.end(), &curve.front());
		std::sort(curveSorted.begin(), curveSorted.end(), [&](const Point<T, Ref2> *a, const Point<T, Ref2> *b)
		{
			return *a < *b;
		});

		std::vector<Point<T, Ref1>> res;
		BoundsCollection<T, Ref2> bc(curve);
		auto points_it = std::lower_bound(points.begin(), points.end(), *curveSorted.front());
		for (auto curve_it = curveSorted.begin(); curve_it != curveSorted.end(); ++curve_it)
		{
			bc.update(**curve_it);
			for (; points_it != points.end() && *curve_it != curveSorted.back() && *points_it < *curve_it[1]; ++points_it)
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