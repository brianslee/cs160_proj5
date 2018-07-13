#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of repeat loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  // WRITEME: Replace with code if necessary
  //Build ClassTable
  classTable = new ClassTable();
  node->visit_children(this);
  // Error Handling?
  //Well, We need to check after the main class and method
  //Although I think the main method thing can be in classnode
  if(classTable->count("Main") == 0)
    {
      typeError(no_main_class);
    }
}

void TypeCheck::visitClassNode(ClassNode* node) {
  // WRITEME: Replace with code if necessary
  //Build MethodTable and VariableTable
  MethodTable *memberMethods = new MethodTable();
  VariableTable *memberVariables = new VariableTable();
  currentLocalOffset = 0;
  currentMemberOffset = 0;
  currentClassName = node->identifier_1->name;

  //Create classinfo
  ClassInfo *thisClass = new ClassInfo();
  //thisClass->superClassName = node->identifier_2->name;

  if(node->identifier_2 != NULL)
    {
      //Does superclass exist in classTable?
      if(classTable->count(node->identifier_2->name) == 0)
	{
	  typeError(undefined_class);
	}
      else
	{
	  thisClass->superClassName = node->identifier_2->name;
	}
    }
  else
    {
      thisClass->superClassName = "";
    }

  std::string supClass = thisClass->superClassName;

  currentVariableTable = memberVariables;
  currentMethodTable = memberMethods;

  for(std::list<DeclarationNode*>::iterator it = node->declaration_list->begin(); it != node->declaration_list->end(); ++it)
    {
      visitDeclarationNode(*it);
    }

  //thisClass->methods = memberMethods;
  thisClass->members = currentVariableTable;

  //I think this is asking the number of member variables. That or the number of member functions.
  thisClass->membersSize = memberVariables->size() * 4;

  //Check error: Does main class have any members?
  if(currentClassName == "Main" && currentVariableTable->size() > 0)
    {
      typeError(main_class_members_present);
    }

  for(std::list<MethodNode*>::iterator it2 = node->method_list->begin(); it2 != node->method_list->end(); ++it2)
    {
      visitMethodNode(*it2);
    }

  thisClass->methods = currentMethodTable;

  //Does Main class have main function?
  if(currentClassName == "Main" && currentMethodTable->count("main") == 0)
    {
      typeError(no_main_method);
    }

  //Insert into classTable
  classTable->insert(std::pair<std::string, ClassInfo>(currentClassName, *thisClass));
}

