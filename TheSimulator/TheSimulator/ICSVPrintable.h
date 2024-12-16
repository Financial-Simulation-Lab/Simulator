#pragma once

#include "IPrintable.h"
#include <string>

class ICSVPrintable : public virtual IPrintable {
public:
	ICSVPrintable() = default;

	virtual std::string printCSV() const = 0;
};

