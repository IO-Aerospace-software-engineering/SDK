#include <numeric>
#include <limits>
#include <chrono>
#include <ManeuverBase.h>

IO::SDK::Maneuvers::ManeuverBase::ManeuverBase(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, IO::SDK::Propagators::Propagator &propagator)
    : m_engines{engines}, m_spacecraft{engines[0].GetFuelTank().GetSpacecraft()}, m_propagator{propagator}
{
    for (auto &&engine : m_engines)
    {
        m_fuelTanks.insert(&engine.GetFuelTank());
    }

    //Create dynamics fuel tank for spread thrust compute
    for (auto &&engine : m_engines)
    {
        m_dynamicFuelTanks[&engine.GetFuelTank()].EquivalentFuelFlow += engine.GetFuelFlow();
    }
}

IO::SDK::Maneuvers::ManeuverBase::ManeuverBase(const std::vector<IO::SDK::Body::Spacecraft::Engine> &engines, const IO::SDK::Time::TDB &minimumEpoch, IO::SDK::Propagators::Propagator &propagator) : ManeuverBase(engines, propagator)
{
    m_minimumEpoch = std::make_unique<IO::SDK::Time::TDB>(minimumEpoch);
}

void IO::SDK::Maneuvers::ManeuverBase::Handle(const IO::SDK::Time::TDB &notBeforeEpoch)
{
    if (m_minimumEpoch)
    {
        //current minimum epich is updated only if new notBeforeEpoch is greater
        if (*m_minimumEpoch < notBeforeEpoch)
        {
            m_minimumEpoch.reset(new IO::SDK::Time::TDB(notBeforeEpoch));
        }
    }
    else
    {
        //We initialize minimum epoch
        m_minimumEpoch = std::make_unique<IO::SDK::Time::TDB>(notBeforeEpoch);
    }
    m_propagator.SetStandbyManeuver(this);
}

IO::SDK::Maneuvers::ManeuverResult IO::SDK::Maneuvers::ManeuverBase::TryExecute(const IO::SDK::OrbitalParameters::OrbitalParameters &maneuverPoint)
{
    IO::SDK::Maneuvers::ManeuverResult result;

    try
    {
        //Check if maneuver can be executed at this point
        if (!CanExecute(maneuverPoint))
        {
            result.SetInvalid("Maneuver can't be executed at this point");
            return result;
        }

        //Compute maneuver parameters
        Compute(maneuverPoint);

        //Check if maneuver parameters are compatible with vessel state
        auto validationResult = Validate();

        if (!validationResult.IsValid())
        {
            result.SetInvalid(validationResult.GetMessage());
            return result;
        }

        //Complete maneuver operations and write data to propagator
        ExecuteAt(maneuverPoint);

        if (m_nextManeuver)
        {
            //Maneuver is complete, next maneuver will be handled by propagator and can't be executed before end of this maneuver
            m_nextManeuver->Handle(m_window->GetEndDate());
        }
        else
        {
            m_propagator.SetStandbyManeuver(nullptr);
        }

        result.SetValid("Maneuver successfully executed");
    }
    catch (const IO::SDK::Exception::TooEarlyManeuverException &e)
    {
        result.SetTooEarly();
    }
    catch (const std::exception &e)
    {
        result.SetInvalid(e.what());
    }

    return result;
}

IO::SDK::Maneuvers::ManeuverResult IO::SDK::Maneuvers::ManeuverBase::Validate()
{
    //Get fuel mass available
    const double fuelMassAvailable = std::accumulate(m_fuelTanks.begin(), m_fuelTanks.end(), 0.0, [](double total, const IO::SDK::Body::Spacecraft::FuelTank *f)
                                                     { return total + f->GetQuantity(); });

    //Get delta V available
    const double deltaVAvailable{IO::SDK::Body::Spacecraft::Engine::ComputeDeltaV(GetRemainingAvgISP(), m_spacecraft.GetMass(), m_spacecraft.GetDryOperatingMass())};

    //Check if available delta V is enough to execute maneuver
    if (deltaVAvailable < m_deltaV->Magnitude())
    {
        std::string message{"No enought delta V available. Required " + std::to_string(m_deltaV->Magnitude()) + " | Available : " + std::to_string(deltaVAvailable)};
        return IO::SDK::Maneuvers::ManeuverResult(false, message);
    }
    std::string s;
    return IO::SDK::Maneuvers::ManeuverResult(true, s);
}

