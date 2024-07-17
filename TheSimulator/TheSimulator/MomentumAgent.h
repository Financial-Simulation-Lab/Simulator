#pragma once

#include "Agent.h"
#include "Order.h"

struct MomentumAgentOrder {
	OrderID id;
	Volume volume;

	MomentumAgentOrder(OrderID id, Volume volume) : id(id), volume(volume) { }
};

class MomentumAgent : public Agent {
public:
	MomentumAgent(const Simulation* simulation);
	MomentumAgent(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	Volume m_volumeUnit;
	double m_depthSpread;
	double m_orderTypeFraction;
	double m_orderMeanLife;
	double m_orderFrequency;
	double m_momentumTerm;

	double m_movingAverage = 0;
	double m_prePrice = 0;

	void scheduleNextOrderPlacement();
};