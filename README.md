## Load-balancer with priority queuing
The repository contains the code to model a load-balancing system (with priority queue scheduling) using the DEVS formalism and with [Cadmium](https://github.com/SimulationEverywhere/cadmium_v2) library.

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

This example runs the simulation with job generation period of 2 time units, each server having the processing time with mean value of 5, and for the duration of 1000 time units. Running this example produces:

```
...
End time: 1046.06
Jobs generated: 501
Jobs processed: 501
Average Residence Time: 14.702
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

### Simulation result
The result for different job arrival periods with fixed mean processing time, 2, is shown below:

| **Arrival period** | **Throughput** | **Residence time** |
|:------------------:|----------------|--------------------|
|         10         |     0.1016     |        2.090       |
|          5         |     0.2000     |        2.059       |
|          1         |     0.9924     |        3.473       |
|         0.7        |     1.4168     |       17.762       |
|         0.5        |     1.4991     |       314.11       |
|         0.1        |     1.4618     |       2802.25      |
|        0.05        |     1.4818     |       6136.41      |

The result shows that by increasing the job arrival rate, the throughput increases until the servers become bottleneck and hit a certain limit. During this process, the residence time also increases (starting before the servers become bottleneck, due to the nature of exponentially distributed process time). The throughput limit in our simulation turned out to be around 1.5 which matches with the result of utilization law in queuing theory. This law states that the saturation throughput in an open-system is calculated by the reciprocal of the total system demand. Mathematically:

$$
\lambda_{sat} = \frac{1}{D_{max}}
$$

In the example runs, the mean processing time of each server was chosen to be 2. Since the jobs visit each server one third of the times on average, the service demand of each server is (1/3)*2 = 2/3. Hence, the saturation throughput is calculated as:


$$
\lambda_{sat} = \frac{3}{2} = 1.5
$$

Which was identical to the one that was observed in our simulation.
