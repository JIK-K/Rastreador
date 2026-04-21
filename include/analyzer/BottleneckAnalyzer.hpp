#pragma once
#include "common/types.hpp"
#include <string>
#include <Windows.h>

class BottleneckAnalyzer {
public:
	BottleneckAnalyzer() = default;

	AnalysisResult analyze(const CollectorData& sysData, \
		const std::map<std::string, float>& procNet,
		float maxNetSpeed);
private:
	void transition(AnalysisResult::State newState);
	std::string findTopProcess(const std::map<std::string, float>& procNet);
private:
	AnalysisResult::State m_state = AnalysisResult::State::NORMAL;

	float m_netThreshold = 0.9f;
	float m_cpuThreshold = 0.8f;
	float m_memThreshold = 0.9f;
};