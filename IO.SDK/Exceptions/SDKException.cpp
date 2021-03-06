/**
 * @file SDKException.cpp
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "SDKException.h"
IO::SDK::Exception::SDKException::SDKException(const std::string &msg) : std::exception(), m_msg{msg}
{
}
const char *IO::SDK::Exception::SDKException::what() const noexcept
{
	return m_msg.c_str();
}