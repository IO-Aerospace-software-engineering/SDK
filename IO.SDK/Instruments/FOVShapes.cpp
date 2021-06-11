#include "FOVShapes.h"
#include<map>

std::string IO::SDK::Instruments::FOVShapes::ToString(const FOVShapeEnum e) const
{
	const std::map<IO::SDK::Instruments::FOVShapeEnum, const char*> FOVStrings{
		{ FOVShapeEnum::Circular, "CIRCLE" },
		{ FOVShapeEnum::Elliptical, "ELLIPSE" },
		{ FOVShapeEnum::Rectangular, "RECTANGLE" }
	};
	auto   it = FOVStrings.find(e);
	return it == FOVStrings.end() ? std::string("Out of range") : std::string(it->second);
}