void TypeCheck::visitMethodNode(MethodNode* node) {
  // WRITEME: Replace with code if necessary
  //Build VariableTable
  VariableTable *methodVars = new VariableTable();
  currentVariableTable = methodVars;
  //Create methodinfo
  MethodInfo *thisMethod = new MethodInfo();
  CompoundType comType;
  //thisMethod->variables = methodVars;
  currentParameterOffset = 12;
  currentLocalOffset = -4;

  std::string currentMethName = node->identifier->name;

  node->type->accept(this);

  comType.baseType = node->type->basetype;
  comType.objectClassName = node->type->objectClassName;

  thisMethod->returnType = comType;

  std::list<CompoundType> *params = new std::list<CompoundType>;
  if(node->parameter_list != NULL){
    for(std::list<ParameterNode*>::iterator it = node->parameter_list->begin(); it != node->parameter_list->end(); ++it)
      {
	(*it)->type->accept(this);
	CompoundType comType2;
	comType2.baseType = (*it)->type->basetype;
	comType2.objectClassName = (*it)->type->objectClassName;

	if(thisMethod->parameters)thisMethod->parameters->push_back(comType2);

	VariableInfo thisParam;
	thisParam.type = comType2;
	thisParam.offset = currentParameterOffset;
	thisParam.size = 4;
	currentParameterOffset += 4;

	currentVariableTable->insert(std::pair<std::string, VariableInfo>((*it)->identifier->name, thisParam));
      }
  }
  visitMethodBodyNode(node->methodbody);

  thisMethod->variables = currentVariableTable;
  thisMethod->localsSize = -currentLocalOffset - 4;

  //Checking errors
  if((node->type->basetype == bt_none) && (node->methodbody->returnstatement != NULL))
    {
      typeError(return_type_mismatch);
    }

  if((node->type->basetype != bt_none) && (node->methodbody->basetype != node->type->basetype))//Return Type Problem
    {
      //typeError(return_type_mismatch);
    }

  if((node->type->basetype != bt_none) && (node->methodbody->returnstatement == NULL))
    {
      typeError(return_type_mismatch);
    }

  if(node->type->basetype == bt_object && node->methodbody->basetype == bt_object)
    {
      if(node->methodbody->objectClassName != node->type->objectClassName)
	{
	  //Check superclasses
	  std::string supClass = classTable->find(node->methodbody->objectClassName)->second.superClassName;

	  while(supClass != "")
	    {
	      if(supClass != node->type->objectClassName)
		{
		  supClass = classTable->find(supClass)->second.superClassName;
		}
	      else
		{
		  break;
		}
	    }

	  if(supClass == "")
	    {
	      typeError(return_type_mismatch);
	    }
	}
    }

  if(currentClassName == node->identifier->name && node->type->basetype != bt_none)
    {
      typeError(constructor_returns_type);
    }

  if(currentClassName == "Main" && node->identifier->name == "main")
    {
      //Main function in main Class
      if(thisMethod->parameters && thisMethod->parameters->size() > 0)
	{
	  typeError(main_method_incorrect_signature);
	}
    }

  //Insert into MethodTable
  currentMethodTable->insert(std::pair<std::string, MethodInfo>(node->identifier->name, *thisMethod));
  std::map<std::string, ClassInfo>::iterator it2;
  it2 = classTable->find(currentClassName);
  it2->second.methods = currentMethodTable;
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->returnstatement != NULL)
    {
      node->basetype = node->returnstatement->basetype;
      node->objectClassName = node->returnstatement->objectClassName;
    }
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  // WRITEME: Replace with code if necessary
  CompoundType compType;
  node->type->accept(this);
  compType.baseType = node->type->basetype;

  if(node->type->objectClassName != "")
    {
      if(classTable->count(node->type->objectClassName) == 0)
	{
	  typeError(undefined_class);
	}
      compType.objectClassName = node->type->objectClassName;
    }

  //Create VariableInfos for each thing on the std::list
  for(std::list<IdentifierNode*>::iterator it = node->identifier_list->begin(); it != node->identifier_list->end(); ++it)
    {
      VariableInfo thisVar;
      thisVar.type = compType;
      thisVar.size = 4;

      (*it)->basetype = node->type->basetype;

      if(currentLocalOffset == 0)
	{
	  thisVar.offset = currentMemberOffset;
	  currentMemberOffset += 4;
	}
      else
	{
	  thisVar.offset = currentLocalOffset;
	  currentLocalOffset -= 4;
	}

      (*currentVariableTable)[(*it)->name] = thisVar;
    }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  // WRITEME: Replace with code if necessary
  node->expression->accept(this);
  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  // WRITEME: Replace with code if necessary
  node->expression->accept(this);
  CompoundType compType;
  compType.objectClassName = " ";

  if(currentVariableTable->count(node->identifier_1->name) > 0)
    {
      compType = currentVariableTable->find(node->identifier_1->name)->second.type;
    }
  else {
    if(classTable->count(currentClassName) > 0) {
      if(classTable->find(currentClassName)->second.members->count(node->identifier_1->name) > 0)
	{
	  compType = classTable->find(currentClassName)->second.members->find(node->identifier_1->name)->second.type;
	}
      else
	{
	  ClassInfo curClassInfo = classTable->find(currentClassName)->second;
	  std::string supClass = curClassInfo.superClassName;

	  while(supClass != "")
	    {
	      curClassInfo = classTable->find(supClass)->second;

	      if(curClassInfo.members->count(node->identifier_1->name) > 0)
		{
		  compType = curClassInfo.members->find(node->identifier_1->name)->second.type;
		  break;
		}
	      supClass = curClassInfo.superClassName;
	    }
	}
    }
  }

  //Check for Errors
  if(node->identifier_2 == NULL)
    {
      if(compType.objectClassName != " ")
	{
	  node->basetype = compType.baseType;
	  node->objectClassName = compType.objectClassName;
	}
      else
	{
	  typeError(undefined_variable);
	}
    }
  else
    {
      if(compType.objectClassName == " ")
	{
	  typeError(undefined_variable);
	}

      if(compType.baseType != bt_object)
	{
	  typeError(not_object);
	}

      if(classTable->count(compType.objectClassName) > 0 && classTable->find(compType.objectClassName)->second.members->count(node->identifier_2->name) > 0)
	{
	  node->basetype = classTable->find(compType.objectClassName)->second.members->find(node->identifier_2->name)->second.type.baseType;
	  node->objectClassName = classTable->find(compType.objectClassName)->second.members->find(node->identifier_2->name)->second.type.objectClassName;
	}
      else
	{
	  typeError(undefined_member);
	}
    }

  if(node->basetype != node->expression->basetype)//Assignment type mismatch problem.
    {
      //    typeError(assignment_type_mismatch);
    }
  if((node->basetype == bt_object) && (node->objectClassName != node->expression->objectClassName) )
    {
      typeError(assignment_type_mismatch);
    }
}

