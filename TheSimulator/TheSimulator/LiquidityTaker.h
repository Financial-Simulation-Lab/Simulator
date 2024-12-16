#pragma once
#include "Agent.h"
#include "Order.h"

#include <memory>
#include <fstream>
#include "ExchangeAgentMessagePayloads.h"

class LiquidityTaker : public Agent {
public:
	LiquidityTaker(const Simulation* simulation);
	LiquidityTaker(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	Volume m_volumeUnit;
	std::string m_impactSide;
	double m_orderDirection;
	double m_increment;
	Timestamp m_timeStep;
	double m_freq;

	// self code
	int step = 0;

	void scheduleLiquidityTaking();
};
