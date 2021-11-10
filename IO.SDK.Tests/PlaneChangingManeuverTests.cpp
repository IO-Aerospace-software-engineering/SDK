#include <gtest/gtest.h>
#include <Engine.h>
#include <Spacecraft.h>
#include <vector>
#include <CelestialBody.h>
#include <OrbitalPlaneChangingManeuver.h>
#include <TimeSpan.h>
#include <Propagator.h>
#include <VVIntegrator.h>
#include <ConicOrbitalElements.h>
#include <TestsConstants.h>
#include <UTC.h>
#include <TDB.h>

#include <chrono>
#include <memory>

using namespace std::chrono_literals;

TEST(PlaneChangingManeuver, CanExecute)
{
    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.5, 60.0 * IO::SDK::Constants::DEG_RAD, 10.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.5, 45.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));

    s.AddFuelTank("ft1", 1000.0, 900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654);
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654 + IO::SDK::Constants::PI);
    //Initialize
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector()));

    //Can't execute, too early
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN - IO::SDK::Time::TimeSpan(10s))));

    //Must execute at 125.93° == t+6600s
    ASSERT_TRUE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN + IO::SDK::Time::TimeSpan(10s))));

    //Can't execute because node is behind
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN + IO::SDK::Time::TimeSpan(30s))));

    //==============
    //!!NEXT NODE!!
    //==============

    //Can't execute, too far
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN - IO::SDK::Time::TimeSpan(10s))));

    //Must execute at 125.93°+180° == t+32959
    ASSERT_TRUE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN + IO::SDK::Time::TimeSpan(10s))));

    //Can't execute because node is behind
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN + IO::SDK::Time::TimeSpan(30s))));

    //==============
    //Test another lap to validate switching between node inbound and outbound
    //==============

    //Can't execute, too early
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN - IO::SDK::Time::TimeSpan(10s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));

    //Must execute at 125.93° == t+6600s+Orbital Period
    ASSERT_TRUE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN + IO::SDK::Time::TimeSpan(10s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));

    //Can't execute because node is behind
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN + IO::SDK::Time::TimeSpan(30s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));

    //==============
    //!!NEXT NODE!!
    //==============

    //Can't execute, too far
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN - IO::SDK::Time::TimeSpan(10s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));

    //Must execute at 125.93°+180° == t+32959+Orbital Period
    ASSERT_TRUE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN + IO::SDK::Time::TimeSpan(10s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));

    //Can't execute because node is behind
    ASSERT_FALSE(maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN + IO::SDK::Time::TimeSpan(30s) + s.GetOrbitalParametersAtEpoch()->GetPeriod())));
}

TEST(PlaneChangingManeuver, IdentifyNode)
{
    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 60.0 * IO::SDK::Constants::DEG_RAD, 10.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 45.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));

    s.AddFuelTank("ft1", 1000.0, 900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197893);
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197893 + IO::SDK::Constants::PI);

    //Initialize
    ASSERT_FALSE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN)));
    ASSERT_TRUE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN)));
}

TEST(PlaneChangingManeuver, IdentifyNode2)
{
    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 60.0 * IO::SDK::Constants::DEG_RAD, 220.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 130.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));

    s.AddFuelTank("ft1", 1000.0, 900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(5.4677);
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(5.4677 - IO::SDK::Constants::PI);

    //Initialize
    ASSERT_FALSE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN)));
    ASSERT_TRUE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN)));
}

TEST(PlaneChangingManeuver, IdentifyNode3)
{
    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 140.0 * IO::SDK::Constants::DEG_RAD, 220.0 * IO::SDK::Constants::DEG_RAD, 70.0 * IO::SDK::Constants::DEG_RAD, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.2, 130.0 * IO::SDK::Constants::DEG_RAD, 300.0 * IO::SDK::Constants::DEG_RAD, 205.0 * IO::SDK::Constants::DEG_RAD, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));

    s.AddFuelTank("ft1", 1000.0, 900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(6.24);
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(6.24 - IO::SDK::Constants::PI);

    //Initialize
    ASSERT_FALSE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN)));
    ASSERT_TRUE(maneuver.IsAscendingNode(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN)));
}

