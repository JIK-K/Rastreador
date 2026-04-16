#pragma once
#include "common/types.hpp"

class IDataCollector {
public:
	virtual ~IDataCollector() = default;
	virtual void collect() = 0;
	virtual CollectorData getData() const = 0;
};