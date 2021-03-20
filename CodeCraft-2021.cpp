#include "iostream"
#include "cstdlib"
#include "cstdio"
#include <unordered_map>
#include <string>
#include <sstream>
#include <array>
#include <set>
#include <vector>

using namespace std;
//* 宏定义
#define CPU			0
#define MEM			1
#define NODE 		2
#define IDVM 		3
#define VSIZE 		4
#define ADD_INDEX 	5
#define NODE_A 		0
#define NODE_B 		1
#define NODE_DOUBLE	2
#define VMSET 		2
#define VM2PM_INDEX 	3	
#define ENLARGE		100000


string targetPM;

typedef array<int, 5> arrayPM; // cpu mem price mantain vsize
int physizeAlg(arrayPM* arr) { // 计算物理机的vsize，也用于计算node的vsize
	return arr->at(CPU) + arr->at(MEM); 
}


typedef array<int, 3> arrayVM;

typedef array<int, 4> arrayVM2PM;


typedef array<int, 2> arrayNodeResource;
typedef struct PMResourceMsg_t
{
	arrayNodeResource nodeA;
	arrayNodeResource nodeB;
	set<int> vmset;
}PMResourceMsg;


PMResourceMsg buyPM(string PMName, unordered_map<string, arrayPM>* matPM) {
	PMResourceMsg msg;
	arrayNodeResource rsc = { matPM->at(PMName)[CPU] / 2,matPM->at(PMName)[MEM] / 2 };
	msg.nodeA = rsc;
	msg.nodeB = rsc;
	return msg;
}


typedef array<int, 6> arrayOpAdd;


void vsizeAlg(vector<arrayOpAdd> *opAdd){
	int sumCPU = 0,sumMEM = 0;
	for (int i=0;i<opAdd->size();++i){
		sumCPU += opAdd->at(i)[CPU];
		sumMEM += opAdd->at(i)[MEM];
	}
	for (int i=0;i<opAdd->size();++i){
		opAdd->at(i)[VSIZE] = ENLARGE*(opAdd->at(i)[CPU]/sumCPU + opAdd->at(i)[MEM]/sumMEM);
	}
}

void quicksortUP(vector<arrayOpAdd> *opAdd,int key,int begin,int end){
	int mid = begin;
	int begin_ = begin,end_ = end;
	begin += 1;
	while (begin <  end){
		while(begin<end){
			if (opAdd->at(end)[VSIZE] >= opAdd->at(mid)[VSIZE]){
				end -= 1;
			}
			else{
				break;
			}
		}
		while(begin<end){
			if (opAdd->at(begin)[VSIZE] <= opAdd->at(mid)[VSIZE]){
				begin += 1;
			}
			else{
				break;
			}
		}
		auto temp = opAdd->at(begin);
		opAdd->at(begin) = opAdd->at(end);
		opAdd->at(end) = temp;

	}
	auto temp = opAdd->at(begin);
	opAdd->at(begin) = opAdd->at(mid);
	opAdd->at(mid) = temp;

	quicksortUP(opAdd,key,begin_,mid-1);
	quicksortUP(opAdd,key,mid+1,end_);


}

void quicksortDOWN(vector<arrayOpAdd> *opAdd,int key,int begin,int end){
	int mid = begin;
	int begin_ = begin,end_ = end;
	begin += 1;
	while (begin <  end){
		while(begin<end){
			if (opAdd->at(end)[VSIZE] <= opAdd->at(mid)[VSIZE]){
				end -= 1;
			}
			else{
				break;
			}
		}
		while(begin<end){
			if (opAdd->at(begin)[VSIZE] >= opAdd->at(mid)[VSIZE]){
				begin += 1;
			}
			else{
				break;
			}
		}
		auto temp = opAdd->at(begin);
		opAdd->at(begin) = opAdd->at(end);
		opAdd->at(end) = temp;

	}
	auto temp = opAdd->at(begin);
	opAdd->at(begin) = opAdd->at(mid);
	opAdd->at(mid) = temp;

	quicksortUP(opAdd,key,begin_,mid-1);
	quicksortUP(opAdd,key,mid+1,end_);


}

bool canSetSingle(arrayNodeResource* nodeMSG,arrayOpAdd* opaddMSG){
	if (nodeMSG->at(CPU)>= opaddMSG->at(CPU) && nodeMSG->at(MEM)>= opaddMSG->at(MEM)){
		return true;
	}
	else{
		return false;
	}

}

