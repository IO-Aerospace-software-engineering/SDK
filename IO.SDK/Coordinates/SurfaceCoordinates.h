/**
 * @file SurfaceCoordinates.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef SURFACE_COORDINATES_H
#define SURFACE_COORDINATES_H
#include <Vector3D.h>
#include <vector>
#include <memory>

namespace IO::SDK::Coordinates
{
	/**
	 * @brief Surface coordinates
	 * 
	 */
	class SurfaceCoordinates
	{
	private:
		std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>> m_surfacePoints;
		std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>> m_surfaceNormals;

	public:
		/**
		 * @brief Construct a new Surface Coordinates object
		 * 
		 * @param longitudeSpan 
		 * @param latitudeSpan 
		 */
		SurfaceCoordinates(const size_t longitudeSpan,const size_t latitudeSpan);

		/**
		 * @brief Construct a new Surface Coordinates object
		 * 
		 * @param surfaceCoordinates 
		 */
		SurfaceCoordinates(const SurfaceCoordinates& surfaceCoordinates);

		SurfaceCoordinates& operator=(const SurfaceCoordinates& surfaceCoordinates);


		/**
		 * @brief Get the Surface Points
		 * 
		 * @return const std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>>& 
		 */
		const std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>>& GetSurfacePoints() const
		{
			return m_surfacePoints;
		}

		/**
		 * @brief Get the Surface Normals
		 * 
		 * @return const std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>>& 
		 */
		const std::vector<std::unique_ptr<IO::SDK::Math::Vector3D>>& GetSurfaceNormals() const
		{
			return m_surfaceNormals;
		}

		/**
		 * @brief Add a surface point
		 * 
		 * @param point 
		 */
		void AddPoint(const IO::SDK::Math::Vector3D& point);

		/**
		 * @brief Add normal vector
		 * 
		 * @param normal 
		 */
		void AddNormal(const IO::SDK::Math::Vector3D& normal);
	};
}
#endif
