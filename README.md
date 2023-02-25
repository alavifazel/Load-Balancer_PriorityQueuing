## Load-balancer with priority queuing
This repository contains the code to model a load-balancing system (with priority queue scheduling) using the DEVS formalism and with [Cadmium](https://github.com/SimulationEverywhere/cadmium_v2) library.

### Build instruction
To build the project, first clone the repository:

```
git clone git@github.com:alavifazel/Load-Balancer_PriorityQueuing.git
```

Afterwards, run:

```
cd Load-Balancer_PriorityQueuing/
cmake .
make all
```

The `cmake` script present in the project takes care of obtaining the latest version of Cadmium (v2) and building a the appropriate *Makefile*. After successful compilation, the main program can be run via:

```
./bin/main <GENERATION_PERIOD> <PROCESSING_TIME_EXP_MEAN> <OBSERVATION_TIME>
```

For instance:

```
./bin/main 2 5 1000
```

This example runs the simulation with job generation period of 2 time units, each server having the processing time with mean value of 5, and for 1000 time units. Running this example produces:

```
...
End time: 1046.06
Jobs generated: 501
Jobs processed: 501
Average Processing Time: 5.0685
Throughput: 0.478939
```


### Test programs
After compilation, the `bin` folder will contain four additional programs to test each atomic model individually. These programs (with their corresponding input data) can be run with:

#### Generator
`` ./bin/test_generator input_data/generator_test_input.txt``

#### Balancer
``./bin/test_balancer input_data/balancer_test_input.txt``

#### Server
``./bin/test_server input_data/server_test_input.txt``

#### Transducer
``./bin/test_transducer input_data/transducer_test_input_generator.txt input_data/transducer_test_input_processed.txt``

After executing the programs, log files (CSV) will be generated which contain the details about the test runs. 