void TypeCheck::visitCallNode(CallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  node->basetype = node->methodcall->basetype;
  node->objectClassName = node->methodcall->objectClassName;
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype != bt_boolean)//WHY?
    {
      //typeError(if_predicate_type_mismatch);
    }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype!=bt_boolean){
    typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression->basetype!=bt_boolean)
    {
      typeError(do_while_predicate_type_mismatch);
    }
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    {
      //typeError(expression_type_mismatch);
    }
  node->basetype = bt_integer;
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    {
      //typeError(expression_type_mismatch);
    }
  node->basetype = bt_integer;
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)//WHY THE FUCK DOESN'T THIS WORK?
    {
      //typeError(expression_type_mismatch);
    }
  node->basetype = bt_integer;
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_integer;
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_boolean;
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression_1->basetype != bt_integer || node->expression_2->basetype != bt_integer)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_boolean;
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if((node->expression_1->basetype != node->expression_2->basetype) || (node->expression_1->basetype == bt_none || node->expression_1->basetype == bt_object))
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_boolean;
}

void TypeCheck::visitAndNode(AndNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_boolean;
}

void TypeCheck::visitOrNode(OrNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if(node->expression_1->basetype != bt_boolean || node->expression_2->basetype != bt_boolean)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_boolean;
}

