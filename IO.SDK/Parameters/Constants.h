#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace IO::SDK::Constants
{
	inline constexpr double G{6.67430e-11};
	inline constexpr double ECCENTRIC_ANOMALY_ACCURACY{1e-9};
	inline constexpr double PI{3.141592653589793116};
	inline constexpr double _2PI{2 * PI};
	inline constexpr double PI2{PI / 2};
	inline constexpr double RAD_DEG{180.0 / PI};
	inline constexpr double DEG_RAD{PI / 180.0};
	inline constexpr double g0{9.80665};
	inline constexpr double OfficialTwilight{-0.8 * DEG_RAD};
	inline constexpr double CivilTwilight{-6.0 * DEG_RAD};
	inline constexpr double NauticalTwilight{-12.0 * DEG_RAD};
	inline constexpr double AstronomicalTwilight{-18.0 * DEG_RAD};
}
#endif