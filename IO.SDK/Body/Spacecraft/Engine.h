/**
 * @file Engine.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-04
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef ENGINE_H
#define ENGINE_H

#include<chrono>

#include <string>
#include <Vector3D.h>
#include <FuelTank.h>
#include <TimeSpan.h>

namespace IO::SDK::Body::Spacecraft
{
	class Spacecraft;
	/**
	 * @brief Engine class
	 * 
	 */
	class Engine final
	{
	private:
		const std::string m_name{};
		const IO::SDK::Body::Spacecraft::FuelTank &m_fuelTank;
		const IO::SDK::Math::Vector3D m_position;
		const IO::SDK::Math::Vector3D m_orientation;
		const double m_isp{};
		const double m_fuelFlow{};
		const std::string m_serialNumber{};
		const double m_thrust{};
		

	public:
		/**
		 * @brief Construct a new Engine object
		 * 
		 * @param serialNumber 
		 * @param name 
		 * @param fueltank Associated fuel tank
		 * @param position 
		 * @param orientation 
		 * @param isp s
		 * @param fuelFlow kg/s
		 */
		Engine(const std::string &serialNumber, const std::string &name, const IO::SDK::Body::Spacecraft::FuelTank &fueltank, const Math::Vector3D &position, const Math::Vector3D &orientation, const double isp, const double fuelFlow);
		/**
		 * @brief Get the Name object
		 * 
		 * @return std::string 
		 */
		std::string GetName() const;

		/**
		 * @brief Get the Serial Number object
		 * 
		 * @return std::string 
		 */
		std::string GetSerialNumber() const;

		/**
		 * @brief Get the Position object
		 * 
		 * @return Math::Vector3D 
		 */
		const Math::Vector3D &GetPosition() const;

		/**
		 * @brief Get the Orientation object
		 * 
		 * @return Math::Vector3D 
		 */
		const Math::Vector3D &GetOrientation() const;

		/**
		 * @brief Get then engine specific impulse
		 * 
		 * @return double 
		 */
		double GetISP() const;

		/**
		 * @brief Get the Fuel Flow object
		 * 
		 * @return double 
		 */
		double GetFuelFlow() const;

		/**
		 * @brief Get the Remaining Delta V object
		 * 
		 * @return double 
		 */
		double GetRemainingDeltaV() const;

		/**
		 * @brief Get the Fuel Tank object
		 * 
		 * @return const IO::SDK::Body::Spacecraft::FuelTank& 
		 */
		const IO::SDK::Body::Spacecraft::FuelTank &GetFuelTank() const;

		/**
		 * @brief Get the Thrust
		 * 
		 * @return double 
		 */
		double GetThrust() const;

		/**
		 * @brief Ignite engine and get burned fuel
		 * 
		 * @param duration 
		 * @return double 
		 */
		double Burn(const IO::SDK::Time::TimeSpan& duration);

		bool operator==(const IO::SDK::Body::Spacecraft::Engine &other) const;
		bool operator!=(const IO::SDK::Body::Spacecraft::Engine &other) const;

		/**
		 * @brief Comptue Delta V from mass changing
		 * 
		 * @param isp Specific impulse
		 * @param initialMass Spacecraft initial mass
		 * @param finalMass Spacecraft final mass
		 * @return double Delta V
		 */
		static double ComputeDeltaV(double isp, double initialMass, double finalMass);

		/**
		 * @brief Compute time required to reach Delta V
		 * 
		 * @param isp Specific impulse
		 * @param initialMass Spacecraft initial mass
		 * @param fuelFlow Engine fuel flow
		 * @param deltaV Delat V
		 * @return IO::SDK::Time::TimeSpan 
		 */
		static IO::SDK::Time::TimeSpan ComputeDeltaT(double isp, double initialMass, double fuelFlow, double deltaV);

		/**
		 * @brief Compute fuel mass required to reach Delta V
		 * 
		 * @param isp Specific impulse
		 * @param initialMass Spacecraft initial mass
		 * @param deltaV Delta V
		 * @return double 
		 */
		static double ComputeDeltaM(double isp, double initialMass, double deltaV);
	};
}
#endif // !ENGINE_H
