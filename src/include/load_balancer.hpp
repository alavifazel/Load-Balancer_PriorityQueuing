#ifndef LOAD_BALANCER_MODEL
#define LOAD_BALANCER_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <optional>
#include <array>
#include <queue>
#include <utility>
#include "job.hpp"
#include "job_pair.hpp"
#include "phase.hpp"

namespace cadmium::loadbalancer {
	typedef std::pair<int,Job> jobpair_t;
	class jobPairCompare {
	public:
		bool operator()(const jobpair_t& lhs, const jobpair_t& rhs)
		{
			return rhs.first < lhs.first;
		}
	};

	struct LoadBalancerState {
		double sigma;
		Phase phase;
		std::priority_queue<jobpair_t, std::vector<jobpair_t>, jobPairCompare> jobQueue;
		LoadBalancerState(): sigma(std::numeric_limits<double>::infinity()), jobQueue(), phase(Passive) {}
	};

	std::ostream& operator<<(std::ostream &out, const LoadBalancerState& s) {
		out << s.sigma;
		return out;
	}

	class LoadBalancer: public Atomic<LoadBalancerState> {
	private:
		double dispatchTime;	
	public:
		Port<JobPair> inJob;
		std::array<Port<Job>, 3> outJob;

		LoadBalancer(const std::string& id, double dispatchTime=0.1)
		: Atomic<LoadBalancerState>(id, LoadBalancerState()), 
		dispatchTime(dispatchTime)
		{
			inJob = addInPort<JobPair>("in");
			outJob[0] = addOutPort<Job>("out1");
			outJob[1] = addOutPort<Job>("out2");
			outJob[2] = addOutPort<Job>("out3");
		}
		void internalTransition(LoadBalancerState& s) const override {
			s.jobQueue.pop();
			if(s.jobQueue.empty()) {
				s.phase = Passive;
				s.sigma = std::numeric_limits<double>::infinity();
			} else {
				s.sigma = dispatchTime;
			}
		}

		void externalTransition(LoadBalancerState& s, double e) const override {
			auto newJob = inJob->getBag().back();
			s.jobQueue.push(std::make_pair(newJob.priority, Job(newJob.job.id, newJob.job.timeGenerated, newJob.job.timeGenerated)));

			switch(s.phase) {
				case Active:
				{
					s.sigma -= e;
					break;
				}
				case Passive:
				{
					s.sigma = dispatchTime;
					s.phase = Active;
					break;
				}
			}
		}

		void output(const LoadBalancerState& s) const override {
			Job job = s.jobQueue.top().second;
			if (job.id % 3 == 0) {
				outJob[0]->addMessage(job);
			} else if (job.id % 3 == 1) {
				outJob[1]->addMessage(job);
			} else {
				outJob[2]->addMessage(job);
			}
		}

		[[nodiscard]] double timeAdvance(const LoadBalancerState& s) const override {
			return s.sigma;
		}
	};

}  

#endif
