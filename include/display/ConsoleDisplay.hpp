#pragma once
#include "display/ConsoleDisplay.hpp"
#include "IDisplay.hpp"
#include "common/types.hpp"
#include <map>
#include <string>

class ConsoleDisplay : public IDisplay {
public:
	ConsoleDisplay();

	void render(const CollectorData& sysData,
		const std::map<std::string, float>& procNet,
		const AnalysisResult& result);
	void clear();

private:
	void moveCursorToTop();
	bool m_firstRender = true;
};