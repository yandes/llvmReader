#include <cstdio>
#include <cstdint>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include <string>
#include <iostream>
#include <unordered_set>
using namespace llvm;
using namespace std;

int main(int argc, char **argv)
{
	typedef std::unordered_set<std::string> setCalled;
	typedef std::unordered_set<std::string> setFunction;
	typedef std::unordered_set<std::string> setVariables;
	typedef std::unordered_set<uintptr_t> setFunctionPointers;
	typedef std::unordered_set<uintptr_t> setFunctionsPointed;
	setCalled Scalls;
	setFunction Sfunc;
	setVariables Svar;
	setFunctionPointers Sfuncptrs;
	setFunctionsPointed Sfuncptd;

	// Read the IR file.
	LLVMContext &Context = getGlobalContext();
	SMDiagnostic Err;
	for(int i=1;i<argc;i++){
	Module *M = ParseIRFile(argv[i], Err, Context);
	if (M == NULL)
	{
		fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
		return EXIT_FAILURE;
	}

	for (auto curFref = M->getFunctionList().begin(), 
			endFref = M->getFunctionList().end(); 
			curFref != endFref; ++curFref) {
		string str = curFref->getName().str().c_str();
		Sfunc.insert(str);
		Sfuncptd.insert(reinterpret_cast<uintptr_t>(&curFref));
		printf("functions: %s\n", curFref->getName());
		//printf("found function:%s\n",curFref->getName().str().c_str())reinterpret_cast<uintptr_t>(curFref);
	}

	// Function Pointers are stored as global variables
	for (auto curVref = M->getGlobalList().begin(),
			endVref = M->getGlobalList().end();
			curVref != endVref; ++curVref) {
		string var = curVref->getName().str().c_str();
		Svar.insert(var);
		printf("Globals:%s and %lu\n",curVref->getName() ,reinterpret_cast<uintptr_t>(&curVref));
		//printf("Variable: %s\n", var);
	}


	for (auto &F: *M){
		for (auto &BB: F){
			for (auto &I: BB){

				// S/O copied code that I have no idea how to use
				int s1=0,s2=0;
				if (StoreInst *si = dyn_cast<StoreInst>(&I)) {
				
				//printf("-----------------Store inst Value: %s-----------------\n",vSi->getName());
				printf("\n-----------------Store 0 str: %s-----------------\n",si->getOperand(0)->getName());
				//printf("-----------------Store add:%u-------------\n", si->getOperand(0));
				printf("-----------------Store 1 str: %s-----------------\n",si->getOperand(1)->getName());
				std::unordered_set<std::string>::const_iterator store1 = Sfunc.find (si->getOperand(0)->getName());
				if (!(store1 == Sfunc.end()) ){
					s1=1;
					printf("s1: %d\n",s1);
				}
				std::unordered_set<std::string>::const_iterator store2 = Svar.find (si->getOperand(1)->getName());
				if (!(store2 == Svar.end())){
					s2=1;
					printf("s2: %d\n",s2);
				}
				if(s1 && s2){
				Scalls.insert(si->getOperand(0)->getName());
				printf("\n-----------------Inserted-------------\n");
				}
				
				//printf("-----------------Store add: %u-------------\n\n", si->getOperand(1));
				//printf("-------------StoreInstI: %lu-------------\n\n", reinterpret_cast<uintptr_t>(&I));			

				
					Value* v = si->getPointerOperand();
					Type* ptrType = v->getType()->getPointerElementType();
					if (PointerType* pt = dyn_cast<PointerType>(ptrType)) {
						do {
							Type* pointedType = pt->getTypeAtIndex((unsigned int)0);
							if (pointedType->isFunctionTy()) {

								break;
							}
							ptrType = pointedType;
						} while ((pt = (dyn_cast<PointerType>(ptrType))));
					}
					//printf("Second address of function ptr: %lu\n", reinterpret_cast<uintptr_t>(&I));				
					Sfuncptrs.insert(reinterpret_cast<uintptr_t>(&I));
				}

				CallInst *Call = dyn_cast<CallInst>(&I);
				if (Call == NULL) {
					continue;
				}
				Function *G = Call->getCalledFunction();
				if (G == NULL) {
					continue;
				}
				string str = G->getName().str().c_str();
				Scalls.insert(str);
				printf("Called: %s\n", G->getName().str().c_str());
				//I.dump();

			}		
		}
	}

	// Comparing addresses of function ptr and functions; same address means the ptr is calling the function
	for (auto element: Sfuncptrs) {
		for (auto curFref = M->getFunctionList().begin(), 
				endFref = M->getFunctionList().end(); 
				curFref != endFref; ++curFref) {
			//printf("Third address of function ptr: %lu\n", element);
			//printf("Third address of function: %lu\n", reinterpret_cast<uintptr_t>(&curFref));
			if (element == reinterpret_cast<uintptr_t>(&curFref)){
				Scalls.insert(curFref->getName().str().c_str());
				printf("Called: %s\n", curFref->getName().str().c_str());
			}
		}
	}
	}

	printf("Total Number of Calls: %lu\n",Scalls.size());
	printf("Total Number of Functions: %lu\n",Sfunc.size());

	int counter = 0;
	printf("List of Dead functions:\n");
	for(auto& elem: Sfunc){
		std::unordered_set<std::string>::const_iterator got = Scalls.find (elem);
		if ( got == Scalls.end() ){
			if (elem != "main"){
				std::cout <<elem <<endl;
				counter++; 
			}
		}
	}
	printf("Total Number of Dead functions: %d\n",counter);





	return 0;
}
