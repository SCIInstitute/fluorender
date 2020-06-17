#ifndef AGENT_WRAPPER_HPP
#define AGENT_WRAPPER_HPP

#include <string>

template<class Agent>
class AgentWrapper
{
  public:
	AgentWrapper(const std::string &name, Agent *agent) : functionName(name), parentAgent(agent){}
    
	template<typename T>
    void updateValue(T value) { parentAgent->setValue(functionName,value); } 

  private:
	const std::string functionName;
	Agent *parentAgent;
};

#endif
