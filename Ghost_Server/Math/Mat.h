#pragma once

#include "Vector.h"

struct Mat2
{
	Vec2 rows[2];


	Mat2() {};
	Mat2(const Mat2& rhs);
	Mat2(const float* rhs);
	Mat2(const Vec2& row1, const Vec2& row2);
	Mat2& operator= (const Mat2& rhs);

	const Mat2& operator *=  (const float rhs);
	const Mat2& operator += (const Mat2& rhs);

	float Determinant() const { return rows[0].x * rows[1].y - rows[0].y * rows[1].x; }
};

inline Mat2::Mat2(const Mat2& rhs)
{
	rows[0] = rhs.rows[0];
	rows[1] = rhs.rows[1];
}

inline Mat2::Mat2(const float* rhs)
{
	rows[0] = rhs + 0;
	rows[1] = rhs + 2;
}

inline Mat2::Mat2(const Vec2& row1, const Vec2& row2)
{
	rows[0] = row1;
	rows[1] = row2;
}

inline Mat2& Mat2::operator= (const Mat2& rhs)
{
	rows[0] = rhs.rows[0];
	rows[1] = rhs.rows[1];

	return *this;
}

inline const Mat2& Mat2::operator *=  (const float rhs)
{
	rows[0] *= rhs;
	rows[1] *= rhs;

	return *this;
}

inline const Mat2& Mat2::operator += (const Mat2& rhs)
{
	rows[0] += rhs.rows[0];
	rows[1] += rhs.rows[1];

	return *this;
}


struct Mat3
{
	Vec3 rows[3];


	Mat3() {}
	Mat3(const Mat3& rhs);
	Mat3(const float* mat);
	Mat3(const Vec3& row0, const Vec3& row1, const Vec3& row2);
	Mat3& operator = (const Mat3& rhs);

	void Zero();
	void Identity();

	float Trace() const;
	float Determinant() const;
	Mat3 Transpose() const;
	Mat3 Inverse() const;
	Mat2 Minor(const int i, const int j) const;
	float Cofactor(const int i, const int j) const;

	Vec3 operator * (const Vec3& rhs) const;
	Mat3 operator * (const float rhs) const;
	Mat3 operator * (const Mat3& rhs) const;
	Mat3 operator + (const Mat3& rhs) const;
	const Mat3& operator *= (const float rhs);
	const Mat3& operator += (const Mat3& rhs);

	Vec3& operator[](int index) { return rows[index]; }
	const Vec3& operator[](int index) const { return rows[index]; }

};

inline Mat3::Mat3(const Mat3& rhs)
{
	rows[0] = rhs.rows[0];
	rows[1] = rhs.rows[1];
	rows[2] = rhs.rows[2];
}

inline Mat3::Mat3(const float* mat)
{
	rows[0] = mat + 0;
	rows[1] = mat + 3;
	rows[2] = mat + 6;
}

inline Mat3::Mat3(const Vec3& row0, const Vec3& row1, const Vec3& row2)
{
	rows[0] = row0;
	rows[1] = row1;
	rows[2] = row2;
}

inline Mat3& Mat3::operator = (const Mat3& rhs)
{
	rows[0] = rhs.rows[0];
	rows[1] = rhs.rows[1];
	rows[2] = rhs.rows[2];

	return *this;
}

inline const Mat3& Mat3::operator *= (const float rhs)
{
	rows[0] *= rhs;
	rows[1] *= rhs;
	rows[2] *= rhs;

	return *this;
}

inline const Mat3& Mat3::operator += (const Mat3& rhs)
{
	rows[0] += rhs.rows[0];
	rows[1] += rhs.rows[1];
	rows[2] += rhs.rows[2];

	return *this;
}

inline void Mat3::Zero()
{
	rows[0].Zero();
	rows[1].Zero();
	rows[2].Zero();
}

inline void Mat3::Identity()
{
	rows[0] = Vec3(1.0f, 0.0f, 0.0f);
	rows[1] = Vec3(0.0f, 1.0f, 0.0f);
	rows[2] = Vec3(0.0f, 0.0f, 1.0f);
}

inline float Mat3::Trace() const
{
	const float xx = rows[0][0] * rows[0][0];
	const float yy = rows[1][1] * rows[1][1];
	const float zz = rows[2][2] * rows[2][2];

	return (xx + yy + zz);
}


inline float Mat3::Determinant() const
{
	const float i = rows[0][0] * ((rows[1][1] * rows[2][2]) - (rows[1][2] * rows[2][1]));
	const float j = rows[0][1] * ((rows[1][0] * rows[2][2]) - (rows[1][2] * rows[2][0]));
	const float k = rows[0][2] * ((rows[1][0] * rows[2][1]) - (rows[1][1] * rows[2][0]));

	return (i - j + k);
}

inline Mat3 Mat3::Transpose() const
{
	Mat3 transpose;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			transpose.rows[i][j] = rows[j][i];
		}
	}

	return transpose;
}

inline Mat2 Mat3::Minor(const int i, const int j) const
{
	Mat2 minor;

	int yy = 0;
	for (int y = 0; y < 3; y++)
	{
		if (y == j)
		{
			continue;
		}

		int xx = 0;

		for (int x = 0; x < 3; x++)
		{
			if (x == i)
			{
				continue;
			}

			minor.rows[xx][yy] = rows[x][y];
			xx++;
		}

		yy++;
	}

	return minor;
}

inline float Mat3::Cofactor(const int i, const int j) const
{
	const Mat2 minor = Minor(i, j);
	const float C = float(std::pow(-1, i + 1 + j + 1)) * minor.Determinant();
	return C;
}

inline Mat3 Mat3::Inverse() const
{
	Mat3 inv;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			inv.rows[j][i] = Cofactor(i, j);
		}
	}

	float det = Determinant();
	float invDet = 1.0f / det;
	inv *= invDet;
	return inv;
}

inline Vec3 Mat3::operator * (const Vec3& rhs) const
{
	Vec3 temp;
	temp[0] = rows[0].Dot(rhs);
	temp[1] = rows[1].Dot(rhs);
	temp[2] = rows[1].Dot(rhs);

	return temp;
}

inline Mat3 Mat3::operator * (const float rhs) const
{
	Mat3 tmp;
	tmp.rows[0] = rows[0] * rhs;
	tmp.rows[1] = rows[1] * rhs;
	tmp.rows[2] = rows[2] * rhs;
	return tmp;

}


inline Mat3 Mat3::operator * (const Mat3& rhs) const
{
	Mat3 temp;
	for (int i = 0; i < 3; i++)
	{
		temp.rows[i].x = rows[i].x * rhs.rows[0].x + rows[i].y * rhs.rows[1].x + rows[i].z * rhs.rows[2].x;
		temp.rows[i].y = rows[i].x * rhs.rows[0].y + rows[i].y * rhs.rows[1].y + rows[i].z * rhs.rows[2].y;
		temp.rows[i].x = rows[i].x * rhs.rows[0].z + rows[i].y * rhs.rows[1].z + rows[i].z * rhs.rows[2].z;

	}

	return temp;
}

inline Mat3 Mat3::operator + (const Mat3& rhs) const
{
	Mat3 temp;
	for (int i = 0; i < 3; i++)
	{
		temp.rows[i] = rows[i] + rhs.rows[i];
	}
	return temp;
}



