#include "LiquidityProvider.h"

#include "Simulation.h"
#include "SimulationException.h"
#include "ParameterStorage.h"

#include <math.h>

LiquidityProvider::LiquidityProvider(const Simulation* simulation)
	: Agent(simulation), m_exchange(""), m_orderDirection(0.5), m_volumeUnit(1), m_timeStep(1), m_depth(100), m_c(10), m_freq(0.15), m_delta(0.025) {}

LiquidityProvider::LiquidityProvider(const Simulation* simulation, const std::string& name)
	: Agent(simulation, name), m_exchange(""), m_orderDirection(0.5), m_volumeUnit(1), m_timeStep(1), m_depth(100), m_c(10), m_freq(0.15), m_delta(0.025) { }

void LiquidityProvider::configure(const pugi::xml_node& node, const std::string& configurationPath) {
	Agent::configure(node, configurationPath);

	pugi::xml_attribute att;
	if (!(att = node.attribute("exchange")).empty()) {
		m_exchange = simulation()->parameters().processString(att.as_string());
	}

	if (!(att = node.attribute("orderDirection")).empty()) {
		m_orderDirection = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("volumeUnit")).empty()) {
		m_volumeUnit = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("timeStep")).empty()) {
		m_timeStep = std::stod(simulation()->parameters().processString(att.as_string()));
	}

    if (!(att = node.attribute("depth")).empty()) {
		m_depth = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("integer_c")).empty()) {
		m_c = std::stoi(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("frequency")).empty()) {
		m_freq = std::stod(simulation()->parameters().processString(att.as_string()));
	}

	if (!(att = node.attribute("delta")).empty()) {
		m_delta = std::stod(simulation()->parameters().processString(att.as_string()));
	}
}

void LiquidityProvider::receiveMessage(const MessagePtr& msg) {
	const Timestamp currentTimestamp = simulation()->currentTimestamp();

	if (msg->type == "EVENT_SIMULATION_START") {
		scheduleLiquidityProviding();
	} else if (msg->type == "WAKEUP_FOR_MARKETMAKING") {
		std::bernoulli_distribution orderFrequencyDistribution(m_freq);

		if (orderFrequencyDistribution(simulation()->randomGenerator())) {
			simulation()->dispatchMessage(currentTimestamp, 0, name(), m_exchange, "RETRIEVE_L1", std::make_shared<EmptyPayload>());
		} else {
			// no op
			step++;
			scheduleLiquidityProviding();
		}
	} else if (msg->type == "RESPONSE_RETRIEVE_L1") {
		auto l1ptr = std::dynamic_pointer_cast<RetrieveL1ResponsePayload>(msg->payload);
		std::bernoulli_distribution orderDirectionDistribution(m_orderDirection);
        std::uniform_real_distribution<double> uDistribution(0.0, 1.0);

        OrderDirection direction = orderDirectionDistribution(simulation()->randomGenerator()) ? OrderDirection::Buy : OrderDirection::Sell;

		double u = uDistribution(simulation()->randomGenerator());
		double lambda_t = simulation()->lambda_t(step);
		int eta = floor(-lambda_t * log(u));

		Money spread = Money(1 + eta);
		const auto inCents = spread.floorToCents();

		Money price;
		if (direction == OrderDirection::Buy) {
			price = l1ptr->bestAskPrice - inCents;
		} else {
			price = l1ptr->bestBidPrice + inCents;
		}

        auto pptr = std::make_shared<PlaceOrderLimitPayload>(direction, m_volumeUnit, price);
		simulation()->dispatchMessage(currentTimestamp, 0, this->name(), m_exchange, "PLACE_ORDER_LIMIT", pptr);

		step++;
		scheduleLiquidityProviding();
	} else if (msg->type == "RESPONSE_PLACE_ORDER_LIMIT") {
		std::bernoulli_distribution orderFrequencyDistribution(m_delta);

		if (orderFrequencyDistribution(simulation()->randomGenerator())) {
			auto polptr = std::dynamic_pointer_cast<PlaceOrderLimitResponsePayload>(msg->payload);
			auto cpptr = std::make_shared<CancelOrdersPayload>();
			cpptr->cancellations.push_back(CancelOrdersCancellation(polptr->id, (Volume)1));
			simulation()->dispatchMessage(currentTimestamp, 0.5, this->name(), m_exchange, "CANCEL_ORDERS", cpptr);
		} else {
			// no op
		}

		
	}
}

void LiquidityProvider::scheduleLiquidityProviding() {
	// fix time step to 1 sec
	const Timestamp currentTimestamp = simulation()->currentTimestamp();
	simulation()->dispatchMessage(currentTimestamp, 1, this->name(), this->name(), "WAKEUP_FOR_MARKETMAKING", std::make_shared<EmptyPayload>());

	// const Timestamp currentTimestamp = simulation()->currentTimestamp();
	// std::exponential_distribution orderDelayDistribution(1.0 / m_timeStep);
	// Timestamp delay = (Timestamp)orderDelayDistribution(simulation()->randomGenerator());

	// simulation()->dispatchMessage(currentTimestamp, delay, this->name(), this->name(), "WAKEUP_FOR_MARKETMAKING", std::make_shared<EmptyPayload>());
}