void IO::SDK::Maneuvers::ManeuverBase::ExecuteAt(const IO::SDK::OrbitalParameters::OrbitalParameters &maneuverPoint)
{
    //Compute fuel required
    m_fuelBurned = IO::SDK::Body::Spacecraft::Engine::ComputeDeltaM(GetRemainingAvgISP(), m_spacecraft.GetMass(), m_deltaV->Magnitude());

    //Compute Thrust spreading
    SpreadThrust();

    //Compute maneuver window
    m_window = std::make_unique<IO::SDK::Time::Window<IO::SDK::Time::TDB>>(maneuverPoint.GetEpoch() - m_thrustDuration / 2, m_thrustDuration);

    if (m_minimumEpoch && m_window->GetStartDate() < *m_minimumEpoch)
    {
        throw IO::SDK::Exception::TooEarlyManeuverException("The maneuver begins too early.");
    }

    //Find position at maneuver begin
    //Get lower value nearest maneuver begin epoch
    const IO::SDK::OrbitalParameters::OrbitalParameters *nearestLowerState = m_propagator.FindNearestLowerStateVector(m_window->GetStartDate());

    if (m_propagator.GetStateVectors().empty())
    {
        nearestLowerState = &maneuverPoint;
    }

    //propagate from nearest value up to begin epoch
    auto beginState = nearestLowerState->GetStateVector(m_window->GetStartDate());

    //Compute oriention at begining
    auto orientationBegining = ComputeOrientation(beginState);

    //write orientation
    std::vector<IO::SDK::OrbitalParameters::StateOrientation> stateOrientations;
    stateOrientations.push_back(orientationBegining);

    //Find position at maneuver end
    //Add deltaV vector to maneuver point
    IO::SDK::OrbitalParameters::StateVector newManeuverState(maneuverPoint.GetCenterOfMotion(), maneuverPoint.GetStateVector().GetPosition(), maneuverPoint.GetStateVector().GetVelocity() + *m_deltaV, maneuverPoint.GetEpoch(), maneuverPoint.GetFrame());

    //Propagate from new maneuver point up to end maneuver epoch
    auto endState = newManeuverState.GetStateVector(m_window->GetEndDate());

    //Compute oriention at end
    auto orientationEnd = ComputeOrientation(endState);
    stateOrientations.push_back(orientationEnd);

    //Write orientation
    m_propagator.AddStateOrientation(stateOrientations);

    //Write Data in propagator
    //Erase unecessary vector states
    m_propagator.EraseDataFromEpochToEnd(beginState.GetEpoch());

    //Write vector states at maneuver begin and end;
    m_propagator.AddStateVector(beginState);
    m_propagator.AddStateVector(endState);
}

bool IO::SDK::Maneuvers::ManeuverBase::IsValid()
{
    return m_isValid;
}

IO::SDK::Maneuvers::ManeuverBase &IO::SDK::Maneuvers::ManeuverBase::SetNextManeuver(IO::SDK::Maneuvers::ManeuverBase &nextManeuver)
{
    m_nextManeuver = &nextManeuver;
    return nextManeuver;
}

IO::SDK::Time::Window<IO::SDK::Time::TDB> *IO::SDK::Maneuvers::ManeuverBase::GetWindow() const
{
    return m_window.get();
}

