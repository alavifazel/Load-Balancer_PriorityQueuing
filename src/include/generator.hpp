#ifndef GENERATOR_MODEL
#define GENERATOR_MODEL

#include <cadmium/core/modeling/atomic.hpp>
#include <iostream>
#include <random>
#include <utility>
#include "job.hpp"
#include "job_pair.hpp"
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
		double genPeriod;
		size_t randInt(size_t a, size_t b) const {
			static std::random_device dev;
			static std::mt19937 rng(dev());
			static std::uniform_int_distribution<std::mt19937::result_type> dist6(a,b);
			return dist6(rng);
		}
	 public:
		Port<bool> inStop;
		Port<JobPair> outGenerated;

		Generator(const std::string& id, double genPeriod)
			: Atomic<GeneratorState>(id, GeneratorState()), genPeriod(genPeriod)
		{
			inStop = addInPort<bool>("inStop");
			outGenerated = addOutPort<JobPair>("outGenerated");
		}

		void internalTransition(GeneratorState& s) const override {
			switch(s.phase) {
				case Active:
					s.sigma = genPeriod;
					s.jobCount += 1;
					s.clock += s.sigma;
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
			outGenerated->addMessage(JobPair(randInt(1,5), Job(s.jobCount, s.clock + s.sigma)));
		}

		[[nodiscard]] double timeAdvance(const GeneratorState& s) const override {
			return s.sigma;
		}
	};
}

#endif 