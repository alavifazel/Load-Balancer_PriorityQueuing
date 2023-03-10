#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "load_balancer_system.hpp"
#include "generator.hpp"
#include "transducer.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer {
    Port<double> throughput;
    Port<double> averageProcessingTime;
	struct TopLevelModel: public Coupled {
		TopLevelModel(const std::string& id, double genPeriod, double processingTimeExpMean, double obsTime): Coupled(id) {
            averageProcessingTime = addOutPort<double>("averageProcessingTime");	
            throughput = addOutPort<double>("throughput");	
            auto lbs = addComponent<LoadBalancerSystem>("LoadBalancerSystem", processingTimeExpMean);
			auto generator = addComponent<Generator>("generator", genPeriod);
			auto transducer = addComponent<Transducer>("transducer", obsTime, processingTimeExpMean);
			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(generator->outGenerated, lbs->inJob);
			addCoupling(lbs->outProcessed[0], transducer->inProcessed[0]);
			addCoupling(lbs->outProcessed[1], transducer->inProcessed[1]);
			addCoupling(lbs->outProcessed[2], transducer->inProcessed[2]);
			addCoupling(transducer->outStop, generator->inStop);
			addCoupling(transducer->throughput, throughput);
            addCoupling(transducer->averageProcessingTime, averageProcessingTime);
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
    double genPeriod = stof(argv[1]);
    double processingTimeExpMean = stof(argv[2]);
    double observationTime = stod(argv[3]);
    if(genPeriod < 0 or processingTimeExpMean < 0 or observationTime < 0) {
        cerr << "ERROR: Parameters cannot be negative.\n";
        return -1;
    }

    auto model = make_shared<TopLevelModel>("top", genPeriod, processingTimeExpMean, observationTime);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}
