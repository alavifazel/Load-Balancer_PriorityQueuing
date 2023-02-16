#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <limits>
#include "model/lbs.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 4) {
        cerr << "ERROR: not enough arguments.\n";
        cerr << "Usage:\n"; 
        cerr << "> ./main <GENERATION_PERIOD> <PROCESSING_TIME_EXP_LAMBDA> <OBSERVATION_TIME>\n";
        return -1;
    }
    int jobPeriod = stoi(argv[1]);
    int processingTimeExpLambda = stoi(argv[2]);
    double observationTime = stod(argv[3]);
    if(jobPeriod < 0 or processingTimeExpLambda < 0 or observationTime < 0) {
        cerr << "ERROR: Parameters cannot be negative.\n";
        return -1;
    }

    auto model = make_shared<LBS>("LBS", jobPeriod, processingTimeExpLambda, observationTime);
    auto rootCoordinator = cadmium::RootCoordinator(model);
    auto logger = make_shared<cadmium::CSVLogger>("log_gpt.csv", ";");
    rootCoordinator.setLogger(logger);
    rootCoordinator.start();
    rootCoordinator.simulate(numeric_limits<double>::infinity());
    rootCoordinator.stop();
    return 0;
}