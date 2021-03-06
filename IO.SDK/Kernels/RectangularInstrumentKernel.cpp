/**
 * @file RectangularInstrumentKernel.cpp
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "RectangularInstrumentKernel.h"
#include<filesystem>
#include<fstream>

void IO::SDK::Kernels::RectangularInstrumentKernel::BuildKernel()
{
	if (std::filesystem::exists(m_filePath))
	{
		unload_c(m_filePath.c_str());
		std::filesystem::remove(m_filePath);
	}

	std::ofstream outFile(m_filePath);
	std::ifstream readFile(m_templatePath);
	std::string readout;
	std::string search;
	std::string replace;

	if (readFile.good() && outFile.good())
	{
		while (std::getline(readFile, readout))
		{
			auto posinstid = readout.find("{instrumentid}");
			if (posinstid != std::string::npos)
			{
				readout = readout.replace(posinstid, 14, std::to_string(m_instrument.GetId()));
			}

			auto posframename = readout.find("{framename}");
			if (posframename != std::string::npos)
			{
				readout = readout.replace(posframename, 11, m_instrument.GetFrame()->GetName());
			}

			auto posspid = readout.find("{spacecraftid}");
			if (posspid != std::string::npos)
			{
				readout = readout.replace(posspid, 14, std::to_string(m_instrument.GetSpacecraft().GetId()));
			}

			auto posbx = readout.find("{bx}");
			if (posbx != std::string::npos)
			{
				readout = readout.replace(posbx, 4, std::to_string(m_boresight.GetX()));
			}

			auto posby = readout.find("{by}");
			if (posby != std::string::npos)
			{
				readout = readout.replace(posby, 4, std::to_string(m_boresight.GetY()));
			}

			auto posbz = readout.find("{bz}");
			if (posbz != std::string::npos)
			{
				readout = readout.replace(posbz, 4, std::to_string(m_boresight.GetZ()));
			}

			auto posrx = readout.find("{rx}");
			if (posrx != std::string::npos)
			{
				readout = readout.replace(posrx, 4, std::to_string(m_refVector.GetX()));
			}

			auto posry = readout.find("{ry}");
			if (posry != std::string::npos)
			{
				readout = readout.replace(posry, 4, std::to_string(m_refVector.GetY()));
			}

			auto posrz = readout.find("{rz}");
			if (posrz != std::string::npos)
			{
				readout = readout.replace(posrz, 4, std::to_string(m_refVector.GetZ()));
			}

			auto posangle = readout.find("{angle}");
			if (posangle != std::string::npos)
			{
				readout = readout.replace(posangle, 7, std::to_string(m_angle));
			}

			auto poscrossangle = readout.find("{cangle}");
			if (poscrossangle != std::string::npos)
			{
				readout = readout.replace(poscrossangle, 8, std::to_string(m_crossAngle));
			}

			outFile << readout << std::endl;
		}
	}
}

IO::SDK::Kernels::RectangularInstrumentKernel::RectangularInstrumentKernel(const IO::SDK::Instruments::Instrument& instrument, const IO::SDK::Math::Vector3D& boresight, const IO::SDK::Math::Vector3D& refVector, const double angle, const double crossAngle)
	:InstrumentKernel(instrument, boresight, refVector, angle, "IKRectangularTemplate.ti"), m_crossAngle{ crossAngle }
{
	BuildKernel();
	furnsh_c(m_filePath.c_str());
	m_isLoaded = true;
}
