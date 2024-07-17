#include "MarketMaker.h"

#include "Simulation.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>

MarketMaker::MarketMaker(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_volumeUnit(100), m_depthSpread(2), m_orderMeanLife(600), m_orderFrequency(3), m_safe(101), m_limit(2000) { }

MarketMaker::MarketMaker(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_volumeUnit(100), m_depthSpread(2), m_orderMeanLife(600), m_orderFrequency(3), m_safe(101), m_limit(2000) { }

void MarketMaker::configure(const pugi::xml_node& node, const std::string& configurationPath) {
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

	if (!(att = node.attribute("safeVolume")).empty()) {
		m_safe = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("limitVolume")).empty()) {
		m_limit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderMeanLife")).empty()) {
		m_orderMeanLife = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderFrequency")).empty()) {
		m_orderFrequency = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void MarketMaker::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleNextOrderPlacement();
	} else if (msg->type == "WAKEUP_FOR_PLACEMENT") {

		std::cout << currentTimestamp << " WAKEUP_FOR_PLACEMENT" << std::endl;

		if (position > m_limit) flag = false;
		else if (position < m_safe) flag = true;

		if (flag) {
			// queue an L1 data request
			simulation()->dispatchMessage(currentTimestamp, 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
		}
		else {
			scheduleNextOrderPlacement();
		}
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {

		std::cout << currentTimestamp << " RETRIEVE_L1" << std::endl;

		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);

		// place orders based on the current L1 status
		std::uniform_real_distribution<> priceUniformDistribution(std::numeric_limits<double>::min(), m_depthSpread);
		Money adjustPrice = Money(Decimal(priceUniformDistribution(simulation()->randomGenerator())) + (l1ptr->bestAskPrice - l1ptr->bestBidPrice).abs());
		Money price;

		price = (l1ptr->bestAskPrice + l1ptr->bestBidPrice) / 2 - adjustPrice.floorToCents();
		auto pptr = std::make_shared<PlaceOrderLimitPayload>(OrderDirection::Buy, m_volumeUnit, price);
		simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);

		price = (l1ptr->bestAskPrice + l1ptr->bestBidPrice) / 2 + adjustPrice.floorToCents();
		pptr = std::make_shared<PlaceOrderLimitPayload>(OrderDirection::Sell, m_volumeUnit, price);
		simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);
		
		position += 2 * m_volumeUnit;
	} else if (msg->type == "RESPONSE_PLACE_ORDER_LIMIT") {

		std::cout << currentTimestamp << " PLACE_ORDER_LIMIT" << std::endl;

		// auto responsepptr = std::dynamic_pointer_cast<PlaceOrderLimitResponsePayload>(msg->payload);
		// Timestamp delay = orderCancellationDelay();

		// auto pptr = std::make_shared<CancelOrdersPayload>();
		// pptr->cancellations.push_back(CancelOrdersCancellation(responsepptr->id, responsepptr->requestPayload->volume));
		// simulation()->dispatchMessage(currentTimestamp + delay, 0, this->name(), m_exchange, "CANCEL_ORDER", pptr);

		scheduleNextOrderPlacement();
	} else if (msg->type == "RESPONSE_CANCEL_ORDER") {

		std::cout << currentTimestamp << " CANCEL_ORDER" << std::endl;

		position -= m_volumeUnit;
	} else {
		// no op
	}
}

void MarketMaker::scheduleNextOrderPlacement() {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();
	double rate = 1.0 / m_orderFrequency;

	// generate a random wake up delay
	std::exponential_distribution<> exponentialDistribution(rate);
	Timestamp delay = (Timestamp)std::floor(exponentialDistribution(simulation()->randomGenerator()));

	std::cout << "delay " << delay << std::endl;

	// queue a placement
	simulation()->dispatchMessage(currentTimestamp, delay, name(), name(), "WAKEUP_FOR_PLACEMENT", std::make_shared<EmptyPayload>());
}

Timestamp MarketMaker::orderCancellationDelay() {
	double rate = 1.0 / m_orderMeanLife;

	// generate a random cancellation delay
	std::exponential_distribution<> exponentialDistribution(rate);
	return (Timestamp)std::floor(exponentialDistribution(simulation()->randomGenerator()));
}
