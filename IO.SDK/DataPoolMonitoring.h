/**
 * @file DataPoolMonitoring.h
 * @author Sylvain Guillet (sylvain.guillet@live.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef KERNEL_POOL_MONITORING_H
#define KERNEL_POOL_MONITORING_H

#include<vector>
#include<string>
#include<SpiceUsr.h>

namespace IO::SDK
{
	/**
	 * @brief Class used to read raw data pool
	 * 
	 */
	class DataPoolMonitoring
	{
	public:
		static DataPoolMonitoring& Instance();

		std::vector<std::string> GetStringProperty(const std::string& propertyName, const int nbValuesExpected) const;
		std::vector<int> GetIntegerProperty(const std::string& propertyName, const int nbValuesExpected)const;
		std::vector<double> GetDoubleProperty(const std::string& propertyName, const int nbValuesExpected)const;

	private:
		DataPoolMonitoring& operator= (const DataPoolMonitoring&) = delete;
		DataPoolMonitoring(const DataPoolMonitoring&) = delete;

		static DataPoolMonitoring m_instance;
		DataPoolMonitoring() = default;
		~DataPoolMonitoring() = default;
	};
}

#endif // !KERNEL_POOL_MONITORING_H


