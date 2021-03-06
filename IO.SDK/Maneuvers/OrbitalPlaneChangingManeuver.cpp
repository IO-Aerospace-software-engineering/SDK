/**
 * @file OrbitalPlaneChangingManeuver.cpp
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <OrbitalPlaneChangingManeuver.h>
#include <cmath>
#include <Constants.h>
#include <ConicOrbitalElements.h>

IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::OrbitalPlaneChangingManeuver(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator, IO::SDK::OrbitalParameters::OrbitalParameters *targetOrbit) : IO::SDK::Maneuvers::ManeuverBase(engines, propagator), m_targetOrbit{targetOrbit}
{
}

IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::OrbitalPlaneChangingManeuver(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator, IO::SDK::OrbitalParameters::OrbitalParameters *targetOrbit, const IO::SDK::Time::TDB &minimumEpoch) : IO::SDK::Maneuvers::ManeuverBase(engines, propagator, minimumEpoch), m_targetOrbit{targetOrbit}
{
}

bool IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::CanExecute(const IO::SDK::OrbitalParameters::OrbitalParameters &orbitalParams)
{
    auto spacecraftSv = orbitalParams.GetStateVector();

    if (std::abs(spacecraftSv.GetPosition().GetAngle(m_targetOrbit->GetSpecificAngularMomentum().CrossProduct(orbitalParams.GetSpecificAngularMomentum()))) < Parameters::NodeDetectionAccuraccy)
    {
        m_isAscendingNode = true;
        return true;
    }

    if (std::abs(spacecraftSv.GetPosition().GetAngle(m_targetOrbit->GetSpecificAngularMomentum().CrossProduct(orbitalParams.GetSpecificAngularMomentum()).Reverse())) < Parameters::NodeDetectionAccuraccy)
    {
        return true;
    }

    return false;
}

void IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::Compute(const IO::SDK::OrbitalParameters::OrbitalParameters &orbitalParams)
{
    auto currentvectorState = orbitalParams.GetStateVector();

    IO::SDK::Math::Vector3D vel = currentvectorState.GetVelocity();
    IO::SDK::Math::Vector3D pos = currentvectorState.GetPosition();

    //Project vector
    IO::SDK::Math::Vector3D projectedVector = vel - (pos * (vel.DotProduct(pos) / pos.DotProduct(pos)));

    //Compute relative inclination
    m_relativeInclination = std::acos(std::cos(orbitalParams.GetInclination()) * std::cos(m_targetOrbit->GetInclination()) + (std::sin(orbitalParams.GetInclination()) * std::sin(m_targetOrbit->GetInclination())) * std::cos(m_targetOrbit->GetRightAscendingNodeLongitude() - orbitalParams.GetRightAscendingNodeLongitude()));

    double rotationAngle = IO::SDK::Constants::PI2 + m_relativeInclination * 0.5;

    if (m_isAscendingNode)
    {
        rotationAngle = -rotationAngle;
    }

    //Compute deltaV
    auto deltaV = 2.0 * projectedVector.Magnitude() * std::sin(m_relativeInclination * 0.5);

    //Compute the quaternion
    auto q = IO::SDK::Math::Quaternion(pos.Normalize(), rotationAngle);

    //Rotate velocity vector
    auto rotateVecor = projectedVector.Normalize().Rotate(q);

    //Compute delta V vector
    m_deltaV = std::make_unique<IO::SDK::Math::Vector3D>(rotateVecor * deltaV);
}

IO::SDK::OrbitalParameters::StateOrientation IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::ComputeOrientation(const IO::SDK::OrbitalParameters::OrbitalParameters &maneuverPoint)
{
    IO::SDK::Math::Vector3D targetVector{maneuverPoint.GetSpecificAngularMomentum().Normalize()};
    if (m_isAscendingNode)
    {
        targetVector = targetVector.Reverse();
    }

    IO::SDK::Math::Quaternion q = m_spacecraft.Front.To(targetVector);

    return IO::SDK::OrbitalParameters::StateOrientation(q, IO::SDK::Math::Vector3D(0.0, 0.0, 0.0), maneuverPoint.GetEpoch(), maneuverPoint.GetFrame());
}

double IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver::GetRelativeInclination() const
{
    return m_relativeInclination;
}