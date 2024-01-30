from dataclasses import dataclass
import queue
import os
import sys
import re
from pathlib import Path
from typing_extensions import Self
from anytree import AnyNode, Node, RenderTree , AsciiStyle, PreOrderIter


def get_parent(arr):
    """
    Takes in any array and returns the last appended value. 
    If the array is empty then it returns None.
    """
    if len(arr) == 0:
        return None
    else:
        temp = arr.pop()
        arr.append(temp)
        return temp

# def releventVariables(nodes):


def nodesToTxt(nodes):
    """
    Converts any given list of nodes to text for easier printing
    """
    txt = ""
    for i in nodes:
        i.id = i.id.strip()
        if len(i.children) != 0:
            txt += i.id+"\n"
            txt += nodesToTxt(i.children)
        else:
            txt += i.id+"\n"

    return txt


def findFuncName(line):
    name = ''
    if (re.search(".*\=.*\(.*\)\;", line)):
        name = line[line.find('=')+1:line.find('(')]

    elif (re.search(".*\(.*\)\;", line)):
        name = line[0:line.find('(')]
    return name.strip()


def findFuncs(nodes):
    here = os.path.dirname(os.path.realpath(__file__))
    for i in nodes:

        if i.type == "funcCall":
            func = findFuncName(i.id)
            func += ".cpp"
            found = False
            for path, subdirs, files in os.walk(os.path.join(here, "multifiles_HPCG")):
                for name in files:
                    if name.strip() == func:
                        code = getCode(name)
                        code_1 = clean_code(code)
                        childnodes = transform_code(code_1)
                        found = True
                        for j in childnodes:
                            if j.parent is None:
                                a = j.src[j.src.index(
                                    "(")+1:j.src.index(")")].split(",")
                                b = i.src[i.src.index(
                                    "(")+1:i.src.index(")")].split(",")
                                for val in range(len(b)):
                                    instantiation = a[val]+" = "+b[val]+" ;"
                                    AnyNode(id=instantiation, parent=i,
                                            src=instantiation, type="instantVar")
                                # for val in j.children:
                                #     va=val.id.strip()
                                #     if va!="return":
                                #         i.children.append(j)

                                i.children = i.children+j.children[:-2]
                                # i.children.extend(j.children)

                        # for j in childnodes:
                        #     if j.parent is None:
                        #         for pre, fill, node in RenderTree(j):
                        #             print("%s%s" % (pre, node.id))
                        #         j.parent=i
            if not found:
                print("did not find func:"+func)

        if len(i.children) != 0:
            findFuncs(i.children)

    return nodes
    # writeToFile(nodes)


def writeToFile2(txt, filename):
    """
    Writes to the file given the nodes
    """
    here = os.path.dirname(os.path.realpath(__file__))
    filepath = os.path.join(here, filename)
    try:
        ex = open(filepath, 'a+')
        ex.write(txt+"\n")
        ex.close()
    except IOError:
        print("Wrong path provided")


def writeToFile(txt):
    """
    Writes to the file given the nodes
    """
    here = os.path.dirname(os.path.realpath(__file__))
    filename = "HPCG.c"
    filepath = os.path.join(here, filename)
    try:
        
        ex = open(filepath, 'a+')
        ex.write(txt+"\n")
        ex.close()
    except IOError:
        print("Wrong path provided")


def releventIterations(nodes):
    """
    Takes all possible nodes and tries to figure out the relevant nodes.
    Picks for loops on the basis of if they are calling any relevant functions
    outputs a list of relevant for loops
       nodes: contains all the nodes in the tree


    """
    forCalls = []
    check = False
    for i in nodes:
        if i.type == "forCall":

            for j in i.children:
                if j.type == "funcCall":

                    check = True
                    break

            if check:
                forCalls.append(i)

        check = False

    return forCalls


def transform_code(code):
    """
    Takes string code and converts it into a tree based on curly brackets.
    Uses the AnyTree Library to create tree and stores 2 extra variables
       src: contains the name of the function being called
       type: contains the type of line of code
    """
    parents = []
    nodes = []
    for i in code.splitlines():
        i = i.strip()

        if i.endswith(";"):
            if ";" in i[:-1] and not (i.startswith("for")):
                r = i.split(";")
                for j in r:
                    if len(j.strip()) > 1:
                        t, n = findPurpose(j+";")

                        if t != "if-print" and t != "times":

                            temp = AnyNode(id=n, parent=get_parent(
                                parents), src=j+";", type=t)

            else:
                t, n = findPurpose(i)
                if t == "ifCall":
                    for k in kernels:
                        j = i
                        if k[0] in j:
                            ind = j.index(k[0])
                            temp = AnyNode(
                                id=j[:ind-1], parent=get_parent(parents), src=j[:ind-1], type=t)
                            temp2 = AnyNode(
                                id=j[ind:], parent=temp, src=j[ind:], type="funcCall")

                if t != "if-print" and t != "times":
                    temp = AnyNode(id=n, parent=get_parent(
                        parents), src=i, type=t)
        elif i.endswith("{"):
            if i.startswith("}"):
                t, n = findPurpose(i)
                temp = AnyNode(id="}", parent=get_parent(
                    parents), src="}", type=t)
                parents.pop()
                i = i[2:]
            t, n = findPurpose(i)
            temp = AnyNode(id=n, parent=get_parent(parents), src=i, type=t)
            parents.append(temp)
        elif i.endswith("}"):
            t, n = findPurpose(i)
            temp = AnyNode(id=n, parent=get_parent(parents), src=i, type=t)
            parents.pop()
        nodes.append(temp)

    return nodes


