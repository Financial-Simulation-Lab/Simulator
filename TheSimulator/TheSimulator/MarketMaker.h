#pragma once

#include "Agent.h"
#include "Order.h"

struct MarketMakerOrder {
	OrderID id;
	Volume volume;

	MarketMakerOrder(OrderID id, Volume volume) : id(id), volume(volume) { }
};

class MarketMaker : public Agent {
public:
	MarketMaker(const Simulation* simulation);
	MarketMaker(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	Volume m_volumeUnit;
	Volume m_safe;
	Volume m_limit;
	double m_depthSpread;
	double m_orderMeanLife;
	double m_orderFrequency;

	bool flag = true;
	Volume position = 0;

	void scheduleNextOrderPlacement();
	Timestamp orderCancellationDelay();
};