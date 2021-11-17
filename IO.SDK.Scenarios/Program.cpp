#include <memory>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>

#include <CelestialBody.h>
#include <LaunchSite.h>
#include <OrbitalParameters.h>
#include <Spacecraft.h>
#include <StateVector.h>
#include <TLE.h>
#include <Launch.h>
#include <LaunchWindow.h>
#include <Window.h>
#include <UTC.h>
#include <TDB.h>
#include <Propagator.h>
#include <VVIntegrator.h>
#include <OrbitalPlaneChangingManeuver.h>
#include <ApogeeHeightChangingManeuver.h>
#include <ApsidalAlignmentManeuver.h>
#include <PhasingManeuver.h>

using namespace std::literals::chrono_literals;

void DisplayManeuverSummary(IO::SDK::Maneuvers::ManeuverBase *maneuver, const std::string title)
{
    std::cout << "======================================== " << title << " ========================================" << std::endl;
    std::cout << "Maneuver window : " << maneuver->GetManeuverWindow()->GetStartDate().ToString() << " => " << maneuver->GetManeuverWindow()->GetEndDate().ToString() << std::endl;
    std::cout << "Thrust window : " << maneuver->GetThrustWindow()->GetStartDate().ToString() << " => " << maneuver->GetThrustWindow()->GetEndDate().ToString() << std::endl;
    std::cout << "Delta V : " << maneuver->GetDeltaV().Magnitude() << " m/s" << std::endl;
    auto v = maneuver->GetDeltaV().Normalize();
    std::cout << "Spacecraft orientation : X : " << v.GetX() << " Y : " << v.GetY() << " Z : " << v.GetZ() << " ( ICRF )" << std::endl;
    std::cout << "Fuel burned :" << maneuver->GetFuelBurned() << " kg" << std::endl;
}

void DisplayLaunchWindowsSummary(const std::vector<IO::SDK::Maneuvers::LaunchWindow> &launchWindows)
{
    for (size_t i = 0; i < launchWindows.size(); i++)
    {
        std::cout << "========================================"
                  << "Launch Window " << i << " ========================================" << std::endl;
        std::cout << "Launch epoch :" << launchWindows[i].GetWindow().GetStartDate().ToString().c_str() << std::endl;
        std::cout << "Inertial azimuth :" << launchWindows[i].GetInertialAzimuth() * IO::SDK::Constants::RAD_DEG << " °" << std::endl;
        std::cout << "Non inertial azimuth :" << launchWindows[i].GetNonInertialAzimuth() * IO::SDK::Constants::RAD_DEG << " °" << std::endl;
        std::cout << "Inertial insertion velocity :" << launchWindows[i].GetInertialInsertionVelocity() << " m/s" << std::endl;
        std::cout << "Non inertial insertion velocity :" << launchWindows[i].GetNonInertialInsertionVelocity() << " m/s" << std::endl;
    }
}

