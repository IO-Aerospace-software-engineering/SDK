/**
 * @file Site.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef SITE_H
#define SITE_H

#include <string>
#include <memory>
#include <vector>

#include <Geodetic.h>
#include <UTC.h>
#include <CelestialBody.h>
#include <RADec.h>
#include <Illumination.h>
#include <Aberrations.h>
#include <Window.h>
#include <SiteFrameFile.h>
#include <HorizontalCoordinates.h>
#include <Constraint.h>
#include <CoordinateSystem.h>
#include <Coordinate.h>
#include <IlluminationAngle.h>

namespace IO::SDK::Sites
{
    /**
     * @brief Site class
     * 
     */
    class Site
    {
    private:
        const int m_id;
        const std::string m_name;
        const IO::SDK::Coordinates::Geodetic m_coordinates;

        const std::shared_ptr<IO::SDK::Body::CelestialBody> m_body;
        std::unique_ptr<IO::SDK::Frames::SiteFrameFile> m_frame;

    public:
        /**
         * @brief Construct a new Site object
         * 
         * @param name 
         * @param coordinates 
         */
        Site(const int id, const std::string &name, const IO::SDK::Coordinates::Geodetic &coordinates, std::shared_ptr<IO::SDK::Body::CelestialBody> &body);

        /**
         * @brief Get the Id
         * 
         * @return int 
         */
        int GetId() const { return m_id; }

        /**
         * @brief Get the Name
         * 
         * @return std::string 
         */
        std::string GetName() const { return m_name; }

        /**
         * @brief Get the Coordinates
         * 
         * @return IO::SDK::Coordinates::Geodetic 
         */
        IO::SDK::Coordinates::Geodetic GetCoordinates() const { return m_coordinates; }

        /**
         * @brief Get the Body
         * 
         * @return std::shared_ptr<IO::SDK::Body::CelestialBody> 
         */
        std::shared_ptr<IO::SDK::Body::CelestialBody> GetBody() const { return m_body; }

        /**
         * @brief Get Right ascension, declination and range
         * 
         * @param body Target body
         * @param epoch 
         * @return IO::SDK::Coordinates::RADec 
         */
        IO::SDK::Coordinates::RADec GetRADec(const IO::SDK::Body::Body &body, const IO::SDK::AberrationsEnum aberrationCorrection, const IO::SDK::Time::TDB &epoch) const;

        /**
         * @brief Get rectangular position
         * 
         * @return IO::SDK::Math::Vector3D 
         */
        IO::SDK::OrbitalParameters::StateVector GetStateVector(const IO::SDK::Frames::Frames frame, const IO::SDK::Time::TDB &epoch) const;

        /**
         * @brief Get the Illumination
         * 
         * @param aberrationCorrection 
         * @param epoch 
         * @return IO::SDK::Illumination::Illumination 
         */
        IO::SDK::Illumination::Illumination GetIllumination(const IO::SDK::AberrationsEnum aberrationCorrection, const IO::SDK::Time::TDB &epoch) const;

        /**
         * @brief Know if it's day
         * 
         * @param epoch 
         * @return true 
         * @return false 
         */
        bool IsDay(const IO::SDK::Time::TDB &epoch, const double twilight) const;

        /**
         * @brief Know if it's night
         * 
         * @param epoch 
         * @return true 
         * @return false 
         */
        bool IsNight(const IO::SDK::Time::TDB &epoch, const double twilight) const;

        /**
         * @brief Find day windows
         * 
         * @param searchWindow 
         * @param twilight 
         * @return std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> 
         */
        std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> FindDayWindows(const IO::SDK::Time::Window<IO::SDK::Time::UTC> &searchWindow, const double twilight) const;

        /**
         * @brief Find night windows
         * 
         * @param searchWindow 
         * @param twilight 
         * @return std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> 
         */
        std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> FindNightWindows(const IO::SDK::Time::Window<IO::SDK::Time::UTC> &searchWindow, const double twilight) const;

        /**
         * @brief Find windows where illumination constraint is satisfied
         * 
         * @param searchWindow 
         * @param targetBody 
         * @param observerBody 
         * @param illuminationAgngle 
         * @param constraint 
         * @param value 
         * @return std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> 
         */
        std::vector<IO::SDK::Time::Window<IO::SDK::Time::UTC>> FindWindowsOnIlluminationConstraint(const IO::SDK::Time::Window<IO::SDK::Time::UTC> &searchWindow, const IO::SDK::Body::Body &observerBody, const IO::SDK::IlluminationAngle &illuminationAgngle, const IO::SDK::Constraint &constraint, const double value) const;

        /**
         * @brief Get the Horizontal Coordinates
         * 
         * @param body 
         * @param epoch 
         * @return IO::SDK::Coordinates::HorizontalCoordinates 
         */
        IO::SDK::Coordinates::HorizontalCoordinates GetHorizontalCoordinates(const IO::SDK::Body::Body &body, const IO::SDK::AberrationsEnum aberrationCorrection, const IO::SDK::Time::TDB &epoch) const;

        /**
         * @brief Get the State Vector to target body
         * 
         * @param body 
         * @param aberrationCorrection 
         * @param epoch 
         * @return IO::SDK::OrbitalParameters::StateVector 
         */
        IO::SDK::OrbitalParameters::StateVector GetStateVector(const IO::SDK::Body::Body &body, const IO::SDK::Frames::Frames frame, const IO::SDK::AberrationsEnum aberrationCorrection, const IO::SDK::Time::TDB &epoch) const;
    };
}

#endif