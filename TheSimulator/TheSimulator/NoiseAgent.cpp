#include "NoiseAgent.h"

#include "Simulation.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>

NoiseAgent::NoiseAgent(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_volumeUnit(1), m_depthSpread(2), m_orderTypeFraction(0.5) { }

NoiseAgent::NoiseAgent(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_volumeUnit(1), m_depthSpread(2), m_orderTypeFraction(0.5) { }

void NoiseAgent::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("volumeUnit")).empty()) {
		m_volumeUnit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("depthSpread")).empty()) {
		m_depthSpread = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderTypeFraction")).empty()) {
		m_orderTypeFraction = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void NoiseAgent::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleNextOrderPlacement();
	} else if (msg->type == "WAKEUP_FOR_PLACEMENT") {
		// queue an L1 data request
		simulation()->dispatchMessage(simulation()->currentTimestamp(), 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {
		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);
		// place an order based on the current L1 status
		std::bernoulli_distribution orderTypeDistribution(m_orderTypeFraction);
		std::bernoulli_distribution orderDirectionDistribution(0.5);
		bool isMarketOrder = orderTypeDistribution(simulation()->randomGenerator());
		OrderDirection direction = orderDirectionDistribution(simulation()->randomGenerator()) ? OrderDirection::Buy : OrderDirection::Sell;
		if (isMarketOrder) {
			auto pptr = std::make_shared<PlaceOrderMarketPayload>(direction, m_volumeUnit);
			simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_MARKET", pptr);
		} else {
			std::uniform_real_distribution<> priceUniformDistribution(std::numeric_limits<double>::min(), m_depthSpread);
			Money adjustPrice = Money(Decimal(priceUniformDistribution(simulation()->randomGenerator())) + (l1ptr->bestAskPrice - l1ptr->bestBidPrice).abs());
			Money price;
			if (direction == OrderDirection::Buy) {
				price = l1ptr->bestAskPrice - adjustPrice.floorToCents();
			} else {
				price = l1ptr->bestBidPrice + adjustPrice.floorToCents();
			}

			auto pptr = std::make_shared<PlaceOrderLimitPayload>(direction, m_volumeUnit, price);
			simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);
		}
	} else {
		// no op
	}
}

void NoiseAgent::scheduleNextOrderPlacement() {
	// generate a random cancellation delay
	std::uniform_int_distribution<> uniformDistribution(simulation()->durationTimestamp());
	Timestamp delay = (Timestamp)std::floor(uniformDistribution(simulation()->randomGenerator()));

	// queue a placement
	simulation()->dispatchMessage(simulation()->currentTimestamp(), delay, name(), name(), "WAKEUP_FOR_PLACEMENT", std::make_shared<EmptyPayload>());
}
