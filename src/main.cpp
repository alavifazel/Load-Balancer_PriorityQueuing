#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "lbs.hpp"
#include "generator.hpp"
#include "transducer.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer {
	struct LoadBalancedNetwork: public Coupled {
		LoadBalancedNetwork(const std::string& id, double jobPeriod, double processingTimeExpMean, double obsTime): Coupled(id) {

            auto lbs = addComponent<LBS>("LBS", processingTimeExpMean);
			auto generator = addComponent<Generator>("generator", jobPeriod);
			auto transducer = addComponent<Transducer>("transducer", obsTime);

			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(generator->outGenerated, lbs->inJob);
			addCoupling(lbs->outProcessed[0], transducer->inProcessed[0]);
			addCoupling(lbs->outProcessed[1], transducer->inProcessed[1]);
			addCoupling(lbs->outProcessed[2], transducer->inProcessed[2]);
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

    auto model = make_shared<LoadBalancedNetwork>("top", jobPeriod, processingTimeExpMean, observationTime);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}