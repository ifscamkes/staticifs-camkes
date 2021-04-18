from camkes.ast import Instance, Connection, Component, Uses, Provides, IfcPolicy
import ctypes
import sys
import networkx as nx
import matplotlib.pyplot as plt
from collections import defaultdict

global_id = 1
component_list = dict()
connections_list = dict()
interfaces_list = dict()

logging_file = "ifc_policy_log"

def get_key(val, my_dict): 
    for key, value in my_dict.items(): 
         if val == value[1]: 
             return key 

def DFSUtil(tc, graph, s, v):
    tc[s][v] = 1

    for i in graph[v]:
        if tc[s][i]==0:
            DFSUtil(tc, graph, s, i)

def transitiveClosure(edgeList, vertices):
    """Returns transitive closure matrix of a graph."""
    """edgeList doesn't contain (i,i)."""
    graph = defaultdict(list)
    tc = [[0 for j in range(vertices)] for i in range(vertices)]

    for src, dst in edgeList:
        graph[src].append(dst)

    for i in range(vertices):
        DFSUtil(tc, graph, i, i)
    return tc

def print_list (list_obj, list_name):
    """Prints the list contents with its list names """
    file = open(logging_file, 'a+')
    file.write ("\n"+list_name+"\n\n")
    for key, value in list_obj.items():
        file.write ("(" + str(key) + " -> " + str(value) +")\n" )
    file.close()

def CheckIfc(assembly, ast):
    tcAccessControlMatrix = generate_adjacency_control_matrix(assembly)
    ifcPolicyMatrix = get_ifcpolicy_rules(ast)
    if ifcPolicyMatrix is not None:
        print_graph(tcAccessControlMatrix, ifcPolicyMatrix)

def generate_adjacency_control_matrix(assembly):
    """Finds the component names and connections and \
    calculate the adjacency matrix."""
    global global_id

    """Finding and saving the component names which will \
    act as subjects. Format for saving (Name->(Object, ID, Type))"""
    for name, obj in assembly.composition._mapping.items():
        if isinstance(obj, Instance) and name.encode("ascii")!='rwfm_monitor':
            component_list[name.encode("ascii")] = (obj.type, global_id, type(obj.type))
            global_id = global_id + 1
    
    print_list(component_list, "component_list")

    """From here on creating access matrix and transitive closure."""
    number_of_subjects = len(component_list)
    access_control_matrix = [[0 for i in range(number_of_subjects)] \
                            for j in range(number_of_subjects)]
    edgeList = list()

    for i in range(number_of_subjects):
        access_control_matrix[i][i] = 1

    for interfaces in assembly.composition.connections:
        for from_end in interfaces.from_ends:
            if from_end._instance._name.encode("ascii") != 'rwfm_monitor' \
               and from_end._parent._to_ends[0]._instance._name.encode("ascii") != 'rwfm_monitor':
                row_id = component_list[from_end._instance.name.encode("ascii")][1]
                column_id = component_list[from_end._parent._to_ends[0]._instance._name.encode("ascii")][1]
                if interfaces._type._name.encode("ascii") == "seL4RPCCall":
                    edgeList.append(tuple((row_id-1, column_id-1)))
                    edgeList.append(tuple((column_id-1, row_id-1)))
                    access_control_matrix[row_id-1][column_id-1] = \
                            access_control_matrix[column_id-1][row_id-1] = 1
                else:
                    edgeList.append(tuple((row_id-1, column_id-1)))
                    access_control_matrix[row_id-1][column_id-1] = 1
                interfaces_list[from_end] = (from_end.interface.name.encode("ascii"), 
                                            global_id, 
                                            type(from_end.interface), 
                                            from_end.instance.name.encode("ascii"))
                global_id = global_id + 1

        for to_end in interfaces.to_ends:
            if to_end._instance._name.encode("ascii") != 'rwfm_monitor' \
               and to_end._parent._from_ends[0]._instance._name.encode("ascii") != 'rwfm_monitor':
                interfaces_list[to_end] = (to_end.interface.name.encode("ascii"), 
                                            global_id, 
                                            type(to_end.interface),
                                            to_end.instance.name.encode("ascii"))
                global_id = global_id + 1

    print_list (interfaces_list, "interface_list")
    print("AccessControlMatrix:"+str(access_control_matrix))
    tcAccessControlMatrix = transitiveClosure(edgeList, number_of_subjects)
    print("TransitiveAccessControlMatrix:"+str(tcAccessControlMatrix))
    return tcAccessControlMatrix

