#ifndef LOAD_BALANCER_MODEL
#define LOAD_BALANCER_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <optional>
#include <array>
#include <queue>
#include "job.hpp"

namespace cadmium::loadbalancer {
	struct LoadBalancerState {
		double clock;
		double sigma;
		std::queue<Job> jobQueue;
		LoadBalancerState(): clock(), sigma(std::numeric_limits<double>::infinity()), jobQueue() {}
	};

	std::ostream& operator<<(std::ostream &out, const LoadBalancerState& s) {
		out << s.sigma;
		return out;
	}

	class LoadBalancer: public Atomic<LoadBalancerState> {
	private:
		double dispatchTime;	
	public:
		BigPort<Job> inJob;
		BigPort<int> inPriority;
		std::array<BigPort<Job>, 3> outJob;

		LoadBalancer(const std::string& id, double dispatchTime=0.5)
		: Atomic<LoadBalancerState>(id, LoadBalancerState()), 
		dispatchTime(dispatchTime)
		{
			inJob = addInBigPort<Job>("in");
			inPriority = addInBigPort<int>("inPriority");
			outJob[0] = addOutBigPort<Job>("out1");
			outJob[1] = addOutBigPort<Job>("out2");
			outJob[2] = addOutBigPort<Job>("out3");
		}
		void internalTransition(LoadBalancerState& s) const override {
			if(s.jobQueue.size() > 0) {
				s.jobQueue.pop();
				s.sigma = dispatchTime;
			} else {
				s.sigma = std::numeric_limits<double>::infinity();
			}
		}

		void externalTransition(LoadBalancerState& s, double e) const override {
			s.clock += e;
			auto newJob = inJob->getBag().back();
			s.jobQueue.push(Job(newJob->id, newJob->timeGenerated));
			if  (s.jobQueue.size() == 1) {
				s.sigma = dispatchTime;
			}
		}

		void output(const LoadBalancerState& s) const override {
			if(s.jobQueue.size() > 0) {
				Job job = s.jobQueue.front();
				if (job.id % 3 == 0) {
					outJob[0]->addMessage(job);
				} else if (job.id % 3 == 1) {
					outJob[1]->addMessage(job);
				} else {
					outJob[2]->addMessage(job);
				}
			}
		}

		[[nodiscard]] double timeAdvance(const LoadBalancerState& s) const override {
			return s.sigma;
		}
	};

}  

#endif