def findPurpose(line):
    """
    Takes any line of code and tries to intrepret its purpose while seperating any important names
    """
    line = line.strip()
    if line.startswith("TICK") or line.startswith("TOCK") or line.startswith("times") or line.startswith("time") or line.endswith(" mytimer();") or "print_freq" in line:
        return "times", line
    elif line.startswith("MPI"):
        return "MPI", line
    elif line.startswith("else"):
        return "ifCall", line
    elif (re.search(".*\s.*\(.*\);", line)):
        # print(line[line[:line.index('('):].rfind(' '):line.index('('):])
        if (line.startswith("if") | line.startswith("else if") | line.startswith("else ")):
            return "ifCall", line
        elif (line.startswith("for")):
            return "forCall", line
        else:
            # return "funcCall",line[line[:line.index('('):].rfind(' '):line.index('('):]
            return "funcCall", line
    elif (re.search(".*\(.*\);", line)):
        if (line.startswith("if") | line.startswith("else if") | line.startswith("else")):
            return "ifCall", line
        elif (line.startswith("for")):
            return "forCall", line
        else:
            return "funcCall", line
    elif (re.search(".*\(.*\).*{", line)):
        if (line.startswith("if") | line.startswith("else if") | line.startswith("else")):
            return "ifCall", line
        elif (line.startswith("for")):
            return "forCall", line
        elif (line.startswith("struct")):
            return "dataStrcture", line
        return "func", line[:line.index('('):]
    elif (line == "else {"):
        return "ifCall", line
    elif (re.search("int.* = [0-9]+\;", line)):
        return "instantInt", line
    elif (re.search("int.* = .*\;", line)):
        return "instantVar", line
    elif (re.search("if.*\(.*\).*_fout.*\;", line)):
        # finds if statements only for printing
        return "if-print", line
    else:
        return "idk", line


def getCode(filename):
    code = ""
    with open(os.path.join(sys.path[0], "multifiles_HPCG", filename), "r") as f:
        temp = f.read()
        code += temp
    return code


def find_kernel(kernel, func,output):
    if output=="Bool":
        for i in kernel:
            if i[0] in func.src:
                return True
        return False
    else:
        for i in kernel:
            if i[0] in func.src:
                return i
        
def extract_exec(src,name):
    if name[1]=="E":
        ind=src.find(name[0])
        src2=src[ind+len(name[0])+1:src.find(")")]
        vars=src2.split(",")
        txt = ""
        for i in vars:
            txt += "-D "+str(i) + " 100 "

        comp = "DisCosTiC->Exec(\"FILE:"+name[0] + \
            ".c//BREAK:COPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//"+txt+"\", recv);"
        
        return comp

    return ""

def selected_print(nodes, num,kernels):
    path = Path(__file__).parent.absolute()
    f = open(os.path.join(path,  "HPCG-rewrite.c"), "w")
    resume=True
    par=None
    for node in PreOrderIter(nodes):
        if resume:
            if find_kernel(kernels, node,"Bool"):
                k=find_kernel(kernels, node,"Val")
                exec=extract_exec(node.src,k)
                if k[1]=="E":
                    f.write(exec+"\n")
                    
                    if node.is_leaf:
                        pass
                    else:
                        resume=False
                        par=node
                elif k[1]=="M":
                    f.write(exec+"\n")
                elif k[1]=="L":
                    if node.is_leaf:
                        pass
                    else:
                        resume=False
                        par=node
            else:
                f.write(node.src+"\n")
                    
        else:
            if par in node.ancestors:
                if par!="ComputeMG_ref":
                    if find_kernel(kernels, node,"Bool"):
                        print(node.src)
                    else:
                        print(node.src)
                
            else:
                resume=True
                for i in kernels:
                    if i[0] in node.src and i[1] != "L":
                        exec=extract_exec(node.src,i)
                        f.write(exec+"\n")
                        if node.is_leaf:
                            break
                        resume=False
                        par=node
                        break
                else:
                    f.write(node.src+"\n")
                    
                    continue
                continue

