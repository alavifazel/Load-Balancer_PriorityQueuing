#ifndef LBS_MODEL
#define LBS_MODEL

#include <cadmium/core/modeling/coupled.hpp>
#include <array>
#include "server.hpp"
#include "load_balancer.hpp"

namespace cadmium::loadbalancer {
	struct LBS: public Coupled {
		BigPort<JobPair> inJob;
		std::array<BigPort<Job>, 3> outProcessed;

		LBS(const std::string& id, double processingTimeExpMean): Coupled(id) {
			inJob = addInBigPort<JobPair>("inJob");
			outProcessed[0] = addOutBigPort<Job>("outProcessed1");	
			outProcessed[1] = addOutBigPort<Job>("outProcessed2");
			outProcessed[2] = addOutBigPort<Job>("outProcessed3");

            auto loadBalancer = addComponent<LoadBalancer>("loadBalancer");
            std::array<std::shared_ptr<Server>, 3> servers;
			servers[0] = addComponent<Server>("server1", processingTimeExpMean);
			servers[1] = addComponent<Server>("server2", processingTimeExpMean);
			servers[2] = addComponent<Server>("server3", processingTimeExpMean);
			addCoupling(inJob, loadBalancer->inJob);
			addCoupling(loadBalancer->outJob[0], servers[0]->inGenerated);
			addCoupling(loadBalancer->outJob[1], servers[1]->inGenerated);
			addCoupling(loadBalancer->outJob[2], servers[2]->inGenerated);
			addCoupling(servers[0]->outProcessed, outProcessed[0]);
			addCoupling(servers[1]->outProcessed, outProcessed[1]);
			addCoupling(servers[2]->outProcessed, outProcessed[2]);
		}
	};
} 

#endif