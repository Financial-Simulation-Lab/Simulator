#include "MomentumAgent.h"

#include "Simulation.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>

MomentumAgent::MomentumAgent(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_volumeUnit(1), m_depthSpread(2), m_orderTypeFraction(0.5), m_orderMeanLife(600), m_orderFrequency(3), m_momentumTerm(0.5) { }

MomentumAgent::MomentumAgent(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_volumeUnit(1), m_depthSpread(2), m_orderTypeFraction(0.5), m_orderMeanLife(600), m_orderFrequency(3), m_momentumTerm(0.5) { }

void MomentumAgent::configure(const pugi::xml_node& node, const std::string& configurationPath) {
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

	if (!(att = node.attribute("orderMeanLife")).empty()) {
		m_orderMeanLife = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderFrequency")).empty()) {
		m_orderFrequency = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("momentumTerm")).empty()) {
		m_momentumTerm = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void MomentumAgent::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleNextOrderPlacement();
	} else if (msg->type == "WAKEUP_FOR_PLACEMENT") {
		// queue an L1 data request
		simulation()->dispatchMessage(simulation()->currentTimestamp(), 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {
		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);

		if (m_prePrice == 0) {
			//no op
		}
		else {
			// place an order based on the current L1 status
			std::bernoulli_distribution orderTypeDistribution(m_orderTypeFraction);
			bool isMarketOrder = orderTypeDistribution(simulation()->randomGenerator());
			m_movingAverage = (1 - m_momentumTerm) * m_movingAverage + m_momentumTerm * (double(l1ptr->bestAskPrice + l1ptr->bestBidPrice) / 2 - m_prePrice);
			OrderDirection direction = m_movingAverage > 0 ? OrderDirection::Buy : OrderDirection::Sell;
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
		}
		m_prePrice = double(l1ptr->bestAskPrice + l1ptr->bestBidPrice) / 2;
		
		scheduleNextOrderPlacement();
	} else {
		// no op
	}
}

void MomentumAgent::scheduleNextOrderPlacement() {
	double rate = 1.0 / m_orderFrequency;

	// generate a random cancellation delay
	std::exponential_distribution<> exponentialDistribution(rate);
	Timestamp delay = (Timestamp)std::floor(exponentialDistribution(simulation()->randomGenerator()));

	// queue a placement
	simulation()->dispatchMessage(simulation()->currentTimestamp(), delay, name(), name(), "WAKEUP_FOR_PLACEMENT", std::make_shared<EmptyPayload>());
}
