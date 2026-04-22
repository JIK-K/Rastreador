#pragma once
#include <string>
#include <map>
#include <vector>

struct ProcessInfo {
	float cpuUsage = 0.0f;
	float memUsage = 0.0f;
};

struct CollectorData {
	float cpuUsage = 0.0f;
	float memUsage = 0.0f;
	float netSpeed = 0.0f;

	// 프로세스별 네트워크 사용량
	std::map<std::string, float> netPerProc;

	std::map<std::string, ProcessInfo> procInfo;
};

struct AnalysisResult {
	enum class State {
		NORMAL,
		NET_BOTTLENECK,
		CPU_BOTTLENECK,
		MEM_BOTTLENECK
	};

	State state = State::NORMAL;
	std::string message;
	std::string topProcess;
};