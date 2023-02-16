#ifndef JOB
#define JOB

#include <iostream>

namespace cadmium::loadbalancer {
	struct Job {
		int id;
		double timeGenerated;
		double timeProcessed;

		Job(int id, double timeGenerated): id(id), timeGenerated(timeGenerated), timeProcessed(-1) {};
		Job(int id) {};
	};

	std::ostream& operator<<(std::ostream& out, const Job& j) {
		out << "{" << j.id << "," << j.timeGenerated << "," << j.timeProcessed << "}";
		return out;
	}
}

#endif 