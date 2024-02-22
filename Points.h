#pragma once


namespace can2
{
	class Point2
	{
	public:
		Point2() : x(0), y(0) {
		}
		Point2(const Point2& a) {
			*this = a;
		}

	public:
		double x;
		double y;
		
	// Operations:
	public:
		Point2& operator=(const Point2& a) {
			x = a.x;
			y = a.y;
			return *this;
		}


	};
}