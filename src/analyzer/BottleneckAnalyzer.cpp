#include "analyzer/BottleneckAnalyzer.hpp"
#include <algorithm>

AnalysisResult BottleneckAnalyzer::analyze(
	const CollectorData& sysData,
	const std::map<std::string, float>& procNet,
	float maxNetSpeed) 
{
	AnalysisResult result;

	// 네트워크 사용률 계산
	float netUsage = (maxNetSpeed > 0.0f) ? sysData.netSpeed / maxNetSpeed : 0.0f;

	// 메모리 사용률
	MEMORYSTATUSEX memStatus;
	memStatus.dwLength = sizeof(MEMORYSTATUSEX);
	float memUsage = 0.0f;
	if (GlobalMemoryStatusEx(&memStatus)) {
		memUsage = static_cast<float>(memStatus.dwMemoryLoad) / 100.0f;
	}

	if (netUsage >= m_netThreshold) {
		transition(AnalysisResult::State::NET_BOTTLENECK);
	}
	else if (sysData.cpuUsage / 100.0f >= m_cpuThreshold) {
		transition(AnalysisResult::State::CPU_BOTTLENECK);
	}
	else if (memUsage >= m_memThreshold) {
		transition(AnalysisResult::State::MEM_BOTTLENECK);
	}
	else {
		transition(AnalysisResult::State::NORMAL);
	}

	result.state = m_state;

	switch (m_state) {
	case AnalysisResult::State::NORMAL:
		result.message = "Normal";
		break;
	case AnalysisResult::State::NET_BOTTLENECK:
		result.topProcess = findTopProcess(procNet);
		result.message = result.topProcess.empty() ? "Network bottleneck" : result.topProcess + " is hogging the network";
		break;
	case AnalysisResult::State::CPU_BOTTLENECK:
		result.message = "CPU bottleneck - check background processes";
		break;
	case AnalysisResult::State::MEM_BOTTLENECK:
		result.message = "Low memory - close other programs";
		break;
	}

	return result;
}

void BottleneckAnalyzer::transition(AnalysisResult::State newState) {
	m_state = newState;
}

std::string BottleneckAnalyzer::findTopProcess(
	const std::map<std::string, float>& procNet
) {
	if (procNet.empty()) return "";

	auto it = std::max_element(procNet.begin(), procNet.end(),
		[](const auto& a, const auto& b) {
			return a.second < b.second;
		});

	return it->first + " (" + std::to_string(static_cast<int>(it->second * 100) / 100.0f) + "MB/s)";
}