int main()
{
    /*========================== Scenario Description =====================================
    We are at Cap canaveral and we have to join another spacecraft in orbit.
    The launch must occurs by day at launch site and recovery site
    To realize this operation, we'll show you how to use IO SDK to find launch windows then maneuvers sequence to reach our objective.
    For each maneuver you will obtain the maneuver window, the thrust window, Delta V, Spacecraft or satellite orientation and mass of fuel burned
    */

    //=======================Configure universe topology======================================
    //Every celestial body will be involved during propagation and may induce perturbation
    //Bodies id are defined here https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/req/naif_ids.html#NAIF%20Object%20ID%20numbers
    auto sun = std::make_shared<IO::SDK::Body::CelestialBody>(10, "sun");
    auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth", sun);
    auto moon = std::make_shared<IO::SDK::Body::CelestialBody>(301, "moon", earth);

    //========================Compute launch parameters=======================================

    //Define launch site and recovery site
    auto launchSite = std::make_shared<IO::SDK::Sites::LaunchSite>(3, "S3", IO::SDK::Coordinates::Geodetic(-81.0 * IO::SDK::Constants::DEG_RAD, 28.5 * IO::SDK::Constants::DEG_RAD, 0.0), earth);
    auto recoverySite = std::make_shared<IO::SDK::Sites::LaunchSite>(4, "S4", IO::SDK::Coordinates::Geodetic(-80.0 * IO::SDK::Constants::DEG_RAD, 28.5 * IO::SDK::Constants::DEG_RAD, 0.0), earth);

    //Define simulation window. (Warning : Dates must be greater to 2021-01-01 to be compliant with spacecraft clock)
    IO::SDK::Time::TDB startEpoch("2021-06-02T00:00:00");
    IO::SDK::Time::TDB endEpoch("2021-06-03T00:00:00");

    //Define parking orbit
    auto parkingOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                           6700000.0,
                                                                                           0.3,
                                                                                           50.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           40.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           0.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           0.0,
                                                                                           startEpoch,
                                                                                           IO::SDK::Frames::InertialFrames::GetICRF());
    //Define orbit of the target
    auto targetOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                          6700000.0,
                                                                                          0.4,
                                                                                          50.5 * IO::SDK::Constants::DEG_RAD,
                                                                                          42.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          10.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          0.0,
                                                                                          startEpoch,
                                                                                          IO::SDK::Frames::InertialFrames::GetICRF());

    //Compute launch windows, to launch by day on launch site and recovery site when the launch site crosses the parking orbital plane
    IO::SDK::Maneuvers::Launch launch(launchSite, recoverySite, true, *parkingOrbit);
    auto launchWindows = launch.GetLaunchWindows(IO::SDK::Time::Window<IO::SDK::Time::UTC>(startEpoch.ToUTC(), endEpoch.ToUTC()));

    //Display launch window results (this is not necessary)
    DisplayLaunchWindowsSummary(launchWindows);

    //===================Compute maneuvers to reach target body================================

    //Configure spacecraft at insertion orbit
    IO::SDK::Body::Spacecraft::Spacecraft spacecraft{-1789, "MySpacecraft", 1000.0, 10000.0, "mission01", std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(*parkingOrbit)};
    spacecraft.AddFuelTank("fuelTank1", 9000.0, 9000.0);                                                          // Add fuel tank
    spacecraft.AddEngine("serialNumber1", "engine1", "fuelTank1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0); //Add engine and link with fuel tank

    //Configure propagator
    auto step{IO::SDK::Time::TimeSpan(1.0s)};

    //Add gravity to forces model
    //You can add your own force model
    std::vector<IO::SDK::Integrators::Forces::Force *> forces{};
    IO::SDK::Integrators::Forces::GravityForce gravityForce;
    forces.push_back(&gravityForce);

    //Initialize integrator
    IO::SDK::Integrators::VVIntegrator integrator(step, forces);

    //We assume the ship will be in orbit 10 minutes after launch.
    IO::SDK::Time::TDB startDatePropagator = launchWindows[0].GetWindow().GetStartDate().ToTDB().Add(IO::SDK::Time::TimeSpan(600.0s));

    //Initialize propagator
    IO::SDK::Propagators::Propagator propagator(spacecraft, integrator, IO::SDK::Time::Window(startDatePropagator, endEpoch));

    //We define which engines can be used to realize maneuvers
    auto engine1 = spacecraft.GetEngine("serialNumber1");
    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    //We configure each maneuver
    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver planeAlignment(engines, propagator, targetOrbit.get(), startDatePropagator); //The first maneuver must not start until the launch is complete
    IO::SDK::Maneuvers::ApsidalAlignmentManeuver apsidalAlignment(engines, propagator, targetOrbit.get());
    IO::SDK::Maneuvers::PhasingManeuver phasing(engines, propagator, 1, targetOrbit.get());
    IO::SDK::Maneuvers::ApogeeHeightChangingManeuver finalApogeeChanging(engines, propagator, targetOrbit->GetApogeeVector().Magnitude());

    //We order maneuvers
    planeAlignment.SetNextManeuver(apsidalAlignment).SetNextManeuver(phasing).SetNextManeuver(finalApogeeChanging);

    //We set the first maneuver in standby
    propagator.SetStandbyManeuver(&planeAlignment);

    //We execute the propagator
    propagator.Propagate();

    // spacecraft.ReloadKernel();

    //Find sun occultation
    auto occultationWindows = spacecraft.FindWindowsOnOccultationConstraint(IO::SDK::Time::Window(IO::SDK::Time::TDB("2021-06-02T17:30:00"), IO::SDK::Time::TDB("2021-06-02T23:00:00")), *sun, *earth, IO::SDK::OccultationType::Any(), IO::SDK::AberrationsEnum::LT, IO::SDK::Time::TimeSpan(240s));

    //From here Only for data vizualization
    auto epoch = finalApogeeChanging.GetManeuverWindow()->GetEndDate();

    auto ephemeris = spacecraft.ReadEphemeris(IO::SDK::Frames::InertialFrames::GetICRF(), IO::SDK::AberrationsEnum::None, epoch, *earth);
    auto e = targetOrbit->GetStateVector().GetEccentricity();
    auto ti = targetOrbit->GetInclination() * IO::SDK::Constants::RAD_DEG;
    auto si = ephemeris.GetInclination() * IO::SDK::Constants::RAD_DEG;

    auto tom = targetOrbit->GetRightAscendingNodeLongitude() * IO::SDK::Constants::RAD_DEG;
    auto som = ephemeris.GetRightAscendingNodeLongitude() * IO::SDK::Constants::RAD_DEG;

    auto tecc = targetOrbit->GetEccentricity();
    auto secc = ephemeris.GetEccentricity();

    auto tw = targetOrbit->GetStateVector(epoch).GetPeriapsisArgument() * IO::SDK::Constants::RAD_DEG;
    auto sw = ephemeris.GetPeriapsisArgument() * IO::SDK::Constants::RAD_DEG;

    auto tq = targetOrbit->GetStateVector(epoch).GetPerigeeVector().Magnitude();
    auto sq = ephemeris.GetPerigeeVector().Magnitude();

    auto tm = targetOrbit->GetStateVector(epoch).GetMeanLongitude() * IO::SDK::Constants::RAD_DEG;
    auto sm = ephemeris.GetMeanLongitude() * IO::SDK::Constants::RAD_DEG;

    auto tv = targetOrbit->GetStateVector(epoch).GetTrueLongitude() * IO::SDK::Constants::RAD_DEG;
    auto sv = ephemeris.GetTrueLongitude() * IO::SDK::Constants::RAD_DEG;

    auto tp = targetOrbit->GetStateVector(epoch).GetPeriod().GetHours().count();
    auto sp = ephemeris.GetPeriod().GetHours().count();

    auto ta = targetOrbit->GetStateVector(epoch).GetSemiMajorAxis();
    auto sa = ephemeris.GetSemiMajorAxis();

    double dp = (ephemeris.GetPosition() - targetOrbit->GetStateVector(epoch).GetPosition()).Magnitude();

    DisplayManeuverSummary(&planeAlignment, "Plane alignment");
    DisplayManeuverSummary(&apsidalAlignment, "Aspidal alignment");
    DisplayManeuverSummary(&phasing, "Phasing");
    DisplayManeuverSummary(&finalApogeeChanging, "Apogee height changing");
}