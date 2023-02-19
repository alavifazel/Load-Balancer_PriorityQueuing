#ifndef GENERATOR_MODEL
#define GENERATOR_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <random>
#include "job.hpp"
#include "phase.hpp"

namespace cadmium::loadbalancer {
	struct GeneratorState {
		double sigma;
		Phase phase;
		double clock;
		long long jobCount;

		GeneratorState(): phase(Active), clock(), sigma(), jobCount() {}
	};

	std::ostream& operator<<(std::ostream& out, const GeneratorState& s) {
		out << s.jobCount;
		return out;
	}

	class Generator : public Atomic<GeneratorState> {
	 private:
		double jobPeriod;
		size_t randInt(size_t a, size_t b) const {
			static std::random_device dev;
			static std::mt19937 rng(dev());
			static std::uniform_int_distribution<std::mt19937::result_type> dist6(a,b);
			return dist6(rng);
		}
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
			switch(s.phase) {
				case Active:
					s.sigma = jobPeriod;
					s.jobCount += 1;
					break;
				case Passive:
					break; // Never reaches
			}
		}

		void externalTransition(GeneratorState& s, double e) const override {
			s.clock += e;
			if (!inStop->empty() && inStop->getBag().back()) {
				s.sigma = std::numeric_limits<double>::infinity();
				s.phase = Passive;
			}
		}

		void output(const GeneratorState& s) const override {
			outGenerated->addMessage(Job(s.jobCount, s.clock + s.sigma));
			outPriority->addMessage(randInt(1,5));
		}

		[[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
			return s.sigma;
		}
	};
}

#endif 