#pragma once

#include "Agent.h"
#include "Order.h"

struct NoiseAgentOrder {
	OrderID id;
	Volume volume;

	NoiseAgentOrder(OrderID id, Volume volume) : id(id), volume(volume) { }
};

class NoiseAgent : public Agent {
public:
	NoiseAgent(const Simulation* simulation);
	NoiseAgent(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	Volume m_volumeUnit;
	double m_depthSpread;
	double m_orderTypeFraction;

	void scheduleNextOrderPlacement();
	// void scheduleNextOrderCancellation();

	std::vector<NoiseAgentOrder> m_ownedOrders;
};