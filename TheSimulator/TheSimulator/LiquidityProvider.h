#pragma once
#include "Agent.h"
#include "Order.h"

#include <memory>
#include <fstream>
#include "ExchangeAgentMessagePayloads.h"

class LiquidityProvider : public Agent {
public:
	LiquidityProvider(const Simulation* simulation);
	LiquidityProvider(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
    double m_orderDirection;
	Volume m_volumeUnit;
	double m_depth;
	Timestamp m_timeStep;
    double m_c;
	double m_freq;
	double m_delta;

	// self code
	int step = 0;

    void scheduleLiquidityProviding();
};
