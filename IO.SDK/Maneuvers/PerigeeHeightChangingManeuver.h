/**
 * @file PerigeeHeightChangingManeuver.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef PERIGEE_HEIGHT_CHANGING_MANEUVERS_H
#define PERIGEE_HEIGHT_CHANGING_MANEUVERS_H

#include <memory>
#include <vector>

#include <ManeuverBase.h>
#include <OrbitalParameters.h>
#include <Engine.h>
#include <Spacecraft.h>
#include <Propagator.h>
#include <StateVector.h>
#include <StateOrientation.h>

namespace IO::SDK::Maneuvers
{
    class PerigeeHeightChangingManeuver final: public IO::SDK::Maneuvers::ManeuverBase
    {
    private:
        double m_targetHeight;

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
        /* data */
    public:
        /**
         * @brief Construct a new Perigee Height Changing Maneuver object
         * 
         * @param engines 
         * @param propagator 
         * @param targetHeight 
         */
        PerigeeHeightChangingManeuver(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator, const double targetHeight);

        /**
         * @brief Construct a new Perigee Height Changing Maneuver object
         * 
         * @param engines 
         * @param propagator 
         * @param targetHeight 
         * @param minimumEpoch 
         */
        PerigeeHeightChangingManeuver(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator, const double targetHeight, const IO::SDK::Time::TDB &minimumEpoch);

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