TEST(PlaneChangingManeuver, ExecuteInsuffisantDeltaV)
{

    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 60.0 * IO::SDK::Constants::DEG_RAD, 10.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 45.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));

    s.AddFuelTank("ft1", 1000.0, 900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654);
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654 + IO::SDK::Constants::PI);

    //To detect if we're passing through the node we need at least two
    //This canExecute will evaluate a first point
    maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN - IO::SDK::Time::TimeSpan(10.0s)));

    //The "TryExecute" will evaluate a second time "CanExecute", so it will be able to check the passage of the node
    auto res = maneuver.TryExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN));

    ASSERT_FALSE(res.IsValid());
    ASSERT_DOUBLE_EQ(3849.8575175874462, maneuver.GetDeltaV().Magnitude());
}

TEST(PlaneChangingManeuver, ExecuteDN)
{

    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 60.0 * IO::SDK::Constants::DEG_RAD, 10.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 45.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));
    //Add fictive data
    prop.AddStateVector(IO::SDK::OrbitalParameters::StateVector(earth, IO::SDK::Math::Vector3D(1.0, 2.0, 3.0), IO::SDK::Math::Vector3D(4.0, 5.0, 6.0), IO::SDK::Time::TDB(4260.0s), IO::SDK::Frames::InertialFrames::GetICRF()));

    s.AddFuelTank("ft1", 2000.0, 1900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654);                          //4282s
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654 + IO::SDK::Constants::PI); //10402s

    //To detect if we're passing through the node we need at least two
    //This canExecute will evaluate a first point
    maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN - IO::SDK::Time::TimeSpan(10.0s)));

    //The "TryExecute" will evaluate a second time "CanExecute", so it will be able to check the passage of the node
    auto res = maneuver.TryExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyDN));

    ASSERT_TRUE(res.IsValid());

    //Check Delta V magnitude
    ASSERT_DOUBLE_EQ(3849.8575175874462, maneuver.GetDeltaV().Magnitude());

    //Check if delta v vector points to the right direction
    auto orientation = maneuver.GetDeltaV().Normalize();
    ASSERT_NEAR(0.38598208120028127, orientation.GetX(), 1E-07);
    ASSERT_NEAR(-0.66574946629008014, orientation.GetY(), 1E-07);
    ASSERT_NEAR(0.63858866348125298, orientation.GetZ(), 1E-07);

    //Check inclination
    ASSERT_DOUBLE_EQ(0.66556812329478388, maneuver.GetRelativeInclination());

    //Check fuel burned
    ASSERT_DOUBLE_EQ(1687.9427131389323, maneuver.GetFuelBurned());

//Check maneuver window
#ifdef _WIN32
    ASSERT_EQ(IO::SDK::Time::Window<IO::SDK::Time::TDB>(IO::SDK::Time::TDB(4265.2453386213119s), IO::SDK::Time::TDB(4299.0041923612371s)), *maneuver.GetThrustWindow());

#else
    ASSERT_EQ(IO::SDK::Time::Window<IO::SDK::Time::TDB>(IO::SDK::Time::TDB(4265.245338359885s), IO::SDK::Time::TDB(4299.004192622664s)), *maneuver.GetThrustWindow());
#endif
}

