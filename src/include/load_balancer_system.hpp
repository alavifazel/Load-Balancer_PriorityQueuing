#ifndef LBS_MODEL
#define LBS_MODEL

#include <cadmium/core/modeling/coupled.hpp>
#include <array>
#include "server.hpp"
#include "load_balancer.hpp"

namespace cadmium::loadbalancer {
	struct LoadBalancerSystem: public Coupled {
		Port<JobPair> inJob;
		std::array<Port<Job>, 3> outProcessed;

		LoadBalancerSystem(const std::string& id, double processingTimeExpMean): Coupled(id) {
			inJob = addInPort<JobPair>("inJob");
			outProcessed[0] = addOutPort<Job>("outProcessed1");	
			outProcessed[1] = addOutPort<Job>("outProcessed2");
			outProcessed[2] = addOutPort<Job>("outProcessed3");

            auto loadBalancer = addComponent<LoadBalancer>("loadBalancer");
            std::array<std::shared_ptr<Server>, 3> servers;
			servers[0] = addComponent<Server>("server1", processingTimeExpMean);
			servers[1] = addComponent<Server>("server2", processingTimeExpMean);
			servers[2] = addComponent<Server>("server3", processingTimeExpMean);
			addCoupling(inJob, loadBalancer->inJob);
			addCoupling(loadBalancer->outJob[0], servers[0]->inJob);
			addCoupling(loadBalancer->outJob[1], servers[1]->inJob);
			addCoupling(loadBalancer->outJob[2], servers[2]->inJob);
			addCoupling(servers[0]->outProcessed, outProcessed[0]);
			addCoupling(servers[1]->outProcessed, outProcessed[1]);
			addCoupling(servers[2]->outProcessed, outProcessed[2]);
		}
	};
} 

#endif