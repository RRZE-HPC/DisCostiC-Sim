from dataclasses import dataclass
import queue
import os
import sys
import re
from typing_extensions import Self
from anytree import AnyNode,Node, RenderTree

def deINIT(sentence):
    
    arr= sentence.split(" ")
    sen="";
    for i in arr:
        if i is "=":
            break;
        else:
            sen+=" "+i
    return sen+";"

def findBTWmarkers(mark1,mark2,sampleStr):
    
    try :
        # here ; and / are our two markers 
        # in which string can be found. 
        marker1 = mark1
        marker2 = mark2
        regexPattern = marker1 + '(.+?)' + marker2
        str_found = re.search(regexPattern, sampleStr).group(1)
        if("->" in str_found):
            return None
        else:
            return str_found
    except AttributeError:
        # Attribute error is expected if string 
        # is not found between given markers
        str_found = 'Nothing found between two markers'
    return str_found.strip()

class newNode:
    def __init__(self, data, type):
        self.data = data
        self.type = type
        self.children=[]
        self.name=""
        self.iter=1
        self.left = self.right = None
        
def findVar(val,lis):
    for i in lis:
        if val in i.id:
            if i.type=="instantInt":
                return findBTWmarkers("=",";",i.id)
            elif i.type=="instantVar":
                print(i.id)
                a=findVar(findBTWmarkers("=",";",i.id),lis)
                if a is not None:
                    return a 

def checkChildren(node,val):
    if node.is_leaf:
        if(val in node.src):
            print("1",node.src)
            return node
        else:
            return None
    else:
        if(val in node.src):
            print("2",node.src)
            return node
        for i in node.children:
            a=checkChildren(i,val)
            if a is not None:
                return a    
        
    


def traverseDown(node,val):
    val=val.strip()
    final=None
    for n in node.iter_path_reverse():
        if(n.parent is not None):
            temp=False
            for i in n.parent.children:
                
                if (temp):
                    a=checkChildren(i,val)
                    if a is not None:
                        final=a
                    
                if(i==n):
                    temp=True
    print(final)
    return final
            
                
        
        # for s in n.siblings:
        #     print("this is ",s)
        #     if (val in s.src):
        #         print(n.src)   

def findArg(node,val):
    val=val.strip()
    for n in node.iter_path_reverse():
        print("this is ",n," vs this: ",val)
        if (val in n.src):
            print(n.src)
        for s in n.siblings:
            print("this is ",s)
            if (val in s.src):
                print(n.src)           

def getMother(motherNode):
    if(len(motherNode)==0): 
        return None
    temp=motherNode.pop()
    motherNode.append(temp)
    return temp

class Tree:
    def __init__(self,src, name):
        self.line= []
        self.src=src
        self.name=name
        self.data = ""
        
    def addChild(node):
        line.append(node)
        node.iter=len(line)
        
def fill_the_void(newTree,node,name,segmented,funcList):
    for child in node.children:
                print("THe children are: ",child.name,child.type) 
                if(child.type=="funcCall"):
                    r=compareFunc(funcList,child.name)
                    #fill_the_void(tree,name,segmented,funcList)
                    print(r," ",child.name," Bleep")
                    if(r==0): 
                         AnyNode(id=child.name,parent=newTree,src=child) 
                    else: 
                        fill_the_void( AnyNode(id=child.name,parent=newTree,src=r),child,child.name,segmented,funcList)
                else:
                     AnyNode(id=child.name,parent=newTree,src=child)
        
    
    
def print_list(list):
    for i in list:
        print("Name: ",i.name," Type:",i.type)

def findNodes(name,list):
    start=0;end=0;final_list=[]
    for idx, x in enumerate(list):
        #print(x.name," this vs this ",name)
        if(x.name==name):
            #print(x.name," this vs this ",name)
            end=idx
            final_list=list[start+1:end]
            #print(start,end,final_list)
            break
        elif(not(x.name.endswith("{"))):
            start=idx
    #print(name)    
    #print_list(final_list)
    return final_list

