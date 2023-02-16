#ifndef GENERATOR_MODEL
#define GENERATOR_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <random>
#include "job.hpp"

namespace cadmium::loadbalancer {
	struct GeneratorState {
		double clock;
		double sigma;
		long long jobCount;

		GeneratorState(): clock(), sigma(), jobCount() {}
	};

	std::ostream& operator<<(std::ostream& out, const GeneratorState& s) {
		out << s.jobCount;
		return out;
	}

	class Generator : public Atomic<GeneratorState> {
	 private:
		double jobPeriod;
	 public:
		Port<bool> inStop;
		BigPort<Job> outGenerated;
		BigPort<int> outPriority;

		Generator(const std::string& id, double jobPeriod)
			: Atomic<GeneratorState>(id, GeneratorState()), jobPeriod(jobPeriod)
		{
			inStop = addInPort<bool>("inStop");
			outPriority = addOutBigPort<int>("outPriority");
			outGenerated = addOutBigPort<Job>("outGenerated");
		}

		void internalTransition(GeneratorState& s) const override {
			s.clock += s.sigma;
			s.sigma = jobPeriod;
			s.jobCount += 1;
		}

		void externalTransition(GeneratorState& s, double e) const override {
			s.clock += e;
			// s.sigma = std::max(s.sigma - e, 0.);
			if (!inStop->empty() && inStop->getBag().back()) {
				s.sigma = std::numeric_limits<double>::infinity();
			}
		}

		void output(const GeneratorState& s) const override {
			outGenerated->addMessage(Job(s.jobCount, s.clock + s.sigma));
			outPriority->addMessage(3);
		}

		[[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
			return s.sigma;
		}
	};
}

#endif 