#pragma once
#include "Agent.h"
#include "Order.h"

#include <memory>
#include <fstream>
#include "ExchangeAgentMessagePayloads.h"

class ZIAgent : public Agent {
public:
	ZIAgent(const Simulation* simulation);
	ZIAgent(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	double m_halfSpread;
	Volume m_volumeUnit;
	double m_marketOrderFraction;
	double m_orderDirection;
	Timestamp m_timeStep;
	Timestamp m_orderMeanLifeTime;

	Timestamp computeOrderCancellationDelay();
	void scheduleMarketMaking();
};