def commentsRemover(code):
    code2=""
    for line in code.split('\n'):
        if(re.search(".*\/\*.*\*\/.*", line)):
            code2+='\n'+line[:line.index('/*')].strip()
        elif(line.startswith("#")):
            1==1
        else:
            code2+='\n'+line.strip()
    return code2


def findPurpose(line):
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
    else:
        return "idk",line
    
def compareFunc(funcList,name):
    for i in funcList:
        if(name.startswith("if")|name.startswith("for")):
            print(i.id, " this vs this pt2****** " ,name)
            return i if i.id==name else None
            
        if("=" in name):
            name=name[:name.rfind('(')]
            name=name[name.rfind(' '):].strip()
        i.id[i.id.rfind(' '):].strip() 
        #print(i.name[i.name.rfind(' '):].strip()," i_name, ", name," name to compare")
        if name.startswith(i.id[i.id.rfind(' '):].strip()):
            
            return i
        
    for i in funcList:
        
        if i.id==name:
            return i
    return None
    


 
# This function is here just to test
 
 
# Driver Code
if __name__ == '__main__':
    code = '' ; code2 =''; segments=[]; totalLine=''; funcList=[];
    with open(os.path.join(sys.path[0], "heat.c"), "r") as f:
        temp=f.read()
        code+=temp

    # removes comments
    temp=""; iter=0; prevLine=""; comment=0; segmented=[];t="";n="";
    
    code=commentsRemover(code)
    #print(code)
    for line in code.split('\n'):
        line2=line.strip()
        if(line2.startswith('*/')|line2.startswith('/*')):
                comment+=1
        

        if(comment%2==0 and not(line2.startswith('*')|line2.startswith('/*')|line2.startswith('//')|(line2.strip()==""))):
            

            if(re.search(".*{.*};", line2)):
                
                a=newNode(line2,"init")
                a.data+=line2
                if(len(segments)>0):
                    b=segments.pop()
                    b.data+=line2
                    b.children.append(a)
                    a.iter=len(b.children)
                    segments.append(b)

            elif(line2.endswith('{')):
                a=newNode("", totalLine+" "+line2)
                a.data+=totalLine+" "+line2
                t,n=findPurpose(totalLine+" "+line2)
                a.type=t;a.name=n;
                #print(line2+" appending time")
                if(len(segments)>0):
                    b=segments.pop()
                    b.children.append(a)
                    b.data+=totalLine+" "+line2
                    a.iter=len(b.children)
                    segments.append(b)
                segments.append(a)
                totalLine=""
                
            
            elif(line2.endswith('}') | line2.endswith('};') ):
                #print(line2 + " popping time")
                a=segments.pop()
                a.data+=line2
                segmented.append(a)
                if(len(segments)>0):
                    b=segments.pop()
                    b.data+=a.data
                    segments.append(b)
            elif(line2.endswith(';')):
                
                a=newNode(totalLine+" "+line2,"func")
                t,n=findPurpose(totalLine+" "+line2)
                a.data+=totalLine+" "+line2
                a.type=t;a.name=n;
                if(len(segments)>0):
                    b=segments.pop()
                    b.data+=totalLine+" "+line2
                    b.children.append(a)
                    a.iter=len(b.children)
                    segments.append(b)
                totalLine="";
                             

            else:
                totalLine+=" " + line2
                
                
            
            code2+='\n'+line
            prevLine=line
    #print(str(len(segments)) + str(len(segmented)))
    #Create funclist
    for node in segmented:
        if node.type == "func":
            funcList.append(node)
    #format code to readable form
    code3=''
    for line in code2.split('\n'):
        line=line.strip()
        if (line.endswith(";")|line.endswith("{")|line.endswith("}")):
            code3+=line+'\n'
        else:
            code3+=line+" "
    
    #create tree 
    print(code3)  
    motherNode=[];nodes=[];funcs=[];mainMotherNode=[];tree=None;instant=[]
    for line in code3.split('\n'):
        t,n=findPurpose(line)
        
        if(tree is None):
            if(t in ['func',"ifCall","forCall"]):
                print("this is n: ",n," and its type= ",t)
            if(" main" in n):
                tree= AnyNode(id=n,parent=None,src=line,type=t)
                mainMotherNode.append(tree)
            elif(t in ['func','dataStrcture']):
                temp= AnyNode(id=n,parent=None,src=line,type=t)
                funcs.append(temp)
                nodes.append(temp)
                motherNode.append(temp)
            elif(t in ["ifCall","forCall"]):
                mom=getMother(motherNode)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
                motherNode.append(temp)
            elif(t=="funcCall"):
                r=compareFunc(funcs,n)
                if(r is None):
                    print("func does not exist before: ",n)
                    mom=getMother(motherNode)
                    temp= AnyNode(id=n,parent=mom,src=line,type=t)
                    nodes.append(temp)
                    
                else:
                    # mom=getMother(motherNode)
                    # temp= AnyNode(id=n,parent=mom,src=line)
                    # temp.children=r.children
                    
                    mom=getMother(motherNode)
                    #temp= AnyNode(id=n,parent=mom,src=line)
                    temp= AnyNode(id=r.id,parent=mom,src=line,children=r.children,type=t)
                    
                    
            elif(n=="}"):
                mom=getMother(motherNode)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
                motherNode.pop()
                print("popped ",mom.src)
                    
            else:
                print("in the else with size of mom: " , len(motherNode))
                mom=getMother(motherNode)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
            print("this is the node: " , temp, " with type: ",t)
        else:
            if(t in ["ifCall","forCall"]):
                mom=getMother(mainMotherNode)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
                mainMotherNode.append(temp)
            elif(t=="funcCall"):
                r=compareFunc(funcs,n)
                if(r is None):
                    print("func does not exist after: ",n)
                    mom=getMother(mainMotherNode)
                    temp= AnyNode(id=n,parent=mom,src=line,type=t)
                    nodes.append(temp)
                    
                else:
                    mom=getMother(mainMotherNode)
                    #temp= AnyNode(id=n,parent=mom,src=line)
                    print("this is debug: ",r,"/n",mom,"/n",n,"/n",t,"/n",line,"/n",r.parent,"/n",mom.parent,"/n")
                    temp= AnyNode(id=r.id,parent=tree,src=n,children=r.children,type=t)
                     
                
            elif(n=="}"):
                mom=getMother(mainMotherNode)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
                mainMotherNode.pop()
                    
            else:
                mom=getMother(mainMotherNode)
                print("this is main loop: ",n,mom,line)
                temp= AnyNode(id=n,parent=mom,src=line,type=t)
                nodes.append(temp)
        if(t in ["instantVar","instantInt"]):
            instant.append(temp)
                        
    for pre, fill, node in RenderTree(tree):
        print("%s%s" % (pre, node.id))
    
    commNode=None;parNode=None;arguments=[]; stats=[]
    for leaf in tree.leaves:
        if(leaf.id.startswith('MPI_Isend') ):
            #result = re.search('MPI_Isend(.*);', leaf.id)
            #result=result.group(1)
            result=leaf.id[leaf.id.find("(")+1:leaf.id.find(";")-1]
            arguments=result.split(',')
            res = re.search(arguments[1].strip()+" = [0-9]+;", code3)
            res=res.group(0)
            val="int "+res
            stats.append(val)
            #arguments[1]=res[res.find("=")+1:res.find(";")]
            temp=arguments[0]
            
            print(leaf.type)
            
            print(arguments)
            execNode=traverseDown(leaf,"*"+temp[1:min(arguments[0].index("["),arguments[0].index("("))])
            
            for node in leaf.iter_path_reverse():
                t,n=findPurpose(node.src)
                if t in ["funcCall"] and node!=leaf:
                    parNode=node
                    break;
                elif(t in ["ifCall","forCall"] or node==leaf):
                    commNode=node
            break;
    
    
    # for i in nodes:
    #     if(i.type=="idk"):
    #         print(i.id,i.type)   
    
    
    #print(parNode.descendants)
    
    prn=0
    f = open('../test/P2P_HEAT.hpp', 'w')
    with open('Content//top.txt','r') as firstfile:
      


      
    # read content from first file
       
        for line in firstfile:
                
                # append content to second file
                f.write(line)
    startArgs=[]
    for i in parNode.descendants:
        if "domain" in i.src:
            if i.src.startswith("int"):
                
                vari=i.src[i.src.find("int")+4:i.src.find("=")-1].strip()
                res = re.search(vari+" = [0-9]+;", code3)
                
    
                if res is not None:
                    res=res.group(0)
                    f.write("int "+res+"\n")
                    startArgs.append(res)
                else:
                    i.src=i.src.replace("domain->comm_rank","rank")
                    i.src=i.src.replace("domain->comm_size","systemsize")
                    f.write(i.src+"\n")
            elif i.src.startswith("struct"):
                pass
            else:
                i.src=i.src.replace("domain->comm_rank","rank")
                i.src=i.src.replace("domain->comm_size","systemsize")
                f.write(i.src+"\n")
        elif(i.src.startswith("MPI_Isend")):
            f.write("send = DisCosTiC->Isend("+i.src[i.src.find("(")+1:i.src.find(";")-1]+", comp);"+"\n")
        elif(i.src.startswith("MPI_Irecv")):
            f.write("recv = DisCosTiC->Irecv("+i.src[i.src.find("(")+1:i.src.find(";")-1]+", comp);"+"\n")
        
        else:
            f.write(i.src+"\n")
       
        if(i.src.startswith("MPI_Waitall(")):
            break;
        
    if execNode is not None:    
        for node in execNode.iter_path_reverse():
            t,n=findPurpose(node.src)
            if t in ["funcCall"] and node!=leaf:
                parNode=node
                break;
            
    
        
        prn=0
        #f = open('converted.c', 'w')
        
        #to write as an src
        # f.write("\ncomp = DisCosTiC->Exec(\"SRC:")
        # for i in parNode.descendants:
        #     f.write(i.src+"\\n\\n")
        # f.write("//BREAK:DAXPY//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D dim_x 200 dim_y 200\", recv);")

        here = os.path.dirname(os.path.realpath(__file__))
        subdir = "nodelevel"
        subdir2="kernels"
        filename = "heat.c"
        filepath = os.path.join(here, "..",subdir,subdir2, filename)
        try:
            ex = open(filepath, 'w')
            for i in parNode.descendants:
                if i.src.startswith("struct") or i.src.startswith("int dim_") or i.type=="ifCall" or i.parent.type=="ifCall" or i.src=="{" or i.src=="}":
                    pass
                elif i.src.startswith("double"):
                    if i.src.startswith("double *src"):
                        i.src="double src[dim_x*dim_y];"
                    elif i.src.startswith("double *dst"):
                        i.src="double dst[dim_x*dim_y];"
                    ex.write(i.src+"\n")
                else:
                    if i.src.endswith("{"):
                        i.src=i.src[:-1]
                    ex.write(i.src+"\n")
            ex.close()
            args=''
            for i in startArgs:
                args+= i.replace(" = "," ")+" "
                args=args.replace(";","")
            f.write("comp = DisCosTiC->Exec(\"FILE:heat.c//BREAK:HEAT//./nodelevel/machine-files/BroadwellEP_E5-2697v4_CoD.yml//18//-D "+args+"\", recv, process_Rank, comm);")  
        except IOError:
            print("Wrong path provided")



    else:
        print("Error: Could not find Exec portion of the code")    
        
    with open('Content//bottom.txt','r') as firstfile:
        
    # read content from first file
        for line in firstfile:
                
                # append content to second file
                f.write(line)
        
    
            
    
    
        
    

