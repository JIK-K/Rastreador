#pragma once
#include "common/types.hpp"

class IDisplay {
public:
	virtual ~IDIsplay() = defalut;
	virtual void render(const CollectorData& data, const AnalysisResult& result) = 0;
};