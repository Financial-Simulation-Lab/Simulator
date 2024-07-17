#pragma once

#include "Agent.h"
#include "Order.h"

struct FundamentalAgentOrder {
	OrderID id;
	Volume volume;

	FundamentalAgentOrder(OrderID id, Volume volume) : id(id), volume(volume) { }
};

class FundamentalAgent : public Agent {
public:
	FundamentalAgent(const Simulation* simulation);
	FundamentalAgent(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	Volume m_volumeUnit;
	double m_orderFrequency;

	void scheduleNextOrderPlacement();
	// void scheduleNextOrderCancellation();

	std::vector<FundamentalAgentOrder> m_ownedOrders;
};