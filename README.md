# Cheferd

<h3 align="left">
  <!-- logo as of current commit -->
  <img src="https://img.shields.io/badge/C++-17-yellow.svg?style=flat&logo=c%2B%2B" />
  <img src="https://img.shields.io/badge/status-research%20prototype-green.svg" />
  <a href="https://opensource.org/licenses/BSD-3-Clause">
    <img src="https://img.shields.io/badge/license-BSD--3-blue.svg" />
  </a>
</h3>

Cheferd is a storage manager that is able to hollistically orchestrate and manage storage resources.

The current prototype follows hierarchical design, where the controllers have different responsibilities depending on their control level. This prototype is composed by two types of controllers - global and local.

* <b>Global controller:</b> Controller with system-wide visibility and the ability to holistically orchestrate the storage services. Collects monitoring metrics from the system (e.g., workflows’ rate) and enforces new policies to respond to workload variations or new rules set by system administrators.
* <b>Local controller:</b> Manages the data plane stages, serving as a liaison between the global controller and its corresponding data plane stages. Thus, offloading some of the global controller's load. 


The storage manager follows a [Software-Defined Storage](https://dl.acm.org/doi/10.1145/3385896?cid=99659535288) approach, being composed of two main components:
* <b>Data plane (PADLL):</b> The data plane is a multi-stage component that provides the building blocks for differentiating and rate limiting I/O workflows. The data plane can be found at [dsrhaslab/padll](https://github.com/dsrhaslab/padll).
* <b>Control plane (Cheferd):</b> The control plane is a global coordinator that manages all data plane stages to ensure that storage QoS policies are met over time and adjusted according to workload variations. The control plane corresponds to [**this repository**](https://github.com/dsrhaslab/cheferd).

Please cite the following paper if you use Cheferd:

>**Taming Metadata-intensive HPC Jobs Through Dynamic, Application-agnostic QoS Control**.
Ricardo Macedo, Mariana Miranda, Yusuke Tanimura, Jason Haga, Amit Ruhela, Stephen Lien Harrell, Richard Todd Evans, José Pereira, João Paulo.
*23rd IEEE/ACM International Symposium on Cluster, Cloud and Internet Computing (CCGrid 2023)*.

```bibtex
@inproceedings {Macedo2023Padll,
    title     = {Taming Metadata-intensive HPC Jobs Through Dynamic, Application-agnostic QoS Control},
    author    = {Ricardo Macedo and Mariana Miranda and Yusuke Tanimura and Jason Haga and Amit Ruhela and Stephen Lien Harrell and Richard Todd Evans and Jos{\'e} Pereira and Jo{\~a}o Paulo},
    booktitle = {23rd IEEE/ACM International Symposium on Cluster, Cloud and Internet Computing},
    year      = {2023}
}
```

***

## Getting started with Cheferd
 
This tutorial will guide on how to set up and use Cheferd.

### Requirements and Dependencies
Cheferd is written with C++17 and was built and tested with `g++-9.4.0` and `cmake-3.16`.
It uses the following third party libraries, which are installed at compile time: [spdlog v1.8.1](https://github.com/gabime/spdlog) (logging library), [grpc v1.37.0](https://github.com/grpc/grpc) (RPC communication), [asio v1.18.0](https://github.com/chriskohlhoff/asio) (asynchronous programming), [yaml-cpp v0.6.3](https://github.com/jbeder/yaml-cpp) (YAML files parser) and [gflags v2.2.2](https://github.com/gflags/gflags) (command line flags processing).

### Setup Cheferd

```shell
$ cd /path/to/dir   # select the path to clone the Cheferd github repository
$ git clone https://github.com/dsrhaslab/cheferd.git
$ cd cheferd
$ mkdir build; cd build
$ cmake ..; cmake --build .
```

### Using Cheferd 

To deploy a cheferd controller use the following commmand:

```shell
./cheferd_exec  --config_file <path to configuration file>
```
Depending on the configuration following, it assumes certain proprieties. 
Please check the following examples for a global controller and a local controller.

#### Global controller configuration example:
```yaml
controller: core                                                            # Type of controller (core or local)
core_address: 0.0.0.0:50051                                                 # Global controller address
control_type: 1                                                             # Type of control (1-STATIC, 2-DYNAMIC_VANILLA, 3-DYNAMIC_LEFTOVER)
system_limit: 220000                                                        # Setup a storage system limit 
housekeeping_rules_file: ../files/posix_layer_housekeeping_rules_static_op  # Path to housekeeping rules to be implemented
policies_rules_file: ../files/static_rules_with_time_file_job               # Path to policies rules file to be enforced
```

*Housekeeping rules file example:*
```shell
1 create_channel 1000 posix_meta 1000 no_op meta_op
2 create_object 1000 1 posix_meta no_op meta_op drl 10000 100000000
```

*Policies rules file example:*
```shell
1 20 job app1 meta_op 5000     # <rule_id> <time_to_enforce> <app_name> <context> <limit>
2 40 job app1 meta_op 7500                   
3 60 job app1 meta_op 10000
```

#### Local controller configuration example:
```yaml
controller: local                                                           # Type of controller (core or local)
core_address: 0.0.0.0:50051                                                 # Global controller address
local_address: 0.0.0.0:50053                                                # Local controller address
```


## Control Type

#### 1: Static:
Set a job's I/O limits. 

*Policies rules file example:*
```shell
1 10 job app1 getxattr 5000       # Limit app1's getxattr operations to 5000 IOPS
<or>
1 10 job app1 meta_op 5000        # Limit app1's metadata operations to 5000 IOPS
```

*Global controller configuration example:*
```shell
...
control_type: 1
...
```


#### Dynamic:

* <b> 2: Vanilla (Proportional Sharing) </b>

Max-min fair share control algorithm that enforces per-job rate reservations. 


* <b> 3: Dynamic without Leftover (Proportional Sharing without False Allocation) </b>

Proportional sharing algorithm that prevents false resource allocation to ensure storage QoS under volatile workloads.

Rather than assigning resource shares exclusively based on the number of active jobs in the system and their demands, we consider the actual usage (i.e., I/O load) of each job and redistribute resources in a max-min fair share manner based on those observations.

*Policies rules file example:*
```shell
1 0 demand app1 meta_op 30000       # Demand 30000 IOPS for app1's metadata operations
```

*Global controller configuration example:*
```shell
...
control_type: 2 <or> 3
system_limit: 220000 
...
```

***

## Acknowledgments
>We thank the [National Institute of Advanced Industrial Science and Technologies (AIST)](https://www.aist.go.jp/index_en.html)
for providing access to computational resources of [AI Bridging Cloud Infrastructure (ABCI)](https://abci.ai/).
>Work realized within the scope of the project [BigHPC](https://bighpc.wavecom.pt)
(POCI-01-0247-FEDER-045924), European Regional Development Fund, through the Operational Programme for Competitiveness and 
Internationalisation - COMPETE 2020 Programme under the Portugal 2020 Partnership Agreement, and by National Funds through the 
FCT - Portuguese Foundation for Science and Technology, I.P. on the scope of the UT Austin Portugal Program; PhD Fellowships 
SFRH/BD/146059/2019 and PD/BD/151403/2021; and the UT Austin-Portugal Program, a collaboration between the Portuguese Foundation 
of Science and Technology and the University of Texas at Austin, award UTA18-001217.

<p align="center">
    <img src=".media/main_page/fct-logo.png" width="60">
    <img src=".media/main_page/utaustin-portugal-logo.png" width="160">
    <img src=".media/main_page/aist-logo.gif" width="160">
</p>


## Contact
Please contact us at `mariana.m.miranda@inesctec.pt` with any questions.

