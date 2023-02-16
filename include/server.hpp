#ifndef SERVER_MODEL
#define SERVER_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <queue>
#include <random>
#include "job.hpp"

namespace cadmium::loadbalancer {
	struct ServerState {
		double clock;
		double sigma; 
		std::queue<Job> jobQueue;
		ServerState(): clock(), sigma(std::numeric_limits<double>::infinity()), jobQueue() {}
	};

	std::ostream& operator<<(std::ostream &out, const ServerState& s) {
		out << s.sigma;
		return out;
	}

	class Server: public Atomic<ServerState> {
	private:
		double processingTimeExpLambda;
		double generateProcessingTime() const {
			std::random_device rd;
			std::exponential_distribution<> rng(processingTimeExpLambda);
			std::mt19937 rnd_gen(rd());
			return rng(rnd_gen);
		}
	public:
		BigPort<Job> inGenerated;
		BigPort<Job> outProcessed;

		Server(const std::string& id, double processingTimeExpLambda):
			Atomic<ServerState>(id, ServerState()),
			processingTimeExpLambda(processingTimeExpLambda)
			 {
			inGenerated = addInBigPort<Job>("inGenerated");
			outProcessed = addOutBigPort<Job>("outProcessed");
		}

		void internalTransition(ServerState& s) const override {
			s.clock += s.sigma;
			// Start processing jobs if queue of jobs is not empty
			if(s.jobQueue.size() > 0) {
				s.sigma = generateProcessingTime();
				s.jobQueue.pop();
			} else {
				s.sigma = std::numeric_limits<double>::infinity();
			}
		}

		void externalTransition(ServerState& s, double e) const override {
			s.clock += e;
			s.sigma -= e;
			if (!inGenerated->empty()) {
				auto newJob = inGenerated->getBag().back();
				s.jobQueue.push(Job(newJob->id, newJob->timeGenerated));

				// Start processing if it is the only job in the queue
				if  (s.jobQueue.size() == 1) {
					s.sigma = generateProcessingTime();
				} 
			}
		}

		void output(const ServerState& s) const override {
			if(s.jobQueue.size() > 0)
				outProcessed->addMessage(s.jobQueue.front());
		}

		[[nodiscard]] double timeAdvance(const ServerState& s) const override {
			return s.sigma;
		}
	};
}

#endif