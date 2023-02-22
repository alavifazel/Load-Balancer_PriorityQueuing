#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "load_balancer.hpp"
#include "job_pair.hpp"
#include "server.hpp"
#include "transducer.hpp"
#include "generator.hpp"

using namespace cadmium::loadbalancer;
using namespace std;


class GeneratorTest : public Generator {
public:

    GeneratorTest(const std::string& id, double jobPeriod): Generator(id,jobPeriod){}
    void output(const GeneratorState& s) const override {
            int priority;
            if(s.jobCount == 2 || s.jobCount == 5 || s.jobCount == 6)
                priority = 1;
            else
                priority = 6;
            outGenerated->addMessage(JobPair(priority, Job(s.jobCount, s.clock + s.sigma)));
        }
};

namespace cadmium::loadbalancer {
	struct LoadBalancerTest: public Coupled {
		LoadBalancerTest(const std::string& id, double jobPeriod, double processingTimeExpMean, double obsTime): Coupled(id) {
			auto generator = addComponent<GeneratorTest>("generator", jobPeriod);
            auto loadBalancer = addComponent<LoadBalancer>("loadBalancer", 10);
            std::array<std::shared_ptr<Server>, 3> servers;
			servers[0] = addComponent<Server>("server1", processingTimeExpMean);
			servers[1] = addComponent<Server>("server2", processingTimeExpMean);
			servers[2] = addComponent<Server>("server3", processingTimeExpMean);
			auto transducer = addComponent<Transducer>("transducer", obsTime);
			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(generator->outGenerated, loadBalancer->inJob);
			addCoupling(loadBalancer->outJob[0], servers[0]->inGenerated);
			addCoupling(loadBalancer->outJob[1], servers[1]->inGenerated);
			addCoupling(loadBalancer->outJob[2], servers[2]->inGenerated);
			addCoupling(servers[0]->outProcessed, transducer->inProcessed[0]);
			addCoupling(servers[1]->outProcessed, transducer->inProcessed[1]);
			addCoupling(servers[2]->outProcessed, transducer->inProcessed[2]);
			addCoupling(transducer->outStop, generator->inStop);
		}
	};
} 
int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "ERROR: not enough arguments.\n";
        cerr << "Usage:\n"; 
        cerr << "> ./main <GENERATION_PERIOD> <PROCESSING_TIME_EXP_MEAN> <OBSERVATION_TIME>\n";
        return -1;
    }
    int jobPeriod = stoi(argv[1]);
    double processingTimeExpMean = stof(argv[2]);
    double observationTime = stod(argv[3]);
    if(jobPeriod < 0 or processingTimeExpMean < 0 or observationTime < 0) {
        cerr << "ERROR: Parameters cannot be negative.\n";
        return -1;
    }

    auto model = make_shared<LoadBalancerTest>("LoadBalancerTest", jobPeriod, processingTimeExpMean, observationTime);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_gpt.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}