TEST(PlaneChangingManeuver, ExecuteAN)
{

    const auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams1 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 60.0 * IO::SDK::Constants::DEG_RAD, 10.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    std::unique_ptr<IO::SDK::OrbitalParameters::OrbitalParameters> orbitalParams2 = std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth, 11480000.0, 0.0, 45.0 * IO::SDK::Constants::DEG_RAD, 55.0 * IO::SDK::Constants::DEG_RAD, 0.0, 0.0, IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::OrbitalParameters::StateOrientation attitude(IO::SDK::Time::TDB(0.0s), IO::SDK::Frames::InertialFrames::GetICRF());
    IO::SDK::Body::Spacecraft::Spacecraft s{-1, "sptest", 1000.0, 3000.0, "ms01", std::move(orbitalParams1)};

    IO::SDK::Integrators::VVIntegrator integrator(IO::SDK::Time::TimeSpan(1.0s));
    IO::SDK::Propagators::Propagator prop(s, integrator, IO::SDK::Time::Window(IO::SDK::Time::TDB(100.0s), IO::SDK::Time::TDB(200.0s)));
    //Add fictive data
    prop.AddStateVector(IO::SDK::OrbitalParameters::StateVector(earth, IO::SDK::Math::Vector3D(1.0, 2.0, 3.0), IO::SDK::Math::Vector3D(4.0, 5.0, 6.0), IO::SDK::Time::TDB(4260.0s), IO::SDK::Frames::InertialFrames::GetICRF()));

    s.AddFuelTank("ft1", 2000.0, 1900.0);
    s.AddEngine("sn1", "eng1", "ft1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    auto engine1 = s.GetEngine("sn1");

    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver maneuver(engines, prop, orbitalParams2.get());

    auto timeToTrueAnomalyDN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654);                          //4282s
    auto timeToTrueAnomalyAN = s.GetOrbitalParametersAtEpoch()->GetTimeToTrueAnomaly(2.197937654 + IO::SDK::Constants::PI); //10402s

    //To detect if we're passing through the node we need at least two
    //This canExecute will evaluate a first point
    maneuver.CanExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN - IO::SDK::Time::TimeSpan(10.0s)));

    //The "TryExecute" will evaluate a second time "CanExecute", so it will be able to check the passage of the node
    auto res = maneuver.TryExecute(s.GetOrbitalParametersAtEpoch()->GetStateVector(timeToTrueAnomalyAN));

    ASSERT_TRUE(res.IsValid());

    //Check Delta V magnitude
    ASSERT_DOUBLE_EQ(3849.8575175874421, maneuver.GetDeltaV().Magnitude());

    //Check if delta v vector points to the right direction
    auto orientation = maneuver.GetDeltaV().Normalize();
    ASSERT_NEAR(-0.38598208120028066, orientation.GetX(), 1E-07);
    ASSERT_NEAR(0.66574946629008003, orientation.GetY(), 1E-07);
    ASSERT_NEAR(-0.63858866348125343, orientation.GetZ(), 1E-07);

    //Check inclination
    ASSERT_DOUBLE_EQ(0.66556812329478388, maneuver.GetRelativeInclination());

#ifdef _WIN32
    ASSERT_DOUBLE_EQ(1687.9427131389309, maneuver.GetFuelBurned());
#else
    //Check fuel burned
    ASSERT_DOUBLE_EQ(1687.9427131389309, maneuver.GetFuelBurned());
#endif

    //Check maneuver window
    ASSERT_EQ(IO::SDK::Time::Window<IO::SDK::Time::TDB>(IO::SDK::Time::TDB(10385.842835991318s), IO::SDK::Time::TDB(10419.601690254096s)), *maneuver.GetThrustWindow());
}

