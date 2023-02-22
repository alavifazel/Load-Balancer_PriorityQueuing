#ifndef LBS_MODEL
#define LBS_MODEL

#include <cadmium/core/modeling/coupled.hpp>
#include <array>
#include "generator.hpp"
#include "server.hpp"
#include "transducer.hpp"
#include "load_balancer.hpp"

namespace cadmium::loadbalancer {
	struct LBS: public Coupled {
		LBS(const std::string& id, double jobPeriod, double processingTimeExpMean, double obsTime): Coupled(id) {
			auto generator = addComponent<Generator>("generator", jobPeriod);
            auto loadBalancer = addComponent<LoadBalancer>("loadBalancer");
            std::array<std::shared_ptr<Server>, 3> servers;
			servers[0] = addComponent<Server>("server1", processingTimeExpMean);
			servers[1] = addComponent<Server>("server2", processingTimeExpMean);
			servers[2] = addComponent<Server>("server3", processingTimeExpMean);
			auto transducer = addComponent<Transducer>("transducer", obsTime);
			addCoupling(generator->outGenerated, transducer->inGenerated);
			addCoupling(generator->outGenerated, loadBalancer->inJob);
			addCoupling(generator->outPriority, loadBalancer->inPriority);
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

#endif