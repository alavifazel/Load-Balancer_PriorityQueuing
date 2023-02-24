
#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include <fstream>
#include "generator.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer {
    struct TestInputState {
		double sigma;
		double clock;
		TestInputState(double sigma): sigma(sigma) {}
	};

	std::ostream& operator<<(std::ostream& out, const TestInputState& s) {
		return out;
	}

	class TestInput : public Atomic<TestInputState> {
	public:
		Port<bool> outStop;
		TestInput(const std::string& id, int stopDelay)
			: Atomic<TestInputState>(id, TestInputState(stopDelay))
		{
			outStop = addOutPort<bool>("outStop");
		}

		void internalTransition(TestInputState& s) const override {
            s.sigma = std::numeric_limits<double>::infinity();
		}

		void externalTransition(TestInputState& s, double e) const override {
		}

		void output(const TestInputState& s) const override {
			outStop->addMessage(true);
		}

		[[nodiscard]] double timeAdvance(const TestInputState& s) const override {
			return s.sigma;
		}
	};

	struct GeneratorTest: public Coupled {
		GeneratorTest(const std::string& id, int jobPeriod, int stopDelay): Coupled(id) {
			auto testInput = addComponent<TestInput>("testInput", stopDelay);
            auto generator = addComponent<Generator>("generator", jobPeriod);
            addCoupling(testInput->outStop, generator->inStop);
		}
	};
}

int main(int argc, char *argv[]) {
    const int jobPeriod = 1, stopDelay = 10;

    auto model = make_shared<GeneratorTest>("GeneratorTest", jobPeriod, stopDelay);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_generator_test.csv", ";");

    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}