TEST(PlaneChangingManeuver, CheckOrbitalParametersToHigherInclination)
{
    //=======================Configure universe topology======================================
    // auto sun = std::make_shared<IO::SDK::Body::CelestialBody>(10, "sun");
    auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    // auto moon = std::make_shared<IO::SDK::Body::CelestialBody>(301, "moon", earth);

    //Define parking orbit
    auto parkingOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                           6700000.0,
                                                                                           0.1,
                                                                                           40.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           20.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           10.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           10.0,
                                                                                           IO::SDK::Time::TDB("2021-06-02T00:00:00"),
                                                                                           IO::SDK::Frames::InertialFrames::GetICRF());
    //Define target orbit
    auto targetOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                          6700000.0,
                                                                                          0.1,
                                                                                          55.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          20.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          10.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          10.0,
                                                                                          IO::SDK::Time::TDB("2021-06-02T00:00:00"),
                                                                                          IO::SDK::Frames::InertialFrames::GetICRF());

    //===================Compute maneuvers to reach target body================================

    //Configure spacecraft
    IO::SDK::Body::Spacecraft::Spacecraft spacecraft{-1, "MySpacecraft", 1000.0, 3000.0, "mission01", std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(*parkingOrbit)};
    spacecraft.AddFuelTank("fuelTank1", 2000.0, 1000.0);
    spacecraft.AddEngine("serialNumber1", "engine1", "fuelTank1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    //Configure propagator
    auto step{IO::SDK::Time::TimeSpan(1.0s)};

    //Add gravity to forces model
    std::vector<IO::SDK::Integrators::Forces::Force *> forces{};
    IO::SDK::Integrators::Forces::GravityForce gravityForce;
    forces.push_back(&gravityForce);

    //Initialize integrator
    IO::SDK::Integrators::VVIntegrator integrator(step, forces);

    //Configure propagator
    IO::SDK::Time::UTC startEpoch("2021-06-02T00:00:00");
    IO::SDK::Time::UTC endEpoch("2021-06-03T00:00:00");
    IO::SDK::Propagators::Propagator propagator(spacecraft, integrator, IO::SDK::Time::Window(startEpoch.ToTDB(), endEpoch.ToTDB()));

    //Configure maneuvers

    //We define which engines can be used to realize maneuvers
    auto engine1 = spacecraft.GetEngine("serialNumber1");
    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    //We configre each maneuver
    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver planeAlignment(engines, propagator, targetOrbit.get());

    //We define the first maneuver in standby
    propagator.SetStandbyManeuver(&planeAlignment);

    propagator.Propagate();

    auto startManeuver = planeAlignment.GetThrustWindow()->GetStartDate().Add(IO::SDK::Time::TimeSpan(-60.0s));
    auto endManeuver = planeAlignment.GetThrustWindow()->GetEndDate().Add(IO::SDK::Time::TimeSpan(60.0s));

    auto ephemeris = spacecraft.ReadEphemeris(IO::SDK::Frames::InertialFrames::GetICRF(), IO::SDK::AberrationsEnum::None, endManeuver, *earth);

    auto p = ephemeris.GetPerigeeVector().Magnitude();
    auto e = ephemeris.GetEccentricity();
    auto i = ephemeris.GetInclination() * IO::SDK::Constants::RAD_DEG;
    auto o = ephemeris.GetRightAscendingNodeLongitude() * IO::SDK::Constants::RAD_DEG;
    auto w = ephemeris.GetPeriapsisArgument() * IO::SDK::Constants::RAD_DEG;
    
    ASSERT_DOUBLE_EQ(6700004.1067949599, p);

    ASSERT_DOUBLE_EQ(0.10001034044290764, e);

    ASSERT_DOUBLE_EQ(54.999995365922835, i);

    ASSERT_DOUBLE_EQ(20.015974011532073, o);

    ASSERT_DOUBLE_EQ(9.9879515017488227, w);
}

