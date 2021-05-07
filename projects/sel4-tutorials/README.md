All static IFS-CAmkES examples are available at: staticifs-camkes/projects/sel4-tutorials/tutorials

## To run a project:
1. Change the example folder name to hello-camkes-1
2. In staticifs-camkes directory, run the following commands:

   a. mkdir <project_name>
   
   b. cd <project_name>
   
   c. ../init --plat pc99 --tut hello-camkes-1

The generated information flow policy diagram is available (graph.dot) in <project_name> folder.
   
Here is a brief overview of the examples: 
1. hello-camkes-1-rpc-call: Demonstrates RPC Call connection example with 3 components.
2. hello-camkes-1-rpc: Demonstrates RPC connection example with 3 components. 
3. hello-camkes-1-4-components-incons-ifcp: Demonstrates inconsistent information flow control policy example with 4 components.  
4. hello-camkes-1-4-components-incons-con: Demonstrates inconsistent connections (with respect to information flow control policy) example with 4 components.
