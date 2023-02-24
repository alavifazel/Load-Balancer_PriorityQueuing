#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include <fstream>
#include "load_balancer.hpp"
#include "job_pair.hpp"
#include "job.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer::loadbalancer {
    struct TestInputState {
		double sigma;
		Phase phase;
		double clock;
        int genIntervalIndex;
        std::vector<JobPair> jobPairs;
        std::vector<int> genIntervals;
		TestInputState(std::vector<JobPair> jobPairs, std::vector<int> genIntervals): 
                 phase(Active), genIntervalIndex(0), clock(), sigma() {
            if(jobPairs.size() != genIntervals.size()) {
                throw std::runtime_error("JobPairs should be same size as genInterval.");
            }
            this->jobPairs = jobPairs;
            this->genIntervals = genIntervals;
        }
	};

	std::ostream& operator<<(std::ostream& out, const TestInputState& s) {
		return out;
	}

	class TestInput : public Atomic<TestInputState> {
	 public:
		BigPort<JobPair> outGenerated;

		TestInput(const std::string& id, std::vector<JobPair> jobPairs, std::vector<int> genIntervals)
			: Atomic<TestInputState>(id, TestInputState(jobPairs, genIntervals))
		{
			outGenerated = addOutBigPort<JobPair>("outGenerated");
		}

		void internalTransition(TestInputState& s) const override {
            if(s.genIntervalIndex >= s.genIntervals.size() - 1) {
                s.sigma = std::numeric_limits<double>::infinity();
            } else {
                s.sigma = s.genIntervals[s.genIntervalIndex];
                s.genIntervalIndex++;
            }
		}

		void externalTransition(TestInputState& s, double e) const override {
		}

		void output(const TestInputState& s) const override {
			outGenerated->addMessage(s.jobPairs[s.genIntervalIndex]);

		}

		[[nodiscard]] double timeAdvance(const TestInputState& s) const override {
			return s.sigma;
		}
	};

	struct LoadBalancerTest: public Coupled {
		LoadBalancerTest(const std::string& id, std::vector<JobPair> jobPairs, std::vector<int> genIntervals): Coupled(id) {
			auto testInput = addComponent<TestInput>("TestInput", jobPairs, genIntervals);
            auto loadBalancer = addComponent<LoadBalancer>("loadBalancer", 10);
            addCoupling(testInput->outGenerated, loadBalancer->inJob);
		}
	};
}

int main(int argc, char *argv[]) {
    string filename = "../input_data/balancer_test_input.txt";
    ifstream myfile(filename);
    string line;
    std::vector<JobPair> jobPairs;
    std::vector<int> genIntervals;
    if(myfile.is_open()) {
        int priority, id, genInterval;
        while(getline(myfile, line)) {
            if(line[0] != '#') {
                myfile >> id >> priority >> genInterval;
                jobPairs.push_back(JobPair(priority, Job(id,-1,-1)));
                genIntervals.push_back(genInterval);
            }
        }
    }

    auto model = make_shared<loadbalancer::LoadBalancerTest>("LoadBalancerTest", jobPairs, genIntervals);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_balancer_test.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}