TEST(PlaneChangingManeuver, CheckOrbitalParametersToLowerInclination)
{
    //=======================Configure universe topology======================================
    // auto sun = std::make_shared<IO::SDK::Body::CelestialBody>(10, "sun");
    auto earth = std::make_shared<IO::SDK::Body::CelestialBody>(399, "earth");
    // auto moon = std::make_shared<IO::SDK::Body::CelestialBody>(301, "moon", earth);

    //Define parking orbit
    auto parkingOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                           6700000.0,
                                                                                           0.9,
                                                                                           35.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           30.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           10.0 * IO::SDK::Constants::DEG_RAD,
                                                                                           10.0,
                                                                                           IO::SDK::Time::TDB("2021-06-02T00:00:00"),
                                                                                           IO::SDK::Frames::InertialFrames::GetICRF());
    //Define target orbit
    auto targetOrbit = std::make_shared<IO::SDK::OrbitalParameters::ConicOrbitalElements>(earth,
                                                                                          6700000.0,
                                                                                          0.9,
                                                                                          40.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          15.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          10.0 * IO::SDK::Constants::DEG_RAD,
                                                                                          10.0,
                                                                                          IO::SDK::Time::TDB("2021-06-02T00:00:00"),
                                                                                          IO::SDK::Frames::InertialFrames::GetICRF());

    //===================Compute maneuvers to reach target body================================

    //Configure spacecraft
    IO::SDK::Body::Spacecraft::Spacecraft spacecraft{-1, "MySpacecraft", 1000.0, 3000.0, "mission01", std::make_unique<IO::SDK::OrbitalParameters::ConicOrbitalElements>(*parkingOrbit)};
    spacecraft.AddFuelTank("fuelTank1", 2000.0, 1000.0);
    spacecraft.AddEngine("serialNumber1", "engine1", "fuelTank1", {1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, 450.0, 50.0);

    //Configure propagator
    auto step{IO::SDK::Time::TimeSpan(1.0s)};

    //Add gravity to forces model
    std::vector<IO::SDK::Integrators::Forces::Force *> forces{};
    IO::SDK::Integrators::Forces::GravityForce gravityForce;
    forces.push_back(&gravityForce);

    //Initialize integrator
    IO::SDK::Integrators::VVIntegrator integrator(step, forces);

    //Configure propagator
    IO::SDK::Time::UTC startEpoch("2021-06-02T00:00:00");
    IO::SDK::Time::UTC endEpoch("2021-06-03T00:00:00");
    IO::SDK::Propagators::Propagator propagator(spacecraft, integrator, IO::SDK::Time::Window(startEpoch.ToTDB(), endEpoch.ToTDB()));

    //Configure maneuvers

    //We define which engines can be used to realize maneuvers
    auto engine1 = spacecraft.GetEngine("serialNumber1");
    std::vector<IO::SDK::Body::Spacecraft::Engine> engines;
    engines.push_back(*engine1);

    //We configre each maneuver
    IO::SDK::Maneuvers::OrbitalPlaneChangingManeuver planeAlignment(engines, propagator, targetOrbit.get());

    //We define the first maneuver in standby
    propagator.SetStandbyManeuver(&planeAlignment);

    propagator.Propagate();

    auto startManeuver = planeAlignment.GetThrustWindow()->GetStartDate().Add(IO::SDK::Time::TimeSpan(-60.0s));
    auto endManeuver = planeAlignment.GetThrustWindow()->GetEndDate().Add(IO::SDK::Time::TimeSpan(60.0s));

    auto ephemeris = spacecraft.ReadEphemeris(IO::SDK::Frames::InertialFrames::GetICRF(), IO::SDK::AberrationsEnum::None, endManeuver, *earth);

    auto p = ephemeris.GetPerigeeVector().Magnitude();
    auto e = ephemeris.GetEccentricity();
    auto i = ephemeris.GetInclination() * IO::SDK::Constants::RAD_DEG;
    auto o = ephemeris.GetRightAscendingNodeLongitude() * IO::SDK::Constants::RAD_DEG;
    auto w = ephemeris.GetPeriapsisArgument() * IO::SDK::Constants::RAD_DEG;
    
    ASSERT_DOUBLE_EQ(6700011.5756513746, p);

    ASSERT_DOUBLE_EQ(0.89999976627196931, e);

    ASSERT_DOUBLE_EQ(40.004520687725375, i);

    ASSERT_DOUBLE_EQ(15.004839992067712, o);

    ASSERT_DOUBLE_EQ(21.932445485694235, w);
}