def generate_Ifc_policy_matrix(ifcpolicy):
    """Generate access matrix from user given ifc policy."""
    edgeList = list()
    vertices = len(component_list)
    ifcPolicyMatrix = [[0 for j in range(vertices)] for i in range(vertices)]

    for i in range(vertices):
        ifcPolicyMatrix[i][i] = 1

    for defn in ifcpolicy.definitions:
        src = defn.srcComponent.name.encode("ascii")
        dst = defn.dstComponent.name.encode("ascii")

        row_id = component_list[src][1]
        column_id = component_list[dst][1]
        ifcPolicyMatrix[row_id-1][column_id-1] = 1
        edgeList.append(tuple((row_id-1, column_id-1)))
        
    tcIfcPolicyMatrix = transitiveClosure(edgeList, vertices)

    print ("IfcPolicyMatrix:"+str(ifcPolicyMatrix))
    print ("TransitiveIfcPolicyMatrix:"+str(tcIfcPolicyMatrix))
    newFlows = list()
    for i in range(vertices):
        for j in range(vertices):
            if ifcPolicyMatrix[i][j] != tcIfcPolicyMatrix[i][j]:
                #newFlows.append(tuple((i+1, j+1)))
                newFlows.append(tuple((get_key(i+1, component_list), \
                        get_key(j+1, component_list))))

    if len(newFlows)>0:
        print ("Ifc policy is inconsistent."+str(newFlows))
        sys.exit("Ifc policy is inconsistent") 
    else:   print("Policy is consistent.")

    return ifcPolicyMatrix

def get_ifcpolicy_rules(ast):
    for item in ast._items:
        if isinstance(item, IfcPolicy):
            return generate_Ifc_policy_matrix(item)

def print_graph(tcAccessControlMatrix, ifcPolicyMatrix):
    """Create the flow graph and saves it."""

    redFlows = list()
    number_of_nodes = len(component_list)

    DG = nx.MultiDiGraph()
    for node in component_list:
        DG.add_node(node)
    
    for start in component_list:
        for end in component_list:
            row = component_list[start][1]
            column   = component_list[end][1]
            if tcAccessControlMatrix[row-1][column-1] == 1 and \
                    ifcPolicyMatrix[row-1][column-1] == 1:
                DG.add_edge(start, end, color='green')
            if tcAccessControlMatrix[row-1][column-1] == 1 and \
                    ifcPolicyMatrix[row-1][column-1] == 0:
                DG.add_edge(start, end, color='red')
                redFlows.append(tuple((get_key(row, component_list), \
                        get_key(column, component_list))))
            if tcAccessControlMatrix[row-1][column-1] == 0 and \
                    ifcPolicyMatrix[row-1][column-1] == 1:
                DG.add_edge(start, end, color='blue')
    
    edges = DG.edges()
    colors = []
    for (u,v,attrib_dict) in list(DG.edges.data()):
        colors.append(attrib_dict['color'])

    nx.draw(DG, node_color = 'Y', node_size=2000, \
            edges=edges, edge_color=colors, with_labels=True)
    plt.savefig("checkIfc.png")
    A = nx.nx_agraph.to_agraph(DG)
    A.write('graph.dot') # write it into a dot file. To open, use graphviz
    #plt.show()
    
    print (DG.nodes()) #print nodes in graph
    print (edges) #print edges in graph
    print ("Existing flows in the system are inconsistent with the IFC Policy.\
            Inconsistent flows are:"+str(redFlows))
    if len(redFlows)>0: sys.exit("Existing flows in the system are \
            inconsistent with the IFC Policy.")
