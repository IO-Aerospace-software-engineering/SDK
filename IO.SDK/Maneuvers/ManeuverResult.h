/**
 * @file ManeuverResult.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef MANEUVER_RESULT_H
#define MANEUVER_RESULT_H

#include <string>

namespace IO::SDK::Maneuvers
{
    /**
     * @brief Result from maneuver
     * 
     */
    struct ManeuverResult
    {
    private:
        bool _isValid{false};
        std::string _message{};
        bool _tooEarly{false};
        bool _canRetryLater{false};

    public:
        /**
         * @brief Construct a new Maneuver Result object
         * 
         */
        ManeuverResult(){};

        /**
         * @brief Construct a new Maneuver Result object
         * 
         * @param isValid 
         * @param message 
         */
        ManeuverResult(bool isValid, std::string message) : _isValid{isValid}, _message{message} {};

        /**
         * @brief Set maneuver as valid
         * 
         * @param message 
         */
        void SetValid(const std::string &message)
        {
            _message = message;
            _isValid = true;
        }

        /**
         * @brief Set maneuver as invalid
         * 
         * @param message 
         */
        void SetInvalid(const std::string &message)
        {
            _message = message;
            _isValid = false;
        }

        /**
         * @brief Know if maneuver is valid
         * 
         * @return true 
         * @return false 
         */
        bool IsValid() { return _isValid; }

        /**
         * @brief The current maneuver is not valid new attempt will happen later
         * 
         * @return true 
         * @return false 
         */
        bool CanRetryLater()
        {
            return _canRetryLater;
        }

        /**
         * @brief Maneuver executed too early
         * 
         * @return true 
         * @return false 
         */
        bool IsExecutedTooEarly() { return _tooEarly; }

        void SetTooEarly()
        {
            _message = "Maneuver executed too early";
            _isValid = false;
            _canRetryLater = true;
        }

        /**
         * @brief Get the Message object
         * 
         * @return std::string 
         */
        std::string GetMessage() { return _message; }
    };

}

#endif