#include <math.h>
#include <cmath>

#include "Vec2.h"

Vec2::Vec2()
{

}

Vec2::Vec2(float xin, float yin)
	: x(xin), y(yin)
{

}

bool Vec2::operator == (const Vec2& rhs) const
{
	// 0=False, 1=True
	return (x == rhs.x && y == rhs.y);
}

bool Vec2::operator != (const Vec2& rhs) const
{
	// 0=False, 1=True
	return (x != rhs.x && y != rhs.y);
}

Vec2 Vec2::operator + (const Vec2& rhs) const
{
	return Vec2(x+rhs.x, y+rhs.y);
}

Vec2 Vec2::operator - (const Vec2& rhs) const
{
	return Vec2(x-rhs.x, y-rhs.y);
}

Vec2 Vec2::operator / (const float val) const
{
	return Vec2(x/val, y/val);
}

Vec2 Vec2::operator * (const float val) const
{
	return Vec2(x*val, y*val);
}

void Vec2::operator += (const Vec2& rhs)
{
	x += rhs.x;
	y += rhs.y;
}

void Vec2::operator -= (const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vec2::operator /= (const float val)
{
	x /= val;
	y /= val;
}

void Vec2::operator *= (const float val)
{
	x *= val;
	y *= val;
}

float Vec2::dist(const Vec2& rhs) const
{
	// Calculate the difference between the x and y coordinates
	float dx = rhs.x - x;
	float dy = rhs.y - y;

	// Calculate the square of the difference
	float dxSquared = dx * dx;
	float dySquared = dy * dy;

	// Calculate the sum of the squared differences
	float sumSquared = dxSquared + dySquared;

	// Calculate the square root of the sum to get the distance
	float distance = std::sqrt(sumSquared);

	return distance;
}

Vec2 Vec2::normalize(Vec2 vector)
{
	double L{ std::sqrt(vector.x * vector.x + vector.y * vector.y) };
	return Vec2(vector.x / L, vector.x / L);
}

void Vec2::normalize()
{
	double L{ std::sqrt(x * x + y * y) };
	x = x / L;
	y = y / L;
}