void TypeCheck::visitNotNode(NotNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype =  bt_boolean;
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if (node->expression->basetype != bt_integer)
    {
      ////typeError(expression_type_mismatch);
    }
  node->basetype = bt_integer;
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  MethodInfo calledMethod;
  calledMethod.localsSize = -1;

  if(node->identifier_2 == NULL)
    {
      if(currentMethodTable->count(node->identifier_1->name) > 0)
	{
	  calledMethod = currentMethodTable->find(node->identifier_1->name)->second;
	}

      ClassInfo calledClass;
      if(classTable->count(currentClassName) > 0)
	calledClass = classTable->find(currentClassName)->second;
      std::string supClass = calledClass.superClassName;

      while(supClass != "")
	{
	  calledClass = classTable->find(supClass)->second;
	  if(calledClass.methods->count(node->identifier_1->name) > 0)
	    {
	      calledMethod = calledClass.methods->find(node->identifier_1->name)->second;
	      break;
	    }
	  supClass = calledClass.superClassName;
	}
    }
  else
    {
      CompoundType compType;
      compType.objectClassName = " ";

      if(currentVariableTable->count(node->identifier_1->name) > 0)
	{
	  compType = currentVariableTable->find(node->identifier_1->name)->second.type;
	}
      else
	if(classTable->count(currentClassName) > 0 )
	  if(classTable->find(currentClassName)->second.members->count(node->identifier_1->name) > 0)
	    {
	      compType = classTable->find(currentClassName)->second.members->find(node->identifier_1->name)->second.type;
	    }
          else
	    {
	      ClassInfo curClassInfo = classTable->find(currentClassName)->second;
	      std::string supClass = curClassInfo.superClassName;

	      while(supClass != "")
		{
		  curClassInfo = classTable->find(supClass)->second;
		  if(curClassInfo.members->count(node->identifier_1->name) > 0)
		    {
		      compType = curClassInfo.members->find(node->identifier_1->name)->second.type;
		      break;
		    }
		  supClass = curClassInfo.superClassName;
		}
	    }

      if(compType.objectClassName == " " || compType.baseType != bt_object)
	{
	  typeError(not_object);
	  //return;
	}
      if(classTable->count(compType.objectClassName) > 0 )
	if(classTable->find(compType.objectClassName)->second.methods->count(node->identifier_2->name) > 0)
	  {
	    calledMethod = classTable->find(compType.objectClassName)->second.methods->find(node->identifier_2->name)->second;
	  }
	else
	  {
	    ClassInfo curClassInfo = classTable->find(compType.objectClassName)->second;
	    std::string supClass = curClassInfo.superClassName;
	    std::string curObject = node->identifier_2->name;

	    while(supClass != "")
	      {
		curClassInfo = classTable->find(supClass)->second;
		if(curClassInfo.methods->count(curObject) > 0)
		  {
		    calledMethod = curClassInfo.methods->find(curObject)->second;
		    break;
		  }
		supClass = curClassInfo.superClassName;
	      }
	  }
    }

  if(calledMethod.localsSize == -1)
    {
      typeError(undefined_method);
      //return;
    }

  if(calledMethod.parameters != NULL && node->expression_list != NULL && calledMethod.parameters->size() != node->expression_list->size())
    {
      typeError(argument_number_mismatch);
      //return;
    }
  if(calledMethod.parameters != NULL){
    std::list<CompoundType>::iterator methodParamIter = calledMethod.parameters->begin();
    for(std::list<ExpressionNode*>::iterator nodeParamIter = node->expression_list->begin(); nodeParamIter != node->expression_list->end(); ++nodeParamIter)
      {
	if((*nodeParamIter)->basetype != (*methodParamIter).baseType || (*nodeParamIter)->objectClassName != (*methodParamIter).objectClassName)
	  {
	    typeError(argument_type_mismatch);
	  }
	++methodParamIter;
      }
  }
  node->basetype = calledMethod.returnType.baseType;
  node->objectClassName = calledMethod.returnType.objectClassName;
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  // WRITEME: Replace with code if necessary
  CompoundType compType;
  compType.objectClassName = " ";

  if(currentVariableTable->count(node->identifier_1->name) > 0)
    {
      compType = currentVariableTable->find(node->identifier_1->name)->second.type;
    }

  if(classTable->count(currentClassName) > 0 && classTable->find(currentClassName)->second.members->count(node->identifier_1->name) > 0)
    {
      compType = classTable->find(currentClassName)->second.members->find(node->identifier_1->name)->second.type;
    }
  else
    {
      if(classTable->count(currentClassName) > 0){
	ClassInfo curClassInfo = classTable->find(currentClassName)->second;
	std::string supClass = curClassInfo.superClassName;

	while(supClass != "")
	  {
	    curClassInfo = classTable->find(supClass)->second;

	    if(curClassInfo.members->count(node->identifier_1->name) > 0)
	      {
		compType = curClassInfo.members->find(node->identifier_1->name)->second.type;
		break;
	      }
	    supClass = curClassInfo.superClassName;
	  }
      }
    }

  if(compType.baseType != bt_object || compType.objectClassName == " ")
    {
      //typeError(not_object);
    }
  if(classTable->count(compType.objectClassName) > 0 )
    if(classTable->find(compType.objectClassName)->second.members->count(node->identifier_2->name)> 0)
      {
	compType = classTable->find(compType.objectClassName)->second.members->find(node->identifier_2->name)->second.type;
      }
    else
      {
	ClassInfo curClassInfo;
	curClassInfo.membersSize = -1;
	curClassInfo = classTable->find(compType.objectClassName)->second;
	std::string supClass = curClassInfo.superClassName;
	std::string curObject = node->identifier_2->name;

	compType.objectClassName = " ";

	while(supClass != "")
	  {
	    curClassInfo = classTable->find(supClass)->second;

	    if(curClassInfo.members->count(curObject) > 0)
	      {
		compType = curClassInfo.members->find(curObject)->second.type;
		break;
	      }
	    supClass = curClassInfo.superClassName;
	  }

	if(compType.objectClassName == " ")
	  {
	    typeError(undefined_member);
	    //return;
	  }
      }

  node->basetype = compType.baseType;
  node->objectClassName = compType.objectClassName;
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);
  CompoundType variableType;
  variableType.objectClassName = " ";

  if(currentVariableTable->count(node->identifier->name) > 0)
    {
      variableType = currentVariableTable->find(node->identifier->name)->second.type;
    }
  if(classTable->count(currentClassName) > 0){
    if(classTable->find(currentClassName)->second.members->count(node->identifier->name) > 0)
      {
	variableType = classTable->find(currentClassName)->second.members->find(node->identifier->name)->second.type;
      }
    else
      {
	ClassInfo curClassInfo = classTable->find(currentClassName)->second;
	std::string supClass = curClassInfo.superClassName;

	while(supClass != "")
	  {
	    curClassInfo = classTable->find(supClass)->second;
	    if(curClassInfo.members->count(node->identifier->name) > 0)
	      {
		variableType = curClassInfo.members->find(node->identifier->name)->second.type;
		break;
	      }
	    supClass = curClassInfo.superClassName;
	  }
      }
  }
  if(variableType.objectClassName == " ")//Causes undefined_variable problem
    {
      //typeError(undefined_variable);
    }

  node->basetype = variableType.baseType;
  node->objectClassName = variableType.objectClassName;
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->objectClassName = "";
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->objectClassName = "";
}

