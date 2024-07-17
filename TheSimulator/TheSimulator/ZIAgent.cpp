#include "ZIAgent.h"

#include "Simulation.h"
#include "SimulationException.h"
#include "ExchangeAgentMessagePayloads.h"
#include "ParameterStorage.h"

#include <random>
#include <cmath>

ZIAgent::ZIAgent(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_halfSpread(0.01), m_volumeUnit(1), m_marketOrderFraction(0.0), m_orderDirection(0.0), m_timeStep(1), m_orderMeanLifeTime(1e9) {}

ZIAgent::ZIAgent(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_halfSpread(0.01), m_volumeUnit(1), m_marketOrderFraction(0.0), m_orderDirection(0.0), m_timeStep(1), m_orderMeanLifeTime(1e9) { }

void ZIAgent::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("halfSpread")).empty()) {
		m_halfSpread = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("volumeUnit")).empty()) {
		m_volumeUnit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("marketOrderFraction")).empty()) {
		m_marketOrderFraction = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderDirection")).empty()) {
		m_orderDirection = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("timeStep")).empty()) {
		m_timeStep = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderMeanLifeTime")).empty()) {
		m_orderMeanLifeTime = std::stoull(simulation()->parameters().processString(att.as_string()));
	}
}

void ZIAgent::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleMarketMaking();
	} else if (msg->type == "WAKEUP_FOR_MARKETMAKING") {
		simulation()->dispatchMessage(currentTimestamp, 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {
		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);
		
		std::bernoulli_distribution orderTypeDistribution(m_marketOrderFraction);
		std::bernoulli_distribution orderDirectionDistribution(m_orderDirection);
		std::exponential_distribution<> orderVolumnDistribution(1 / m_volumeUnit);

		bool isMarketOrder = orderTypeDistribution(simulation()->randomGenerator());
		OrderDirection direction = orderDirectionDistribution(simulation()->randomGenerator()) ? OrderDirection::Buy : OrderDirection::Sell;
		Volume orderVolumn = (Volume)orderVolumnDistribution(simulation()->randomGenerator());

		if (isMarketOrder){
			auto pptr = std::make_shared<PlaceOrderMarketPayload>(direction, orderVolumn);
			simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_MARKET", pptr);
		}
		else {
			std::exponential_distribution<> orderSpreadDistribution(1 / m_halfSpread);
			Money orderSpread = orderSpreadDistribution(simulation()->randomGenerator());
			const auto inCents = orderSpread.floorToCents();

			Money price;
			if (direction == OrderDirection::Buy) {
				price = l1ptr->bestAskPrice - inCents;
			} else {
				price = l1ptr->bestBidPrice + inCents;
			}
			auto pptr = std::make_shared<PlaceOrderLimitPayload>(direction, orderVolumn, price);
			simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);
		}

		scheduleMarketMaking();
	} else if (msg->type == "RESPONSE_CANCEL_ORDERS") {
		// no op
	} else if (msg->type == "RESPONSE_PLACE_ORDER_LIMIT") {
		auto polptr = std::dynamic_pointer_cast<PlaceOrderLimitResponsePayload>(msg->payload);
		auto cpptr = std::make_shared<CancelOrdersPayload>();
		cpptr->cancellations.push_back(CancelOrdersCancellation(polptr->id, (Volume)1e9));
		auto delay = computeOrderCancellationDelay();
		simulation()->dispatchMessage(currentTimestamp, delay, this->name(), m_exchange, "CANCEL_ORDERS", cpptr);
	} else if (msg->type == "RESPONSE_PLACE_ORDER_MARKET") {
		// no need to cancel market order, no op
	} else if (msg->type == "EVENT_TRADE") {
		// no op
	}
}

void ZIAgent::scheduleMarketMaking() {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();
	std::exponential_distribution<> orderDelayDistribution(1.0 / m_timeStep);
	Timestamp delay = (Timestamp)orderDelayDistribution(simulation()->randomGenerator());

	simulation()->dispatchMessage(currentTimestamp, delay, this->name(), this->name(), "WAKEUP_FOR_MARKETMAKING", std::make_shared<EmptyPayload>());
}

Timestamp ZIAgent::computeOrderCancellationDelay() {
	Timestamp adjustedMeanOrderLifetime = (Timestamp)(m_orderMeanLifeTime * (1 + m_marketOrderFraction));
	double nextCancellationRate = 1.0 / adjustedMeanOrderLifetime;

	std::exponential_distribution<> exponentialDistribution(nextCancellationRate);
	Timestamp delay = (Timestamp)std::floor(exponentialDistribution(simulation()->randomGenerator()));

	return delay;
}