def clean_code(code):
    """
    Removes all forms of comments from the code and returns a string of the same code without comments or imports
    """
    formatted_code = ''
    includes = []
    newline_com = False
    for i in code.splitlines():
        i = i.strip()
        # print('~~~~'+i+'~~~~')
        if not (newline_com):
            if i.startswith('//'):
                pass
            elif i.startswith('/*'):
                newline_com = True
                if i.endswith('*/'):
                    newline_com = False
            elif i.startswith('#'):
                if i.startswith('#include'):
                    includes.append(i.split(" ")[1])
            else:
                if len(i.strip()) == 0:
                    pass
                elif i.endswith(';') or i.endswith('{') or i.endswith('}'):
                    formatted_code += i+'\n'
                elif "//" in i:
                    formatted_code += i[:i.index("//")]+'\n'
                elif i.startswith("using"):
                    pass
                else:
                    formatted_code += i+" "

        else:
            if i.endswith('*/'):
                newline_com = False
    return formatted_code


def funcCode(node, here):
    func = findFuncName(node.id)
    func += ".cpp"
    found = False
    for path, subdirs, files in os.walk(os.path.join(here, "multifiles_HPCG")):
        for name in files:
            if name.strip() == func:
                code = getCode(name)
                code_1 = clean_code(code)
                childnodes = transform_code(code_1)
                found = True
                for j in childnodes:
                    if j.parent is None:
                        a = j.src[j.src.index(
                            "(")+1:j.src.index(")")].split(",")
                        b = node.src[node.src.index(
                            "(")+1:node.src.index(")")].split(",")
                        for val in range(len(b)):
                            instantiation = a[val]+" = "+b[val]+" ;"
                            AnyNode(id=instantiation, parent=node,
                                    src=instantiation, type="instantVar")
                        node.children = node.children+j.children[:-2]
                        return True, j
                        
    if not found:
        print("did not find func:"+func)
        return False,""


class data:
    nodelist = []
    notlist=[]

    def __init__(self):
        self.nodelist = []
        self.notlist = []
        # return self.nodelist

    def add(self, name, node):
        self.nodelist.append([name, node])

    def exists(self, name):
        for i in self.nodelist:
            if i[0] in name:
                return True
        return False

    def find(self, name):
        for i in self.nodelist:
            if i[0] in name:
                return i[1]
        return False

def cleanup():
    file1 = open('HPCG.c', 'w')
    with open('Content//top.txt','r') as firstfile:
      


      
    # read content from first file
       
        for line in firstfile:
                
                # append content to second file
                file1.write(line)
    
    # Using readlines()
    file2 = open('HPCG-rewrite.c', 'r')
    Lines = file2.readlines()
    
    count = 0
    # Strips the newline character
    for line in Lines:
        if line.strip()=="if (ierr!=0) return ierr;":
            pass
        else:
            file1.write(line)
    
    with open('Content//bottom.txt','r') as firstfile:
        
    # read content from first file
        for line in firstfile:
                
                # append content to second file
                file1.write(line)

        

def finalize(nodes, kernels, a):
    here = os.path.dirname(os.path.realpath(__file__))

    for i in nodes:
        if i.is_leaf and i.type == "funcCall":
            func = findFuncName(i.id)
            if a.exists(func):
                #print("i am in if with already exist:" +i.id)
                for k in kernels:
                    if k[0]==func and k[1]=="M":
                        print("expanded: ",func)
                        node=a.find(func)
                        i.children = i.children+node.children[:-2]


            else:
                #print("i am in if with new:" +i.id)
                b,n=funcCode(i, here)
                if b:
                    a.add(func,n)
                    finalize(nodes, kernels, a)
                else:
                    i.parent=None
                
        elif not i.is_leaf:
            #print("i am in elif:" +i.id)
            finalize(i.children, kernels,a)

    return nodes



if __name__ == '__main__':
    a = data()
    kernels = (["ComputeSPMV_ref","L"], ["ComputeWAXPBY_ref","E"], ["ComputeDotProduct_ref","E"], ["ComputeMG_ref","M"],
               ["ComputeProlongation_ref","E"], ["ComputeSYMGS_ref","E"], ["ComputeRestriction_ref","E"])
    if os.path.exists("HPCG.c"):
        os.remove("HPCG.c")
    if os.path.exists("HPCG2.c"):
        os.remove("HPCG2.c")
    code2 = ''
    segments = []
    totalLine = ''
    funcList = []
    code = getCode("main.cpp")
    code_1 = clean_code(code)
    nodes = transform_code(code_1)
    forCalls = releventIterations(nodes)
    forCall = []
    forCall.append(forCalls[3])
    nodes = finalize(forCall, kernels, a)

    for i in nodes:
        for pre, fill, node in RenderTree(i):
            print("%s%s%s" % (pre, node.id, node.type))
    
    selected_print(nodes[0], 1,kernels)
    cleanup()
