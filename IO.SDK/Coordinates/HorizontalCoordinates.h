/**
 * @file HorizontalCoordinates.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef HORIZONTAL_COORDINATES_H
#define HORIZONTAL_COORDINATES_H
namespace IO::SDK::Coordinates
{
	/**
	 * @brief Horizontal coordinates class
	 * 
	 */
	class HorizontalCoordinates
	{
	private:
		const double m_altitude, m_azimuth, m_elevation;

	public:
		/**
		 * @brief Construct a new Horizontal Coordinates object
		 * 
		 * @param azimuth 
		 * @param elevation 
		 * @param altitude 
		 */
		HorizontalCoordinates(const double azimuth, const double elevation, const double altitude) : m_altitude{altitude}, m_azimuth{azimuth}, m_elevation{elevation}
		{
		}

		HorizontalCoordinates &operator=(const HorizontalCoordinates &rhs)
		{
			if (this != &rhs)
			{
				const_cast<double &>(m_altitude) = rhs.m_altitude;
				const_cast<double &>(m_azimuth) = rhs.m_azimuth;
				const_cast<double &>(m_elevation) = rhs.m_elevation;
			}

			return *this;
		}

		HorizontalCoordinates(const HorizontalCoordinates& horizontalCoordinates) : m_altitude{horizontalCoordinates.m_altitude}, m_azimuth{horizontalCoordinates.m_azimuth}, m_elevation{horizontalCoordinates.m_elevation}
		{
		}

		/**
		 * @brief Get the Altitude
		 * 
		 * @return double 
		 */
		double GetAltitude() const { return this->m_altitude; }

		/**
		 * @brief Get the Azimuth
		 * 
		 * @return double 
		 */
		double GetAzimuth() const { return this->m_azimuth; }

		/**
		 * @brief Get the Elevation
		 * 
		 * @return double 
		 */
		double GetElevation() const { return this->m_elevation; }
	};
}
#endif // !HORIZONTAL_COORDINATES_H
