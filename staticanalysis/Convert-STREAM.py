from dataclasses import dataclass
import queue
import os
import sys
import re
from typing_extensions import Self
from anytree import AnyNode,Node, RenderTree

def get_parent(arr):
    """
    Takes in any array and returns the last appended value. 
    If the array is empty then it returns None.
    """
    if len(arr)==0:
        return None
    else:
        temp=arr.pop()
        arr.append(temp)
        return temp

#def releventVariables(nodes):
def nodesToTxt(nodes):
    """
    Converts any given list of nodes to text for easier printing
    """
    txt=""
    for i in nodes:
        if len(i.children)!=0:
            txt+=i.id+"\n"
            txt+=nodesToTxt(i.children)
        else:
            txt+=i.id+"\n"
    return txt

def findFuncName(line):
    name=''
    if(re.search(".*\=.*\(.*\)\;", line)):
        name=line[line.find('=')+1:line.find('(')]

    elif(re.search(".*\(.*\)\;", line)):
        name=line[0:line.find('(')]
    return name.strip()


def findFuncs(nodes):
    here = os.path.dirname(os.path.realpath(__file__))
    for i in nodes:
        if len(i.children)!=0:
            findFuncs(i.children)
        elif i.type=="funcCall":
            func=findFuncName(i.id)
            func+=".cpp"
            for path, subdirs, files in os.walk(os.path.join(here,"codes")):
                for name in files:
                    if name.strip() ==func:
                        code=getCode(name)
                        code_1=clean_code(code)
                        childnodes=transform_code(code_1)
                        for j in childnodes:
                            if j.parent is None:
                                for pre, fill, node in RenderTree(j):
                                    print("%s%s" % (pre, node.id))
                                j.parent=i
        writeToFile(nodes)

                                
                            




def writeToFile(nodes):
    """
    Writes to the file given the nodes
    """
    here = os.path.dirname(os.path.realpath(__file__))
    filename = "X.c"
    filepath = os.path.join(here, filename)
    try:
        ex = open(filepath, 'w')
        txt=nodesToTxt(nodes)
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
    forCalls=[]
    check=False
    for i in nodes:
        if i.type=="forCall":
            print(i.id)
            for j in i.children:
                if j.type=="funcCall":
                    print(j.id)
                    check=True
                    break;
            
            if check:
                forCalls.append(i)

        check=False
    print(len(forCalls))
    return forCalls


def transform_code(code):
    
    """
    Takes string code and converts it into a tree based on curly brackets.
    Uses the AnyTree Library to create tree and stores 2 extra variables
       src: contains the name of the function being called
       type: contains the type of line of code
    """
    parents=[]
    nodes=[]
    for i in code.splitlines():
        i=i.strip()
        if i.endswith(";"):
            t,n=findPurpose(i)
            if t!="if-print":
                temp= AnyNode(id=n,parent=get_parent(parents),src=i,type=t)
        elif i.endswith("{"):
            t,n=findPurpose(i)
            temp= AnyNode(id=n,parent=get_parent(parents),src=i,type=t)
            parents.append(temp)
        elif i.endswith("}"):
            t,n=findPurpose(i)
            temp= AnyNode(id=n,parent=get_parent(parents),src=i,type=t)
            parents.pop()
        nodes.append(temp)

    return nodes

def findPurpose(line):
    """
    Takes any line of code and tries to intrepret its purpose while seperating any important names
    """
    line=line.strip()
    if(re.search(".*\s.*\(.*\);", line)):
        #print(line[line[:line.index('('):].rfind(' '):line.index('('):])
        if(line.startswith("if")|line.startswith("else if")|line.startswith("else")):
           return "ifCallno",line
        elif(line.startswith("for")):
            return "forCall",line
        else:
            #return "funcCall",line[line[:line.index('('):].rfind(' '):line.index('('):]
            return "funcCall",line
    elif(re.search(".*\(.*\);", line)):
        if(line.startswith("if")|line.startswith("else if")|line.startswith("else")):
           return "ifCall",line
        elif(line.startswith("for")):
            return "forCall",line
        else:
            return "funcCall",line
    elif(re.search(".*\(.*\).*{", line)):
        if(line.startswith("if")|line.startswith("else if")|line.startswith("else")):
           return "ifCall",line
        elif(line.startswith("for")):
            return "forCall",line
        elif(line.startswith("struct")):
            return "dataStrcture",line
        return "func",line[:line.index('('):]
    elif(line=="else {"):
        return "ifCall",line
    elif(re.search("int.* = [0-9]+\;",line)):
        return "instantInt",line
    elif(re.search("int.* = .*\;",line)):
        return "instantVar",line
    elif(re.search("if.*\(.*\).*_fout.*\;", line)):
        # finds if statements only for printing
        return "if-print",line
    else:
        return "idk",line

def getCode(filename):
    code=""
    with open(os.path.join(sys.path[0],filename), "r") as f:
        temp=f.read()
        code+=temp
    return code

def clean_code(code):
    """
    Removes all forms of comments from the code and returns a string of the same code without comments or imports
    """
    formatted_code=''
    includes=[]
    newline_com=False
    for i in code.splitlines():
        i=i.strip()
        #print('~~~~'+i+'~~~~')
        if not(newline_com):
            if i.startswith('//'):
                pass
            elif i.startswith('/*'):
                newline_com=True
                if i.endswith('*/'):
                    newline_com=False
            elif i.startswith('#'):
                if i.startswith('#include'):
                    includes.append(i.split(" ")[1])
            else:
                if len(i.strip())==0:
                    pass
                elif i.endswith(';') or i.endswith('{') or i.endswith('}'):
                    formatted_code+=i+'\n'
                elif "//" in i:
                    formatted_code+=i[:i.index("//")]+'\n'
                elif i.startswith("using"):
                    pass
                else:
                    formatted_code+=i+" "

    


        else:
            if i.endswith('*/'):
                newline_com=False
    return formatted_code


if __name__ == '__main__':
    code2 =''; segments=[]; totalLine=''; funcList=[];
    code=getCode("x.cpp")
    code_1=clean_code(code)
    nodes=transform_code(code_1)
    forCalls=releventIterations(nodes)
    findFuncs(forCalls)
    writeToFile(forCalls)



    # for i in forCalls:
    #     for pre, fill, node in RenderTree(i):
    #         print("%s%s%s" % (pre, node.id,node.type))

    # for i in forCalls:
    #     if i.parent is None:
    #         for pre, fill, node in RenderTree(i):
    #             print("%s%s%s" % (pre, node.id,node.type))
    
    
