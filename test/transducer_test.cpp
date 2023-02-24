#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include <fstream>
#include "transducer.hpp"
#include "job.hpp"
#include "job_pair.hpp"
using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer {
    struct TestInputState {
		double sigma;
		double clock;
        int numOfJobGenerated;
		TestInputState(): sigma(0), clock(0), numOfJobGenerated(0) {}
	};

	std::ostream& operator<<(std::ostream& out, const TestInputState& s) {
		return out;
	}

	class TestInput : public Atomic<TestInputState> {
	public:
		BigPort<JobPair> outGenerated;
		std::array<BigPort<Job>, 3> outProcessed;
        Port<bool> inStop;
        int generationPeriod;

		TestInput(const std::string& id, int generationPeriod)
			: Atomic<TestInputState>(id, TestInputState()), generationPeriod(generationPeriod)
		{
            outGenerated    = addOutBigPort<JobPair>("outGenerated");
			outProcessed[0] = addOutBigPort<Job>("outProcessed1");
			outProcessed[1] = addOutBigPort<Job>("outProcessed2");
			outProcessed[2] = addOutBigPort<Job>("outProcessed3");
            inStop = addInPort<bool>("inStop");
		}

		void internalTransition(TestInputState& s) const override {
            s.clock += s.sigma;
            s.sigma = generationPeriod;
            s.numOfJobGenerated++;
		}

		void externalTransition(TestInputState& s, double e) const override {
            if (!inStop->empty() && inStop->getBag().back()) {
				s.sigma = std::numeric_limits<double>::infinity();
			}
        }

		void output(const TestInputState& s) const override {
            Job job = Job(s.numOfJobGenerated % 4, s.clock, 2);
            if(s.numOfJobGenerated % 2 == 0) {
                outGenerated->addMessage(JobPair(5, job));
            } else {
                outProcessed[s.numOfJobGenerated % 3]->addMessage(job);
            }
		}

		[[nodiscard]] double timeAdvance(const TestInputState& s) const override {
			return s.sigma;
		}
	};

	struct TransducerTest: public Coupled {
		TransducerTest(const std::string& id, int generationPeriod): Coupled(id) {
			auto testInput = addComponent<TestInput>("testInput", generationPeriod);
            auto transducer = addComponent<Transducer>("transducer", 100);
            addCoupling(testInput->outGenerated, transducer->inGenerated);
			addCoupling(testInput->outProcessed[0], transducer->inProcessed[0]);
			addCoupling(testInput->outProcessed[1], transducer->inProcessed[1]);
			addCoupling(testInput->outProcessed[2], transducer->inProcessed[2]);
            addCoupling(transducer->outStop, testInput->inStop);
		}
	};
}

int main(int argc, char *argv[]) {
    const int generationPeriod = 2;
    
    auto model = make_shared<TransducerTest>("GeneratorTest", generationPeriod);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_transducer_test.csv", ";");

    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}