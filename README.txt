Brian Lee
3620101
brianslee@umail.ucsb.edu

Zeyu Zhu
8825663
zzhu@umail.ucsb.edu

Issues/Explanations:
Our thing is giving us Segmentation Faults, and we're not entirely sure why. This whole assignment for us was akin to the blind teaching the blind
to paint-by-numbers. There's way too much for us to explain one function at a time with what we have, so I'll try to give a general sort of high-
level thing of what we've attempted.

So the very first thing we tried to do was to create the symbol table. This started with creating the program's sole ClassTable. Then, with each of
the children of the ProgramNode, which should be ClassNodes. In each of the ClassNodes, we created a ClassInfo which, in addition to information
like superclasses and the size of its members, also holds a MethodTable and a VariableTable for its member methods and variables. One of these
classes should also be the Main class which only holds the main method and nothing else. These ClassInfos will be mapped to the ClassTable by the
class's name.

While we're creating the ClassInfo, we'll be filling out the class's MethodTable by visiting the MethodNodes and creating MethodInfo structs. These
contain information like the returnType, the parameters, the size of the method, and a VariableTable which holds the method's variables. If this
method is in the Main class, it must the the main function. Otherwise, we return an error. The completed MethodInfo will be placed in its parent
class's MethodTable.

Finally, there are the VariableNodes. For each of these , we'll also be filling out a VariableInfo. This holds the variable's type, its size, and
its offset. Then, we pass it back to the parent's VariableTable, which can be in a ClassInfo or a MethodInfo.

Because we can't really tell who or what the node's parent is, we use variables like currentMethodTable and currentVariableTable and so forth to
keep track of the tables that we're inserting the MethodInfos and VariableInfos into.

Once we have a working symbol table, then we begin to typecheck the whole thing. With each node we go down, we ensure that called methods have their
proper return types, and are on the MethodTable of a class, or its superclass. We ensure that variables are properly declared in methods and classes.
We check if the variables or expressions used as the parameters for our operators and the methods are of the proper quantity and type. And that's
the nonexhaustive short list.

Theoretically, if we did everything right, that should be the end of it. Clearly, that isn't the case. I wish I could be more descriptive, and we do
have more than a passing idea of what we did. We just have no idea what exactly is going wrong. That being said, I'd like to think that for two blind
men trying to paint, we got pretty close to what the picture was supposed to be.

Edit: Since the writing of this README, my partner had started to debug the thing. Whether it works or not is hit-and-miss.
Edit 2: Looks like our symbol table is working fine. We're having trouble with our TypeChecking, though. Namely, a problem with the
        undefined_variable error in our visitVariableNode function. We have other problems, but that seems to be the big one, as it dominates the
	        bad tests, and affects most of the good ones.
		Edit 3: We've basically commented out the bad TypeChecks. But I believe out Symbol Table works well.
