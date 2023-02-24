#include <cadmium/core/logger/csv.hpp>
#include <cadmium/core/simulation/root_coordinator.hpp>
#include <cadmium/lib/iestream.hpp>
#include <limits>
#include <fstream>
#include "generator.hpp"
#include "job_pair.hpp"
#include "job.hpp"

using namespace cadmium::loadbalancer;
using namespace std;

namespace cadmium::loadbalancer::iestream {
	struct iestream_coupled : public Coupled {
		iestream_coupled(const std::string& id, const char* filePath, double jobPeriod): Coupled(id) {
			auto iestream1 = addComponent<lib::IEStream<bool>>("iestream", filePath);
            auto generator = addComponent<Generator>("generator", jobPeriod);
            addCoupling(iestream1->out, generator->inStop);
		}
	};
}

int main(int argc, char *argv[]) {
	std::ifstream file;
    const double jobPeriod = 2;
	if (argc < 2) { // Check that file is included
        std::cerr << "ERROR: not enough arguments" << std::endl;
        std::cerr << "    Usage:" << std::endl;
        std::cerr << "    > main_iestream INPUTFILE" << std::endl;
        return -1;
    }

	const char* filePath = argv[1];
    
	file.open(filePath);
    if(!file.is_open()) { // Check file can be opened
        std::cerr << "ERROR: file can not be opened. Check file path." << std::endl;
        return -1;
    }

	auto model = std::make_shared<iestream::iestream_coupled>("IEStreamCoupled", filePath, jobPeriod);
	auto rootCoordinator = cadmium::RootCoordinator(model);
	auto logger = std::make_shared<cadmium::CSVLogger>("generator_test.csv", ";");
	rootCoordinator.setLogger(logger);
	rootCoordinator.start();
	rootCoordinator.simulate(std::numeric_limits<double>::infinity());
	rootCoordinator.stop();
	return 0;
}