void IO::SDK::Maneuvers::ManeuverBase::ManeuverBase::SpreadThrust()
{
    if (m_deltaV == 0)
    {
        return;
    }

    double cumulatedDeltaV{};
    double burnedFuel{};
    double currentAvgISP{};

    //Get the remaining thrust duration
    IO::SDK::Time::TimeSpan remainingThrustDuration{IO::SDK::Body::Spacecraft::Engine::ComputeDeltaT(GetRemainingAvgISP(), m_spacecraft.GetMass(), GetRemainingAvgFuelFlow(), m_deltaV->Magnitude())};

    //Compute spread thrust step by step
    //No analytical solution seems to exist
    while (true)
    {
        //Evaluate minimum thrust duration at each step
        IO::SDK::Time::TimeSpan minimumRemainingThrustDuration = GetMinimumRemainingThrustDuration();

        //If lower we compute thrust with available engines
        //else we have enough fuel to complete the maneuver
        if (minimumRemainingThrustDuration < remainingThrustDuration)
        {
            //We increment thrust duration used for final result
            m_thrustDuration = m_thrustDuration + minimumRemainingThrustDuration;

            //Get current average ISP
            currentAvgISP = GetRemainingAvgISP();

            //Get fuel burned during this step
            burnedFuel = Burn(minimumRemainingThrustDuration);

            //Get spacecraft mass after burn
            double masseAfterStepBurn = m_spacecraft.GetMass();

            //We evaluate cumulated deltaV
            cumulatedDeltaV += IO::SDK::Body::Spacecraft::Engine::ComputeDeltaV(currentAvgISP, masseAfterStepBurn + burnedFuel, masseAfterStepBurn);

            //We compute remaining thrust duration for the next step
            remainingThrustDuration = IO::SDK::Body::Spacecraft::Engine::ComputeDeltaT(GetRemainingAvgISP(), masseAfterStepBurn, GetRemainingAvgFuelFlow(), m_deltaV->Magnitude() - cumulatedDeltaV);

            continue;
        }
        break;
    }

    //Burn fuel from remaining thrust
    Burn(remainingThrustDuration);

    //Now maneuver can execute until the end with this configuration, we can simply add the remaining thrust duration
    m_thrustDuration = m_thrustDuration + remainingThrustDuration;
}

double IO::SDK::Maneuvers::ManeuverBase::GetRemainingAvgFuelFlow()
{
    double res{};

    for (auto &&engine : m_engines)
    {
        if (!engine.GetFuelTank().IsEmpty())
        {
            res += engine.GetFuelFlow();
        }
    }

    return res;
}

double IO::SDK::Maneuvers::ManeuverBase::GetRemainingAvgISP()
{
    double thrust{};
    for (const auto &engine : m_engines)
    {
        if (!engine.GetFuelTank().IsEmpty())
        {
            thrust += engine.GetThrust();
        }
    }

    return (thrust / IO::SDK::Constants::g0) / GetRemainingAvgFuelFlow();
}

IO::SDK::Time::TimeSpan IO::SDK::Maneuvers::ManeuverBase::GetMinimumRemainingThrustDuration()
{
    IO::SDK::Time::TimeSpan minValue{std::chrono::duration<double>(std::numeric_limits<double>::max())};

    for (auto &&tank : m_dynamicFuelTanks)
    {
        if (!tank.first->IsEmpty())
        {
            if (tank.second.GetRemainingT(tank.first->GetQuantity()) < minValue)
            {
                minValue = tank.second.GetRemainingT(tank.first->GetQuantity());
            }
        }
    }

    return minValue;
}

double IO::SDK::Maneuvers::ManeuverBase::Burn(const IO::SDK::Time::TimeSpan &duration)
{
    double totalFuelBurned{};
    for (auto &&engine : m_engines)
    {
        if (!engine.GetFuelTank().IsEmpty())
        {
            totalFuelBurned += const_cast<IO::SDK::Body::Spacecraft::Engine &>(engine).Burn(duration);
        }
    }

    return totalFuelBurned;
}

IO::SDK::Time::TimeSpan IO::SDK::Maneuvers::ManeuverBase::GetThrustDuration() const
{
    return m_thrustDuration;
}

IO::SDK::Math::Vector3D IO::SDK::Maneuvers::ManeuverBase::GetDeltaV() const
{
    return *m_deltaV;
}

double IO::SDK::Maneuvers::ManeuverBase::GetFuelBurned() const
{
    return m_fuelBurned;
}
