#ifndef POINT2D_H
#define POINT2D_H

struct	Point2;
struct	Vector2;
class TaxiPath;

struct Point2 {
	Point2() : x_(0.0), y_(0.0), p{ 0.0, 0.0, 0.0 } { }
	Point2(double ix, double iy) : x_(ix), y_(iy), p{ ix, iy, 0.0 } { }
	Point2(const Point2& rhs) : x_(rhs.x_), y_(rhs.y_), p{ rhs.x_, rhs.y_, 0.0 } { }

	~Point2() { }

	Point2& operator=(const Point2& rhs) { x_ = rhs.x_; y_ = rhs.y_; return *this; }
	bool operator==(const Point2& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_; }
	bool operator!=(const Point2& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_; }

	Point2& operator += (const Vector2& v);
	Point2& operator -= (const Vector2& v);
	Point2 operator+(const Vector2& v) const;
	Point2 operator-(const Vector2& v) const;

	double* as_array();

	double x_, y_;
	double p[3];
	int index = -1;
	TaxiPath* parent = nullptr;
};

struct Vector2 {
	Vector2() : dx(0.0), dy(0.0) { }
	Vector2(double ix, double iy) : dx(ix), dy(iy) { }
	Vector2(const Vector2& rhs) : dx(rhs.dx), dy(rhs.dy) { }
	explicit Vector2(const Point2& rhs) : dx(rhs.x_), dy(rhs.y_) { }
	Vector2(const Point2& p1, const Point2& p2) : dx(p2.x_ - p1.x_), dy(p2.y_ - p1.y_) { }

	Vector2& operator=(const Vector2& rhs) { dx = rhs.dx; dy = rhs.dy; return *this; }
	bool operator==(const Vector2& rhs) const { return dx == rhs.dx && dy == rhs.dy; }
	bool operator!=(const Vector2& rhs) const { return dx != rhs.dx || dy != rhs.dy; }

	Vector2& operator += (const Vector2& rhs) { dx += rhs.dx; dy += rhs.dy; return *this; }
	Vector2& operator -= (const Vector2& rhs) { dx -= rhs.dx; dy -= rhs.dy; return *this; }
	Vector2& operator *= (double scalar) { dx *= scalar; dy *= scalar; return *this; }
	Vector2& operator/= (double scalar) { dx /= scalar; dy /= scalar; return *this; }
	Vector2	 operator+ (const Vector2& v) const { return Vector2(dx + v.dx, dy + v.dy); }
	Vector2	 operator- (const Vector2& v) const { return Vector2(dx - v.dx, dy - v.dy); }
	Vector2  operator* (double scalar) const { return Vector2(dx * scalar, dy * scalar); }
	Vector2  operator/ (double scalar) const { return Vector2(dx / scalar, dy / scalar); }
	Vector2	 operator- (void) const { return Vector2(-dx, -dy); }

	double dx, dy;
};

// These must be defined below because Point2 is declared before Vector2.
inline Point2& Point2::operator += (const Vector2& v) { x_ += v.dx; y_ += v.dy; return *this; }
inline Point2& Point2::operator -= (const Vector2& v) { x_ -= v.dx; y_ -= v.dy; return *this; }
inline Point2 Point2::operator+(const Vector2& v) const { return Point2(x_ + v.dx, y_ + v.dy); }
inline Point2 Point2::operator-(const Vector2& v) const { return Point2(x_ - v.dx, y_ - v.dy); }

#endif
