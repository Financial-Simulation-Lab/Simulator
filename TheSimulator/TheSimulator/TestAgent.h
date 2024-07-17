#pragma once
#include "Agent.h"
#include "Order.h"

#include <memory>
#include <fstream>
#include "ExchangeAgentMessagePayloads.h"
#include <string>

class TestAgent : public Agent {
public:
	TestAgent(const Simulation* simulation);
	TestAgent(const Simulation* simulation, const std::string& name);

	void configure(const pugi::xml_node& node, const std::string& configurationPath);

	// Inherited via Agent
	void receiveMessage(const MessagePtr& msg) override;
private:
	std::string m_exchange;
	std::string m_file;
	std::ifstream inFile;
};
