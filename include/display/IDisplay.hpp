#pragma once
#include "common/types.hpp"

class IDisplay {
public:
	virtual ~IDisplay() = default;
	virtual void render(const CollectorData& data, const AnalysisResult& result) = 0;
};