#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include <fstream>
#include "server.hpp"
#include "job.hpp"

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
		BigPort<Job> outJob;
        int numOfJobsToGenerate;
        int generationPeriod;
		TestInput(const std::string& id, int numOfJobsToGenerate, int generationPeriod)
			: Atomic<TestInputState>(id, TestInputState()), numOfJobsToGenerate(numOfJobsToGenerate), generationPeriod(generationPeriod)
		{
			outJob = addOutBigPort<Job>("outStop");
		}

		void internalTransition(TestInputState& s) const override {
            s.clock += s.sigma;
            if(s.numOfJobGenerated < numOfJobsToGenerate) { 
                s.sigma = generationPeriod;
                s.numOfJobGenerated++;
            } else {
                s.sigma = std::numeric_limits<double>::infinity();
            }
		}

		void externalTransition(TestInputState& s, double e) const override {}

		void output(const TestInputState& s) const override {
			outJob->addMessage(Job(s.numOfJobGenerated, s.clock));
		}

		[[nodiscard]] double timeAdvance(const TestInputState& s) const override {
			return s.sigma;
		}
	};

	struct ServerTest: public Coupled {
		ServerTest(const std::string& id, double processingTimeExpMean, int numOfJobsToGenerate, int generationPeriod): Coupled(id) {
			auto testInput = addComponent<TestInput>("testInput", numOfJobsToGenerate, generationPeriod);
            auto server = addComponent<Server>("server", processingTimeExpMean);
            addCoupling(testInput->outJob, server->inGenerated);
		}
	};
}

int main(int argc, char *argv[]) {
    string filename = "../input_data/server_test_input.txt";
    ifstream myfile(filename);
    string line;
    int processingTimeExpMean;
    int numOfJobsToGenerate;
    int generationPeriod;
    if(myfile.is_open()) {
        while(getline(myfile, line)) {
            if(line[0] != '#') {
                 std::istringstream is(line);
                is >> processingTimeExpMean >> numOfJobsToGenerate >> generationPeriod;
            }
        }
        myfile.close();
    }

    auto model = make_shared<ServerTest>("GeneratorTest", processingTimeExpMean, numOfJobsToGenerate, generationPeriod);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_server_test.csv", ";");

    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}