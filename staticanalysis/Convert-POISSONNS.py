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
        if i == "=":
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
    #print(final)
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
        line=line.strip()
        if(re.search(".*\/\*.*\*\/.*", line)):
            code2+='\n'+line[:line.index('/*')].strip()
        elif(line.startswith("#")):
            pass
        elif(line.startswith("/*") or line.startswith("*/") or line.startswith("*") or line.startswith("//")):
            pass
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
        print(i.id[i.id.rfind(' '):].strip() ," i_name, ", name," name to compare")
        if name.startswith(i.id[i.id.rfind(' '):].strip()):
            
            return i
        
    for i in funcList:
        
        if i.id==name:
            return i
    return None

def isfloat(num):
    try:
        float(num)
        return True
    except ValueError:
        return False

def var_replacer(line, number_dict):
    if (line.find("=") != -1):
        line2 = line[line.find("="):-1].strip()
        for key in number_dict:
            if line2.find('jmaxLocal') != -1:
                pass
            elif line2.find(key) != -1:
                line2 = line2.replace(key , str(number_dict[key]))
                
        line = line.replace(line[line.find("="): -1],line2)
    
    return line
 
 
#############################################################################
#                               Driver Code                                 #
#############################################################################

if __name__ == '__main__':
    code = '' ; code2 =''; segments=[]; totalLine=''; funcList=[]; number_dict=dict();
    with open(os.path.join("poissonNS.c"), "r") as f:
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
                    #b.data+=totalLine+" "+line2
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


    #Creates a dictionary with float values and its variable name from the code.
    #Only those variables with float value as RHS is taken.
    
    for line in code.split('\n'):
        line=line.strip()
        if line.find(" = ") != -1:
            line=line.replace('int ', '')
            line=line.replace('float ', '')
            line=line.replace('char ', '')
            line=line.replace('double ', '')
            subline1 = line[0:line.find("=")]
            subline2 = line[line.find(" = ")+2:len(line)-1]
            
            if(isfloat(subline2)):
                subline1=re.sub(r'^.*->', '', subline1)
                number_dict[subline1.strip()] = subline2
                
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
                    mom=getMother(motherNode)
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
            result=leaf.id[leaf.id.find("(")+1:leaf.id.find(";")-1]
            arguments=result.split(',')
            res = re.search(arguments[1].strip()+" = [0-9]+;", code3)
            res=res.group(0)
            val="int "+res
            stats.append(val)
            temp=arguments[0]
            
            execNode=traverseDown(leaf,"*"+temp[1:min(arguments[0].index("["),arguments[0].index("("))])
            
            for node in leaf.iter_path_reverse():
                t,n=findPurpose(node.src)
                if t in ["funcCall"] and node!=leaf:
                    parNode=node
                    break;
                elif(t in ["ifCall","forCall"] or node==leaf):
                    commNode=node
            break;
    
    
    prn=0
    f = open('../test/P2P_POISSONNS.hpp', 'w')
    with open('Content/top.txt','r') as firstfile:
    # read content from first file
        for line in firstfile:
                # append content to second file
                f.write(line)


    startArgs=[]
    for i in parNode.descendants:
        if "solver->" in i.src:
            i.src=i.src.replace("solver->","")
            if i.src.startswith("double"):
                i.src = var_replacer(i.src, number_dict)
            if i.src.find("size"):
                i.src=i.src.replace("size","systemsize")
            if i.src.startswith("int"):
                i.src = var_replacer(i.src, number_dict)
                vari=i.src[i.src.find("int")+4:i.src.find("=")-1].strip()
                res = re.search(vari+" = [0-9]+;", code3)
                
    
                if res is not None:
                    res=res.group(0)
                    f.write("\t\t\t\tint "+res+"\n")
                    startArgs.append(res)
                else:
                    f.write("\t\t\t\t"+i.src+"\n")
            elif i.src.startswith("struct"):
                pass
            elif i.src.startswith("double *src"):
                i.src="double src[imax+2][jmaxLocal+2];"
                f.write("\t\t\t\t"+i.src+"\n")
            elif i.src.startswith("double *dst"):
                i.src="double dst[imax+2][jmaxLocal+2];"
                f.write("\t\t\t\t"+i.src+"\n")
            elif(i.src.startswith("MPI_Isend")):
                f.write("\t\t\t\t"+"send = DisCosTiC->Isend("+i.src[i.src.find("(")+1:i.src.find(";")-1]+", comp);"+"\n")
            elif(i.src.startswith("MPI_Irecv")):
                f.write("\t\t\t\t"+"recv = DisCosTiC->Irecv("+i.src[i.src.find("(")+1:i.src.find(";")-1]+", comp);"+"\n")
            else:
                f.write("\t\t\t\t"+i.src+"\n")
        
        else:
            
            f.write("\t\t\t\t"+i.src+"\n")
       
        if(i.src.startswith("MPI_Waitall(")):
            break;
        
    if execNode is not None:    
        for node in execNode.iter_path_reverse():
            t,n=findPurpose(node.src)
            if t in ["funcCall"] and node!=leaf:
                parNode=node
                break;
            

        ###################################################################
        #      Code for preparing hotspot as required by KERNCRAFT        #
        ###################################################################

        empty_vars=list()        

        here = os.path.dirname(os.path.realpath(__file__))
        subdir = "nodelevel"
        subdir2="kernels"
        filename = "POISSONNS.c"
        filepath = os.path.join(here, "..",subdir,subdir2, filename)
        
        try:
            ex = open(filepath, 'w')
            for i in parNode.descendants:
                if i.src.startswith("struct") or i.src.startswith("return") or i.type=="ifCall" or i.parent.type=="ifCall":
                    pass
                elif(i.src.find("int jmaxLocal") != -1 or i.src.find("int imax") != -1 or i.src.find("int jmax") != -1):
                    continue
                elif (i.src.find("=") != -1):
                    line = i.src.replace('solver->','')
                    
                    if i.src.startswith("double"):
                        if i.src.startswith("double *src"):
                            i.src="double src["+number_dict['jmax'].strip()+"]["+number_dict['imax'].strip()+"];"
                        elif i.src.startswith("double *rhs"):
                            i.src="double rhs["+number_dict['jmax'].strip()+"]["+number_dict['imax'].strip()+"];"
                        else:
                            i.src=i.src[0:i.src.find("=")]+";"
                        ex.write(i.src+"\n")
                        
                    elif i.src.find("int ") != -1:
                        if(i.src.find("for") == -1):
                            ex.write(i.src.replace('int ', 'double ')+"\n")
                        else:

                            ex.write(i.src+"\n")
                    else:
                        
                        ex.write(i.src+"\n")
                        
                else:
                    if(i.src.find("=") == -1):
                        if(i.src.find(",") == -1):
                            if(len(i.src[i.src.find(" "):-1].strip()) != 0):
                                empty_vars.append(i.src[i.src.find(" "):-1].strip())
                        else:
                            multi=i.src[i.src.find(" "):-1].strip().split(",")
                            multi = [x.strip(' ') for x in multi]
                            empty_vars.extend(multi)
                    ex.write(i.src+"\n")
            
            
            ex.close()
            
            args='-D '
            for i in startArgs:
                args+= i.replace(" = "," ")+" "
                args=args.replace(";","")

            for i in empty_vars:
                args+='-D ' + i + " 0.0 "
            f.write("\t\t\t\t//comp = DisCosTiC->Exec(\"LBL:POISSONNS\", recv, process_Rank, comm);\n")  
            f.write("\t\t\t\t//comp = DisCosTiC->Exec(\"COMP:TOL=128.0||TnOL=24.0|TL1L2=32.2|TL2L3=0.0|TL3Mem=0.0\", recv, process_Rank, comm);\n")  
            f.write("\t\t\t\t//comp = DisCosTiC->Exec(\"SRC:double r1; \\n double res; \\n double dx; \\n double dy; \\n double dx2; \\n double dy2; \\n double idx2; \\n double idy2; \\n double omega; \\n double factor; \\n double src[200][200]; \\n double rhs[200][200]; \\n for( int j=1; j<jmaxLocal+1; j++ ) { \\n for( int i=1; i<imax+1; i++ ) { \\n r1 = rhs[(j)*(imax+2) + (i)] - ((src[(j)*(imax+2) + (i-1)] - 2.0 * src[(j)*(imax+2) + (i)] + src[(j)*(imax+2) + (i+1)]) * idx2 + (src[(j-1)*(imax+2) + (i)] - 2.0 * src[(j)*(imax+2) + (i)] + src[(j+1)*(imax+2) + (i)]) * idy2); \\n src[(j)*(imax+2) + (i)] = src[(j)*(imax+2) + (i)] - (factor * r1); \\n res = res + (r1 * r1); \\n } \\n }\\n//BREAK:POISSONNS//./nodelevel/machine-files/BroadwellEP_E5-2630v4.yml//10//-D imax 200 -D  jmaxLocal \"+std::to_string(jmaxLocal), recv, process_Rank, comm);\n")  
            f.write("\t\t\t\tcomp = DisCosTiC->Exec(\"FILE:POISSONNS.c//BREAK:POISSONNS//./nodelevel/machine-files/BroadwellEP_E5-2630v4.yml//10//"+args+"-D  jmaxLocal \"+std::to_string(jmaxLocal), recv,process_Rank, comm);")  
        except IOError:
            print("Wrong path provided")



    else:
        print("Error: Could not find Exec portion of the code")    
        
    with open('Content/bottom.txt','r') as firstfile:
        
    # read content from first file
        for line in firstfile:
                
                # append content to second file
                f.write(line)
    f.close()
