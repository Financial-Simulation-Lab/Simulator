#include "LiquidityTaker.h"

#include "Simulation.h"
#include "SimulationException.h"
#include "ParameterStorage.h"

LiquidityTaker::LiquidityTaker(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_volumeUnit(1), m_orderDirection(0.5), m_increment(0.01), m_timeStep(1), m_freq(0.025) {}

LiquidityTaker::LiquidityTaker(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_volumeUnit(1), m_orderDirection(0.5), m_increment(0.01), m_timeStep(1), m_freq(0.025) { }

void LiquidityTaker::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("volumeUnit")).empty()) {
		m_volumeUnit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("orderDirection")).empty()) {
		m_orderDirection = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("increment")).empty()) {
		m_increment = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("timeStep")).empty()) {
		m_timeStep = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("frequency")).empty()) {
		m_freq = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void LiquidityTaker::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleLiquidityTaking();
	} else if (msg->type == "WAKEUP_FOR_MARKETMAKING") {
		std::bernoulli_distribution orderFrequencyDistribution(m_freq);

		if (orderFrequencyDistribution(simulation()->randomGenerator())) {
			std::bernoulli_distribution orderDirectionDistribution(simulation()->q_taker(step));
			OrderDirection direction = orderDirectionDistribution(simulation()->randomGenerator()) ? OrderDirection::Buy : OrderDirection::Sell;

			auto pptr = std::make_shared<PlaceOrderMarketPayload>(direction, m_volumeUnit);
			simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_MARKET", pptr);
		} else {
			// no op
		}

		// std::bernoulli_distribution orderDirectionDistribution(simulation()->q_taker(step));
		// OrderDirection direction = orderDirectionDistribution(simulation()->randomGenerator()) ? OrderDirection::Buy : OrderDirection::Sell;

		// auto pptr = std::make_shared<PlaceOrderMarketPayload>(direction, m_volumeUnit);
		// simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_MARKET", pptr);

		step++;
		scheduleLiquidityTaking();
	}
}

void LiquidityTaker::scheduleLiquidityTaking() {
	// fix time step to 1 sec
	const Timestamp currentTimestamp = simulation()->currentTimestamp();
	simulation()->dispatchMessage(currentTimestamp, 1, this->name(), this->name(), "WAKEUP_FOR_MARKETMAKING", std::make_shared<EmptyPayload>());

	// const Timestamp currentTimestamp = simulation()->currentTimestamp();
	// std::exponential_distribution orderDelayDistribution(1.0 / m_timeStep);
	// Timestamp delay = (Timestamp)orderDelayDistribution(simulation()->randomGenerator());

	// simulation()->dispatchMessage(currentTimestamp, delay, this->name(), this->name(), "WAKEUP_FOR_MARKETMAKING", std::make_shared<EmptyPayload>());
}