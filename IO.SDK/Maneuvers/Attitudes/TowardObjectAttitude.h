/**
 * @file TowardObjectAttitude.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef TOWARD_OBJECT_ATTITUDE_H
#define TOWARD_OBJECT_ATTITUDE_H
#include <vector>
#include <memory>

#include <ManeuverBase.h>
#include <OrbitalParameters.h>
#include <Engine.h>
#include <Propagator.h>
#include <Vector3D.h>
#include <Body.h>

namespace IO::SDK::Maneuvers::Attitudes
{
    class TowardObjectAttitude final : public IO::SDK::Maneuvers::ManeuverBase
    {
    private:
        const IO::SDK::Body::Body &m_targetBody;

    protected:
        /**
         * @brief Compute impulsive maneuver
         * 
         * @param maneuverPoint 
         */
        void Compute(const IO::SDK::OrbitalParameters::OrbitalParameters &maneuverPoint) override;

        /**
         * @brief Compute orientation
         * 
         * @param maneuverPoint 
         * @return IO::SDK::OrbitalParameters::StateOrientation 
         */
        IO::SDK::OrbitalParameters::StateOrientation ComputeOrientation(const IO::SDK::OrbitalParameters::OrbitalParameters &maneuverPoint) override;

    public:
        /**
         * @brief Construct a new Toward Object Attitude object
         * 
         * @param engines 
         * @param propagator 
         * @param targetBody 
         */
        TowardObjectAttitude(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator,const IO::SDK::Time::TimeSpan& attitudeHoldDuration, const IO::SDK::Body::Body &targetBody);

        /**
         * @brief Construct a new Toward Object Attitude object
         * 
         * @param engines 
         * @param propagator 
         * @param minimumEpoch 
         * @param targetBody 
         */
        TowardObjectAttitude(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator, const IO::SDK::Time::TDB &minimumEpoch,const IO::SDK::Time::TimeSpan& attitudeHoldDuration, const IO::SDK::Body::Body &targetBody);

        /**
         * @brief Evaluate if maneuver can occurs
         * 
         * @param stateVector Evaluated to check if condition is satisfied
         * @return true 
         * @return false 
         */
        bool CanExecute(const IO::SDK::OrbitalParameters::OrbitalParameters &orbitalParams) override;
    };
}

#endif