void TypeCheck::visitNewNode(NewNode* node) {
  // WRITEME: Replace with code if necessary
  node->visit_children(this);

  if(classTable->count(node->identifier->name) == 0)
    {
      typeError(undefined_class);
      //return;
    }

  if(node->expression_list != NULL)
    {
      MethodInfo thisMethod = classTable->find(node->identifier->name)->second.methods->find(node->identifier->name)->second;
      if(thisMethod.parameters != NULL){
	if(thisMethod.parameters->size() != node->expression_list->size())
	  {
	    typeError(argument_number_mismatch);
	  }

	auto methodParamIter = thisMethod.parameters->begin();
	for(auto nodeParamIter = node->expression_list->begin(); nodeParamIter != node->expression_list->end(); ++nodeParamIter)
	  {
	    if((*nodeParamIter)->basetype != (*methodParamIter).baseType || (*nodeParamIter)->objectClassName != (*methodParamIter).objectClassName)
	      {
		typeError(argument_type_mismatch);
	      }
	    ++methodParamIter;
	  }
      }
    }

  node->basetype = bt_boolean;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_integer;
  node->objectClassName = "";
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_boolean;
  node->objectClassName = "";
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  // WRITEME: Replace with code if necessary
  node->basetype = bt_none;
  node->objectClassName = "";
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  // WRITEME: Replace with code if necessary
}


// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
