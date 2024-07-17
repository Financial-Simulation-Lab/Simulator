#include "FundamentalAgent.h"

#include "Simulation.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>

FundamentalAgent::FundamentalAgent(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_volumeUnit(1), m_orderFrequency(3) { }

FundamentalAgent::FundamentalAgent(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_volumeUnit(1), m_orderFrequency(3) { }

void FundamentalAgent::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("volumeUnit")).empty()) {
		m_volumeUnit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderFrequency")).empty()) {
		m_orderFrequency = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void FundamentalAgent::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleNextOrderPlacement();
	} else if (msg->type == "WAKEUP_FOR_PLACEMENT") {
		// queue an L1 data request
		simulation()->dispatchMessage(currentTimestamp, 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {
		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);

		// place an order based on the current L1 status
		OrderDirection direction = simulation()->fundamental_price(int(currentTimestamp)) > (double(l1ptr->bestAskPrice + l1ptr->bestBidPrice) / 2) ? OrderDirection::Buy : OrderDirection::Sell;
		auto pptr = std::make_shared<PlaceOrderMarketPayload>(direction, m_volumeUnit);
		simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_MARKET", pptr);
		
		scheduleNextOrderPlacement();
	} else {
		// no op
	}
}

void FundamentalAgent::scheduleNextOrderPlacement() {
	double rate = 1.0 / m_orderFrequency;

	// generate a random cancellation delay
	std::exponential_distribution<> exponentialDistribution(rate);
	Timestamp delay = (Timestamp)std::floor(exponentialDistribution(simulation()->randomGenerator()));

	// queue a placement
	simulation()->dispatchMessage(simulation()->currentTimestamp(), delay, name(), name(), "WAKEUP_FOR_PLACEMENT", std::make_shared<EmptyPayload>());
}