bool canSetDouble(arrayNodeResource* nodeMSG,arrayOpAdd* opaddMSG){
	if (nodeMSG->at(CPU)>= opaddMSG->at(CPU)/2 && nodeMSG->at(MEM)>= opaddMSG->at(MEM)/2){
		return true;
	}
	else{
		return false;
	}

}
int main()
{
	// TODO:read standard input
	// TODO:process

	int typePM;
	char _n;// 读取末尾换行符

	cin >> typePM;
	unordered_map<string, arrayPM> matPM;
	int currMaxVsize = -1;
	string currMaxName;
	//* 读取物理机数据并选出 vsize 最大的物理机
	for (int i = 0; i < typePM; ++i) {
		cin >> _n; // 读取 (
		string currLine;
		getline(cin, currLine);
		istringstream is(currLine);
		string pmName;
		is >> pmName;
		pmName.erase(pmName.end() - 1);
		int cpu, mem, price, maintain;

		is >> cpu >> _n >> mem >> _n >> price >> _n >> maintain >> _n;
		arrayPM pmVal = { cpu, mem, price, maintain,0 };
		pmVal[VSIZE] = physizeAlg(&pmVal);
		matPM[pmName] = pmVal;

		if (currMaxVsize == -1) {
			currMaxVsize = matPM[pmName][VSIZE];
			currMaxName = pmName;
		}
		else if (currMaxVsize < matPM[pmName][VSIZE]) {
			currMaxVsize = matPM[pmName][VSIZE];
			currMaxName = pmName;
		}

	}

	targetPM = currMaxName;


	//* 读取虚拟机信息
	int typeVM;
	cin >> typeVM;
	unordered_map<string, arrayVM> matVM;
	for (int i = 0; i < typeVM; ++i) {
		cin >> _n; // 读取 (
		string currLine;
		getline(cin, currLine);
		istringstream is(currLine);
		string vmName;
		is >> vmName;
		vmName.erase(vmName.end() - 1);
		int cpu, mem, node;

		is >> cpu >> _n >> mem >> _n >> node >> _n;
		arrayVM vmVal = { cpu, mem, node };
		matVM[vmName] = vmVal;

	}



	unordered_map<int, arrayVM2PM> vm2pm;
	vector<PMResourceMsg> pm2vm;
	vector<arrayOpAdd> opAdd; // cpu mem node vmID vsize index
	vector<int> opDel; // vmID
	int numPM = 0;

	int days;
	cin >> days;
	for (int i = 0; i < days; ++i) {
		int todayReq;
		cin >> todayReq;
		int opAddIndex = 0, opDelIndex = 0;
		for (int request = 0; request < todayReq; ++request) {
			cin >> _n; // 读取 (
			string currLine;
			getline(cin, currLine);
			istringstream is(currLine);
			string op;
			is >> op;
			op.erase(op.end() - 1);

			int vmID;
			if (op == "add") {
				string vmType;
				is >> vmType;
				vmType.erase(vmType.end() - 1);
				is >> vmID >> _n;
				arrayOpAdd vmMsg = { matVM[vmType][CPU],matVM[vmType][MEM],matVM[vmType][NODE],vmID,0,opAddIndex };
				opAdd.push_back(vmMsg);
				opAddIndex += 1;

			}
			else {
				is >> vmID >> _n >> _n;
				opDel.push_back(vmID);
				opDelIndex += 1;

			}
		}

		//* 删除
		for (int i = 0;i<opDelIndex;++i){
			int vmID = opDel[i];
			arrayVM2PM pmvmMsg = vm2pm[vmID];
			vm2pm.erase(vmID);
			int pmIndex = pmvmMsg[VM2PM_INDEX];
			if (pmvmMsg[NODE] == NODE_DOUBLE){
				pm2vm[pmIndex].nodeA[CPU] += pmvmMsg[CPU];
				pm2vm[pmIndex].nodeA[MEM] += pmvmMsg[MEM];
				pm2vm[pmIndex].nodeB[CPU] += pmvmMsg[CPU];
				pm2vm[pmIndex].nodeB[MEM] += pmvmMsg[MEM];
			}
			else{
				if (pmvmMsg[NODE] == NODE_A){
					pm2vm[pmIndex].nodeA[CPU] += pmvmMsg[CPU];
					pm2vm[pmIndex].nodeA[MEM] += pmvmMsg[MEM];
				}
				else{
					pm2vm[pmIndex].nodeB[CPU] += pmvmMsg[CPU];
					pm2vm[pmIndex].nodeB[MEM] += pmvmMsg[MEM];
				}
			}
			pm2vm[pmIndex].vmset.erase(vmID);
		}

		//* 迁移


		//* 部署
		// 计算需求的 vsize
		vsizeAlg(&opAdd);
		quicksortUP(&opAdd,VSIZE,0,opAdd.size());
		for (int i=0;i<opAddIndex;++i){
			bool isSet = false;
			for (int j=0;j<pm2vm.size();++j){ // 找到一个可以放下请求的物理机
				if (opAdd[i][NODE] == 0){ // single node
					int whichNode = 0;
					if (physizeAlg((arrayPM*)&(pm2vm[j].nodeA)) <= physizeAlg((arrayPM*)&(pm2vm[j].nodeB))){
						whichNode = NODE_B;
					}
					else{
						whichNode = NODE_A;
					}

					if (whichNode == NODE_A && canSetSingle(&(pm2vm[j].nodeA),&(opAdd[i]))){
						pm2vm[j].nodeA[CPU] -= opAdd[i][CPU];
						pm2vm[j].nodeA[MEM] -= opAdd[i][MEM];
						pm2vm[j].vmset.insert(opAdd[i][IDVM]);
						arrayVM2PM vmMsg = {opAdd[i][CPU],opAdd[i][MEM],whichNode,j};
						vm2pm[opAdd[i][IDVM]] = vmMsg;
						isSet = true;
					}
					else if (whichNode == NODE_B && canSetSingle(&(pm2vm[j].nodeB),&(opAdd[i]))){
						pm2vm[j].nodeB[CPU] -= opAdd[i][CPU];
						pm2vm[j].nodeB[MEM] -= opAdd[i][MEM];
						pm2vm[j].vmset.insert(opAdd[i][IDVM]);
						arrayVM2PM vmMsg = {opAdd[i][CPU],opAdd[i][MEM],whichNode,j};
						vm2pm[opAdd[i][IDVM]] = vmMsg;
						isSet = true;
					}
				}
				else{ // 双节点部署
					if (canSetDouble(&(pm2vm[j].nodeA),&(opAdd[i])) && canSetDouble(&(pm2vm[j].nodeB),&(opAdd[i]))){
						pm2vm[j].nodeA[CPU] -= opAdd[i][CPU]/2;
						pm2vm[j].nodeA[MEM] -= opAdd[i][MEM]/2;
						pm2vm[j].nodeB[CPU] -= opAdd[i][CPU]/2;
						pm2vm[j].nodeB[MEM] -= opAdd[i][MEM]/2;
						pm2vm[j].vmset.insert(opAdd[i][IDVM]);
						arrayVM2PM vmMsg = {opAdd[i][CPU],opAdd[i][MEM],NODE_DOUBLE,j};
						vm2pm[opAdd[i][IDVM]] = vmMsg;
						isSet = true;

					}
				}
			}
			if (isSet == false){ // 买一台物理机
				pm2vm.push_back(buyPM(targetPM, &matPM)); // TODO 判断是否一定放得下
				int j = pm2vm.size() - 1;
				if (opAdd[i][NODE] == 0){ // single node
					int whichNode = NODE_A;
					pm2vm[j].nodeA[CPU] -= opAdd[i][CPU];
					pm2vm[j].nodeA[MEM] -= opAdd[i][MEM];
					pm2vm[j].vmset.insert(opAdd[i][IDVM]);
					arrayVM2PM vmMsg = {opAdd[i][CPU],opAdd[i][MEM],whichNode,j};
					vm2pm[opAdd[i][IDVM]] = vmMsg;

				}
				else{ // 双节点部署
					pm2vm[j].nodeA[CPU] -= opAdd[i][CPU]/2;
					pm2vm[j].nodeA[MEM] -= opAdd[i][MEM]/2;
					pm2vm[j].nodeB[CPU] -= opAdd[i][CPU]/2;
					pm2vm[j].nodeB[MEM] -= opAdd[i][MEM]/2;
					pm2vm[j].vmset.insert(opAdd[i][IDVM]);
					arrayVM2PM vmMsg = {opAdd[i][CPU],opAdd[i][MEM],NODE_DOUBLE,j};
					vm2pm[opAdd[i][IDVM]] = vmMsg;
				}
			}
		}


		char* outPMTypeAll = "(purchase, %d)";
		char* outPMType = "(%s, %d)";
		char* outMigration = "(migration, %d)";
		char* outDeploySingle = "(%d, %s)";
		char* outDeployDouble = "(%d)";
		int needtoBuy = pm2vm.size() - numPM;
		if (needtoBuy == 0){
			printf(outPMTypeAll , 0);
		}
		else{
			printf(outPMTypeAll , 1);
			printf(outPMType , targetPM.data(),needtoBuy);
		}
		numPM = pm2vm.size();

		printf(outMigration , 0);

		quicksortDOWN(&opAdd,VM2PM_INDEX,0,opAdd.size());
		for (int i=0;i<todayReq;++i){
			if (opAdd[i][IDVM] != 0){
				int vmID = opAdd[i][IDVM];
				arrayVM2PM pmvmMsg = vm2pm[vmID];
				if (pmvmMsg[NODE] == NODE_DOUBLE){
					printf(outDeployDouble , vm2pm[vmID][VM2PM_INDEX]);
				}
				else{
					if (pmvmMsg[NODE] == NODE_A){
						printf(outDeploySingle , vm2pm[vmID][VM2PM_INDEX],'A');
					}
					else{
						printf(outDeploySingle , vm2pm[vmID][VM2PM_INDEX],'B');
					}
				}
			}
		}

	}


	// TODO:write standard output
	// TODO:fflush(stdout);

	return 0;
}


