#ifndef SERVER_MODEL
#define SERVER_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <queue>
#include <random>
#include "job.hpp"
#include "phase.hpp"

namespace cadmium::loadbalancer {
	struct ServerState {
		double sigma; 
		Phase phase;
		std::queue<Job> jobQueue;
		ServerState(): sigma(std::numeric_limits<double>::infinity()), jobQueue(), phase(Passive) {}
	};

	std::ostream& operator<<(std::ostream &out, const ServerState& s) {
		out << s.sigma;
		return out;
	}

	class Server: public Atomic<ServerState> {
	private:
		double processingTimeExpMean;
		double generateProcessingTime() const {
			std::random_device rd;
			std::exponential_distribution<> rng(1/processingTimeExpMean);
			std::mt19937 rnd_gen(rd());
			return rng(rnd_gen);
		}
	public:
		BigPort<Job> inGenerated;
		BigPort<Job> outProcessed;

		Server(const std::string& id, double processingTimeExpMean):
			Atomic<ServerState>(id, ServerState()),
			processingTimeExpMean(processingTimeExpMean)
			 {
			inGenerated = addInBigPort<Job>("inGenerated");
			outProcessed = addOutBigPort<Job>("outProcessed");
		}

		void internalTransition(ServerState& s) const override {
			s.jobQueue.pop();
			if (s.jobQueue.size() == 0) {
				s.phase = Passive;
				s.sigma = std::numeric_limits<double>::infinity();
			} else {
				s.sigma = generateProcessingTime();
			}
		}

		void externalTransition(ServerState& s, double e) const override {
			auto processingTime = generateProcessingTime();
			auto newJob = inGenerated->getBag().back();
			s.jobQueue.push(Job(newJob->id, 
							newJob->timeGenerated, 
							newJob->timeGenerated + processingTime));
			switch(s.phase) {
				case Active:
				{
					s.sigma -= e;
					break;
				}
				case Passive:
				{
					s.sigma = processingTime;
					s.phase = Active;
					break;
				}
			}
		}

		void output(const ServerState& s) const override {
			outProcessed->addMessage(s.jobQueue.front());
		}

		[[nodiscard]] double timeAdvance(const ServerState& s) const override {
			return s.sigma;
		}
	};
}

#endif