/**
 * @file SpacecraftFrameFile.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef SPACECRAFT_FRAME_H
#define SPACECRAFT_FRAME_H

#include <string>
#include <Spacecraft.h>
#include <FrameFile.h>

//Forward declaration
namespace IO::SDK::Body::Spacecraft
{
	class Spacecraft;
}

namespace IO::SDK::Frames
{
	/**
	 * @brief Spacecraft frame file
	 * 
	 */
	class SpacecraftFrameFile final : public IO::SDK::Frames::FrameFile
	{
	private:
		const int m_id;
		const IO::SDK::Body::Spacecraft::Spacecraft &m_spacecraft;
		void BuildFrame();

		/**
		 * @brief Construct a new Spacecraft Frame File object
		 * 
		 * @param spacecraft 
		 */
		SpacecraftFrameFile(const IO::SDK::Body::Spacecraft::Spacecraft &spacecraft);

	public:
		friend class IO::SDK::Body::Spacecraft::Spacecraft;
	};
}
#endif
