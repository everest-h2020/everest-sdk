Traffic Prediction Examples
=================================


In this folder, there are four examples how to use the EVEREST SDK to accelerate ML inference by the example of traffic prediction using a deep neuronal network (DNN). Also, the user story tells how different user personas work together and using different parts of the SDK.


Example Jupyter Notebooks
------------------------------

### `01_traffic_prediction_full_flow.ipynb`

In this notebook, the interaction of the ML developer with the ML inference flow is shown. It includes the development of a small DNN and its export to ONNX. It also showcases how the ML-developer and the performance engineer can see performance predictions and how the performance engineer could optimize architectural details.


### `02_traffic_prediction_version_generation.ipynb`

This notebook shows how the ML-developer, Workflow-specialist, and/or Infrastructure-specialist interacts with the basecamp tool to bundle different variants of the (accelerated) application together.


### `03_traffic_prediction_quantized_flow.ipynb`

In this example, the additional (experimental)  quantization and calibration support of the Everest ML inference flow is shown, based on a pytorch model.


## `04_traffic_prediction_airflow_api.ipynb`

The last example is a brief demo of how to use the integrated airflow API of the basecamp tool by the Workflow-specialist.



Traffic prediction  user story
-------------------------------------

The user story comprises:
- Minor (the goal is minimalistic) adaptation of the original code
- Identify and mark part of the code (python function) to be outsourced to FPGA
- Application of SDK tools to compile application in a heterogeneous way with suitable parameters
- Deploy the application on a heterogeneous platform using cloud FPGA or Alveo FPGA so that it fit the workflow of the target application
- Execute application and measure its performance


The user story involves the following personas:

### ML-developer

Skills:
- pytorch / tensor-flow
- app development
- HW-awareness

Story:
Gets the job to develop an app that uses an AI-model to predict future congestion on each road segment of a city. (S)he trains a small model for each individual road segment. For a mid-size European city, this leads to few 10000 models which all have the same architecture but different weights. S(h)e integrates all models into an app, which predicts congestion in the near future (up to 1hr ahead) for all road segments of the respective city. After delivering the app to the workflow specialist, (s)he receives a note telling him that his app is several orders of magnitude too slow (by the time the app returns a result, the prediction is no longer valid). 

(S)he finds that re-loading the model 10000s of times destroys performance and also learns from the workflow specialist that their infrastructure offers FPGA devices for acceleration. (S)he changes the application by exploiting the fact that the model architecture remains unchanged and annotates some performance-critical components in the code. The next step is to use the FPGA-aware compilation flow to produce an accelerated version of his app. (S)he downloads the EVEREST SDK and uses it to compile his annotated code for FPGA-accelerated heterogeneous infrastructure. 
(S)he runs into some issues and involves a performance engineer with FPGA expertise to address them. Communication with the performance engineer is simple because they both work on a common Jupyter notebook. The result is an optimized version of the app.

The ML-developer now includes the build-flow in his standard CI-CD pipelines and adjusts a few parameters of the app to meet the updated specs (s)he receives from the workflow specialist. The automation allows him to make those adjustments without involving the performance specialist. After verification, (s)he hands off the new version to the workflow specialists who can now meet performance targets. 


### Workflow specialist (app deployment and integration)

Skills:
- understands end-user-application performance targets
- masters the workflow tools incl. deployment and profiling
- infrastructure aware

Story:
Gets the job to deploy an application that identifies the shortest possible path through a large network of roads for 10'000s of requests. Sets up the workflow including an app that runs inference on an AI-model, which (s)he gets from the ML developer. Sets up the end-to-end workflow and realizes that the inference task is orders of magnitude too slow for his requirements. Meets with the ML-developer and finds that the FPGAs available in his infrastructure portfolio might be able to provide substantial speed-up for the slow ML-inference app in his workflow. After some time, (s)he receives a FPGA-accelerated version of the app from the ML-developer, Downloads the EVEREST SDK and uses the run-time components to re-deploy the FPGA-accelerated app on the target infrastructure. After some adjustments in collaboration with the infrastructure specialist, (s)he finally meets performance targets.

### Performance engineer

Skills:
- detailed knowledge of EVEREST SDK components involved in compiling to FPGA-accelerated heterogeneous infrastructure
- detailed knowledge of heterogeneous FPGA-CPU platforms

Story:
Understands the details of the EVEREST SDK components required to compile ML applications to FPGA-accelerated heterogeneous infrastructure. Analyses the requirements of the ML developer. Based on this analysis, adjusts and optimizes the necessary configuration for the compilation tool chain. Reaches out to the maintainers of the EVEREST SDK to address some error messages. 

### Infrastructure specialist

Skills:
- understands heterogeneous (cloud) infrastructure
- knows the run-time components of the EVEREST SDK needed to debug apps deployed on FPGA-accelerated heterogeneous infrastructure

Story:
Gets a request from the workflow specialist and supports him in the debugging and optimization of an ML-inference app on the target infrastructure of the EVEREST SDK. 


