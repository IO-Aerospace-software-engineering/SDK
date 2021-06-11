/**
 * @file Vector3D.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-04-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef VECTOR3D_H
#define VECTOR3D_H

namespace IO::SDK::Math
{
	class Quaternion;

	class Vector3D
	{
	private:
		double const _x{}, _y{}, _z{};

	public:
		static const Vector3D VectorX;
		static const Vector3D VectorY;
		static const Vector3D VectorZ;
		Vector3D(){};
		/// <summary>
		/// Instanciate a 3D vector
		/// </summary>
		/// <param name="x"></param>
		/// <param name="y"></param>
		/// <param name="z"></param>
		Vector3D(double x, double y, double z) : _x{x}, _y{y}, _z{z} {};

		Vector3D(const Vector3D &vector) : _x{vector._x}, _y{vector._y}, _z{vector._z} {};

		~Vector3D() = default;

		/// <summary>
		/// Get X
		/// </summary>
		/// <returns></returns>
		double GetX() const { return this->_x; }

		/// <summary>
		/// Get Y
		/// </summary>
		/// <returns></returns>
		double GetY() const { return this->_y; }

		/// <summary>
		/// Get Z
		/// </summary>
		/// <returns></returns>
		double GetZ() const { return this->_z; }

		/// <summary>
		/// Get the vector magnitude
		/// </summary>
		/// <returns></returns>
		double Magnitude() const;

		Vector3D const operator+(const Vector3D &vector) const;
		Vector3D const operator-(const Vector3D &vector) const;
		Vector3D const operator*(const double value) const;
		Vector3D const operator/(const double value) const;
		Vector3D &operator=(const Vector3D &vector);

		/// <summary>
		/// Get the cross product from another vector
		/// </summary>
		/// <param name="vector"></param>
		/// <returns></returns>
		Vector3D CrossProduct(const Vector3D &vector) const;

		/// <summary>
		/// Get the dot product
		/// </summary>
		/// <param name="vector"></param>
		/// <returns></returns>
		double DotProduct(const Vector3D &vector) const;

		/// <summary>
		/// Get this normalized vector
		/// </summary>
		/// <returns></returns>
		Vector3D Normalize() const;

		/// <summary>
		/// Get angle from another vector
		/// </summary>
		/// <param name="vector"></param>
		/// <returns></returns>
		double GetAngle(const Vector3D &vector) const;

		/**
		 * @brief Equal
		 * 
		 * @param vector 
		 * @return true 
		 * @return false 
		 */
		bool operator==(const Vector3D &vector) const;

		/**
		 * @brief Rotate vector by quaternion
		 * 
		 * @param quaternion 
		 * @return Vector3D 
		 */
		Vector3D Rotate(const IO::SDK::Math::Quaternion &quaternion) const;

		/**
		 * @brief Return the quaternion to rotate vector to another
		 * 
		 * @param vector 
		 * @return Quaternion 
		 */
		Quaternion To(const Vector3D &vector) const;

		/**
		 * @brief Reverse vector
		 * 
		 * @return Vector3D 
		 */
		Vector3D Reverse() const;
	};
}
#endif // !VECTOR3D_H
