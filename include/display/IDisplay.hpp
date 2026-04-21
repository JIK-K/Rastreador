#pragma once
#include "common/types.hpp"

class IDisplay {
public:
	virtual ~IDisplay() = default;
	virtual void render(const CollectorData& data,
		const std::map<std::string, float>& procNet,
		const std::map<std::string, ProcessInfo>& procInfo,
		const AnalysisResult& result) = 0;
};