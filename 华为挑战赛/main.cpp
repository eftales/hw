#include "iostream"
#include "cstdlib"
#include <cstdio>
#include <unordered_map>
#include <string>
#include <sstream>
#include <array>
#include <set>
#include <vector>
#include <cmath>
#include <cstring>
#include <algorithm>


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
#define PM_INDEX 	3	

#define FAILEDLIM	5
#define MIGTHRESH	5 // 迁移数目下限
#define MIG_VMID	0
#define MIG_PMID	1

const int MEM_bais = 2; // 服务器选型时的超参数
const int ENLARGE = 100;


string targetPM;

typedef array<int, 5> arrayPM; // cpu mem price mantain vsize
int physizeAlg(arrayPM& arr) { // 用于服务器选型
	//return arr[CPU] + MEM_bais * arr[MEM]; // TODO: 多次提交测得最佳参数
	return ENLARGE * arr[MEM] / arr[CPU];
}

string choosePM(unordered_map<string, arrayPM> *matPM,int targetV) {
	int diff = abs(matPM->begin()->second.at(VSIZE) - targetV);
	string pmName = matPM->begin()->first;
	for (auto i = ++matPM->begin(); i != matPM->end(); ++i) {
		if (abs(i->second.at(VSIZE) - targetV) < diff) {
			diff = abs(i->second.at(VSIZE) - targetV);
			pmName = i->first;

		}
	}

	return pmName;
}



typedef array<int, 3> arrayVM; // cpu mem node

typedef array<int, 5> arrayVM2PM; // cpu mem node pmid size

typedef array<int, 3> arrayMigOutput; // vmid pmid node

typedef array<int, 6> arrayOpAdd; // cpu mem node vmID vsize index


void vsizeAlg(vector<arrayOpAdd*> &opAdd, int len) { // TODO: 改成向量夹角

	for (int i = 0; i < opAdd.size(); ++i) {
		//opAdd[i]->at(VSIZE) = ENLARGE * opAdd[i]->at(MEM) / opAdd[i]->at(CPU);
		opAdd[i]->at(VSIZE) = opAdd[i]->at(MEM) + opAdd[i]->at(CPU);
	}
}

void quicksortUP(vector<arrayOpAdd*> &opAdd, int key, int begin, int end) {
	if (begin >= end) {
		return;
	}
	auto mid = opAdd[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (opAdd[end]->at(key) > mid->at(key)) {
				end -= 1;
			}
			else {
				opAdd[begin] = opAdd[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (opAdd[begin]->at(key) < mid->at(key)) {
				begin += 1;
			}
			else {
				opAdd[end] = opAdd[begin];
				end -= 1;
				break;
			}
		}


	}
	opAdd[begin] = mid;


	quicksortUP(opAdd, key, begin_, begin - 1);
	quicksortUP(opAdd, key, begin + 1, end_);


}

void quicksortDOWN(vector<arrayOpAdd*> &opAdd, int key, int begin, int end) {
	if (begin >= end) {
		return;
	}
	auto mid = opAdd[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (opAdd[end]->at(key) < mid->at(key)) {
				end -= 1;
			}
			else {
				opAdd[begin] = opAdd[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (opAdd[begin]->at(key) > mid->at(key)) {
				begin += 1;
			}
			else {
				opAdd[end] = opAdd[begin];
				end -= 1;
				break;
			}
		}

	}

	opAdd[begin] = mid;


	quicksortDOWN(opAdd, key, begin_, begin - 1);
	quicksortDOWN(opAdd, key, begin + 1, end_);


}



typedef array<int, 2> arrayNodeResource;
typedef struct PMResourceMsg_t
{
	arrayNodeResource nodeA;
	arrayNodeResource nodeB;
	set<int> vmset;
	int index;
	int usedSize;
	int unusedSize;
	int cpu, mem;

	PMResourceMsg_t(int index_, int cpu_, int mem_) {
		index = index_;
		usedSize = 0;
		unusedSize = 0;
		cpu = cpu_;
		mem = mem_;

		nodeA = arrayNodeResource  { cpu / 2,mem / 2 };
		nodeB = arrayNodeResource  { cpu / 2,mem / 2 };

	}

	void operator=(struct PMResourceMsg_t newOne) {
		nodeA = newOne.nodeA;
		nodeB = newOne.nodeB;
		usedSize = newOne.usedSize;
		unusedSize = newOne.unusedSize;
		cpu = newOne.cpu;
		mem = newOne.mem;
		index = newOne.index;
		vmset.swap(newOne.vmset);

	}

	void usedSizeAlg() { // 计算物理机负载的 vsize
		//usedSize = sqrt(pow(cpu - nodeA[CPU] - nodeB[CPU],2) + pow(mem - nodeA[MEM] - nodeB[MEM],2));
		usedSize = cpu - nodeA[CPU] - nodeB[CPU] + mem - nodeA[MEM] - nodeB[MEM]; // ��ʡʱ��

	}

	void unusedSizeAlg() { // 计算物理机负载的 vsize
		//unusedSize = sqrt(pow(nodeA[CPU] + nodeB[CPU], 2) + pow(nodeA[MEM] + nodeB[MEM], 2));
		unusedSize = nodeA[CPU] + nodeB[CPU] + nodeA[MEM] + nodeB[MEM];

	}

	int remainsizeAlg(arrayNodeResource& arr) { // 计算物理机的vsize
		return arr[CPU] * arr[MEM]; //!!
	}

	bool canSetSingle(arrayNodeResource& nodeMSG, arrayVM2PM& opaddMSG) {
		if (nodeMSG[CPU] > opaddMSG[CPU] && nodeMSG[MEM] > opaddMSG[MEM]) { // !
			return true;
		}
		else {
			return false;
		}

	}

	bool canSetDouble(arrayVM2PM& opaddMSG) {
		if (nodeA[CPU] > opaddMSG[CPU] / 2 && nodeA[MEM] > opaddMSG[MEM] / 2 && \
			nodeB[CPU] > opaddMSG[CPU] / 2 && nodeB[MEM] > opaddMSG[MEM] / 2) { // !
			return true;
		}
		else {
			return false;
		}

	}

	int addVM(int vmid, arrayVM2PM& vm) {
		if (vm[NODE] == NODE_DOUBLE) { // 原先是双节点
			if (canSetDouble(vm)) {
				nodeA[CPU] -= vm[CPU] / 2;
				nodeA[MEM] -= vm[MEM] / 2;
				nodeB[CPU] -= vm[CPU] / 2;
				nodeB[MEM] -= vm[MEM] / 2;
				vmset.insert(vmid);
				return NODE_DOUBLE;
			}
		}
		else { // 原先是单节点
			int whichNode = 0;
			if (remainsizeAlg(nodeA) <= remainsizeAlg(nodeB)) {
				whichNode = NODE_B;
			}
			else {
				whichNode = NODE_A;
			}

			if (whichNode == NODE_A && canSetSingle(nodeA, vm)) {
				nodeA[CPU] -= vm[CPU];
				nodeA[MEM] -= vm[MEM];
				vmset.insert(vmid);
				return NODE_A;

			}
			else if (whichNode == NODE_B && canSetSingle(nodeB, vm)) {
				nodeB[CPU] -= vm[CPU];
				nodeB[MEM] -= vm[MEM];
				vmset.insert(vmid);
				return NODE_B;
			}
		}
		return -1;

	}

	bool canSetSingle(arrayNodeResource& nodeMSG, arrayOpAdd& opaddMSG) {
		if (nodeMSG[CPU] > opaddMSG[CPU] && nodeMSG[MEM] > opaddMSG[MEM]) { // !
			return true;
		}
		else {
			return false;
		}

	}

	bool canSetDouble(arrayOpAdd& opaddMSG) {
		if (nodeA[CPU] > opaddMSG[CPU] / 2 && nodeA[MEM] > opaddMSG[MEM] / 2 && \
			nodeB[CPU] > opaddMSG[CPU] / 2 && nodeB[MEM] > opaddMSG[MEM] / 2) { // !
			return true;
		}
		else {
			return false;
		}

	}
	int addVM(arrayOpAdd& vm) {
		if (vm[NODE] == 0) { // single node
			int whichNode = 0;
			if (remainsizeAlg(nodeA) <= remainsizeAlg(nodeB)) {
				whichNode = NODE_B;
			}
			else {
				whichNode = NODE_A;
			}

			if (whichNode == NODE_A && canSetSingle(nodeA, vm)) {
				nodeA[CPU] -= vm[CPU];
				nodeA[MEM] -= vm[MEM];
				vmset.insert(vm[IDVM]);
				return NODE_A;

			}
			else if (whichNode == NODE_B && canSetSingle(nodeB, vm)) {
				nodeB[CPU] -= vm[CPU];
				nodeB[MEM] -= vm[MEM];
				vmset.insert(vm[IDVM]);
				return NODE_B;
			}
		}
		else { // 双节点部署
			if (canSetDouble(vm)) {
				nodeA[CPU] -= vm[CPU] / 2;
				nodeA[MEM] -= vm[MEM] / 2;
				nodeB[CPU] -= vm[CPU] / 2;
				nodeB[MEM] -= vm[MEM] / 2;
				vmset.insert(vm[IDVM]);
				return NODE_DOUBLE;
			}
		}
		return -1;

	}

	void delVM(int vmID, arrayVM2PM& vm) {
		if (vm[NODE] == NODE_DOUBLE) {
			nodeA[CPU] += vm[CPU] / 2;
			nodeA[MEM] += vm[MEM] / 2;
			nodeB[CPU] += vm[CPU] / 2;
			nodeB[MEM] += vm[MEM] / 2;
		}
		else {
			if (vm[NODE] == NODE_A) {
				nodeA[CPU] += vm[CPU];
				nodeA[MEM] += vm[MEM];

			}
			else {
				nodeB[CPU] += vm[CPU];
				nodeB[MEM] += vm[MEM];
			}
		}
		vmset.erase(vmID);

	}



}PMResourceMsg;

void quicksortPMUnusedUP(vector<PMResourceMsg*> &pm2vm, int begin, int end) { // 剩余空间升序
	if (begin >= end) {
		return;
	}
	auto mid = pm2vm[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (pm2vm[end]->unusedSize > mid->unusedSize) {
				end -= 1;
			}
			else {
				pm2vm[begin] = pm2vm[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (pm2vm[begin]->unusedSize < mid->unusedSize) {
				begin += 1;
			}
			else {
				pm2vm[end] = pm2vm[begin];
				end -= 1;
				break;
			}
		}
	}
	pm2vm[begin] = mid;


	quicksortPMUnusedUP(pm2vm, begin_, begin - 1);
	quicksortPMUnusedUP(pm2vm, begin + 1, end_);

}


void quicksortPMUnusedDown(vector<PMResourceMsg*> &pm2vm, int begin, int end) { // 剩余空间降序
	if (begin >= end) {
		return;
	}
	auto mid = pm2vm[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (pm2vm[end]->unusedSize < mid->unusedSize) {
				end -= 1;
			}
			else {
				pm2vm[begin] = pm2vm[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (pm2vm[begin]->unusedSize > mid->unusedSize) {
				begin += 1;
			}
			else {
				pm2vm[end] = pm2vm[begin];
				end -= 1;
				break;
			}
		}
	}
	pm2vm[begin] = mid;


	quicksortPMUnusedDown(pm2vm, begin_, begin - 1);
	quicksortPMUnusedDown(pm2vm, begin + 1, end_);

}


void bubblesortPMUsed(vector<PMResourceMsg*> &pm2vm,int begin) // 占用容量升序
{
	bool flag = false; //����һ����־λ
	for (int i = begin; (i < pm2vm.size()-1) && (flag == false); ++i)
	{
		flag = true;
		for (int j = i; j< pm2vm.size()-1; ++j)
		{
			if (pm2vm[j]->usedSize > pm2vm[j+1]->usedSize)
			{
				auto temp = pm2vm[j];
				pm2vm[j] = pm2vm[i];
				pm2vm[i] = temp;
				flag = false;
			}
		}
	}
}


void quicksortPMUsed(vector<PMResourceMsg*> &pm2vm, int begin, int end) { // 占用容量升序
	if (begin >= end) {
		return;
	}
	auto mid = pm2vm[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (pm2vm[end]->usedSize > mid->usedSize) {
				end -= 1;
			}
			else {
				pm2vm[begin] = pm2vm[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (pm2vm[begin]->usedSize < mid->usedSize) {
				begin += 1;
			}
			else {
				pm2vm[end] = pm2vm[begin];
				end -= 1;
				break;
			}
		}
	}
	pm2vm[begin] = mid;


	quicksortPMUsed(pm2vm, begin_, begin - 1);
	quicksortPMUsed(pm2vm, begin + 1, end_);

}

void quicksortPMIndex(vector<PMResourceMsg*> &pm2vm, int begin, int end) { // index 升序
	if (begin >= end) {
		return;
	}
	auto mid = pm2vm[begin];
	int begin_ = begin, end_ = end;
	while (begin < end) {
		while (begin < end) {
			if (pm2vm[end]->index > mid->index) {
				end -= 1;
			}
			else {
				pm2vm[begin] = pm2vm[end];
				begin += 1;
				break;
			}
		}
		while (begin < end) {
			if (pm2vm[begin]->index < mid->index) {
				begin += 1;
			}
			else {
				pm2vm[end] = pm2vm[begin];
				end -= 1;
				break;
			}
		}
	}
	pm2vm[begin] = mid;


	quicksortPMIndex(pm2vm, begin_, begin - 1);
	quicksortPMIndex(pm2vm, begin + 1, end_);

}

void mergesortPMIndex(vector<PMResourceMsg>* pm2vm, int begin, int end) { // index 升序
	if (end - begin <1) {
		return;
	}
	
	if (end - begin >= 2) { // 两个有序数组的排序
		int mid = begin + (end - begin) / 2;
		mergesortPMIndex(pm2vm, begin, mid);
		mergesortPMIndex(pm2vm, mid + 1, end);
		int base = begin,merge= mid+1;
		vector<PMResourceMsg> sorted;
		while (base <= mid || merge <= end) {
			if ((merge <= end) && ( (base > mid ) || (pm2vm->at(base).index > pm2vm->at(merge).index))) {
				sorted.push_back(pm2vm->at(merge));
				merge += 1;
			}
			if ((base <= mid) && ((merge > end) || (pm2vm->at(base).index < pm2vm->at(merge).index))) {
				sorted.push_back(pm2vm->at(base));
				base += 1;
			}

		}
		for (int i = begin; i <= end; ++i) {
			pm2vm->at(i) = sorted[i-begin];
		}
		return;


	}
	if (pm2vm->at(begin).index > pm2vm->at(end).index) { // 仅有两个元素
		auto temp = pm2vm->at(begin);
		pm2vm->at(begin) = pm2vm->at(end);
		pm2vm->at(end) = temp;
	}



}



PMResourceMsg* buyPM(string PMName, unordered_map<string, arrayPM>* matPM, int index_) {
	PMResourceMsg* msg = new PMResourceMsg(index_, matPM->at(PMName)[CPU], matPM->at(PMName)[MEM]);

	return msg;
}



string maxPM(unordered_map<string, arrayPM> &matPM) {
	unordered_map<string, arrayPM>::iterator itor;
	string res = matPM.begin()->first;
	for (auto i = matPM.begin(); i != matPM.end(); ++i) {
		if((matPM[res][CPU]+ MEM_bais * matPM[res][MEM]) < (i->second[CPU]+ MEM_bais * i->second[MEM])){
			res = i->first;
		}

	}
	return res;

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
	char currLine[100];
	string area20_s, area45_s, area60_s, area70_s, area90_s,defaultPM;
	
	int area20_tan=0.364*ENLARGE, area30_tan = 0.577 * ENLARGE, area45_tan = 1 * ENLARGE, area60_tan = 1.73 * ENLARGE, area70_tan=2.75*ENLARGE; // TODO:凹
	for (int i = 0; i < typePM; ++i) {
		cin >> _n; 
		string currLine;
		getline(cin, currLine);
		istringstream is(currLine);
		string pmName;
		is >> pmName;
		pmName.erase(pmName.end() - 1);
		int cpu, mem, price, maintain;

		is >> cpu >> _n >> mem >> _n >> price >> _n >> maintain >> _n;
		if (cpu<100 || mem < 200) {
			continue;
		}
		arrayPM pmVal = { cpu, mem, price, maintain,0 };
		pmVal[VSIZE] = physizeAlg(pmVal);
		if (pmVal[VSIZE] < area20_tan) {
			//if (area20_s == "") {
			//	matPM[pmName] = pmVal;
			//	area20_s = pmName;
			//}
			//else {
			//	if (matPM[area20_s][CPU] + matPM[area20_s][MEM] * MEM_bais < pmVal[CPU] + pmVal[MEM] * MEM_bais) {
			//		matPM.erase(area20_s);
			//		matPM[pmName] = pmVal;
			//		area20_s = pmName;
			//	}
			//}
		}
		else if (pmVal[VSIZE] < area45_tan) {
			if (area45_s == "") {
				matPM[pmName] = pmVal;
				area45_s = pmName;
			}
			else {
				if (matPM[area45_s][CPU] + matPM[area45_s][MEM] * MEM_bais < pmVal[CPU] + pmVal[MEM] * MEM_bais) {
					matPM.erase(area45_s);
					matPM[pmName] = pmVal;
					area45_s = pmName;
				}
			}
		}
		else if (pmVal[VSIZE] < area60_tan) {
			if (area60_s == "") {
				matPM[pmName] = pmVal;
				area60_s = pmName;
			}
			else {
				if (matPM[area60_s][CPU] + matPM[area60_s][MEM] * MEM_bais < pmVal[CPU] + pmVal[MEM] * MEM_bais) {
					matPM.erase(area60_s);
					matPM[pmName] = pmVal;
					area60_s = pmName;
				}
			}
		}
		else if ( pmVal[VSIZE] < area70_tan) {
			if (area70_s == "") {
				matPM[pmName] = pmVal;
				area70_s = pmName;
			}
			else {
				if (matPM[area70_s][CPU] + matPM[area70_s][MEM] * MEM_bais < pmVal[CPU] + pmVal[MEM] * MEM_bais) {
					matPM.erase(area70_s);
					matPM[pmName] = pmVal;
					area70_s = pmName;
				}
			}
		}
		else {
			//if (area90_s == "") {
			//	matPM[pmName] = pmVal;
			//	area90_s = pmName;
			//}
			//else {
			//	if (matPM[area90_s][CPU] + matPM[area90_s][MEM] * MEM_bais < pmVal[CPU] + pmVal[MEM] * MEM_bais) {
			//		matPM.erase(area90_s);
			//		matPM[pmName] = pmVal;
			//		area90_s = pmName;
			//	}
			//}
		}
		// matPM[pmName] = pmVal;

		//if (currMaxVsize == -1) {
		//	currMaxVsize = matPM[pmName][VSIZE];
		//	currMaxName = pmName;
		//}
		//else if (currMaxVsize < matPM[pmName][VSIZE]) {
		//	currMaxVsize = matPM[pmName][VSIZE];
		//	currMaxName = pmName;
		//}


	}

	//targetPM = currMaxName;

	defaultPM = maxPM(matPM);


	//* 读取虚拟机信息
	int typeVM, node;
	cin >> typeVM;
	unordered_map<string, arrayVM> matVM;
	for (int i = 0; i < typeVM; ++i) {
		cin >> _n; 
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
	vector<PMResourceMsg*> pm2vm;
	vector<arrayOpAdd*> opAdd; // cpu mem node vmID vsize index
	vector<int> opDel; // vmID
	vector<int> opDelTODO; // vmID
	int numPM = 0;

	vector<arrayMigOutput> migOutput; // vmid pmid node


	int days;
	cin >> days;
	for (int i = 0; i < days; ++i) {
		int todayReq;
		cin >> todayReq;
		int opAddIndex = 0, opDelIndex = 0, opDelTODOIndex = 0;
		int sumMEM = 0, sumCPU = 0,maxMEM=0,maxCPU=0;

		for (int request = 0; request < todayReq; ++request) {
			cin >> _n; 
			string currLine;
			getline(cin, currLine);
			istringstream is(currLine);
			string op;
			is >> op;
			op.erase(op.end() - 1);

			int vmID;
			if (op == "add") { // op == add
				string vmType;
				is >> vmType;
				vmType.erase(vmType.end() - 1);
				is >> vmID >> _n;
				sumCPU += matVM[vmType][CPU];
				sumMEM += matVM[vmType][MEM];
				maxCPU = max(maxCPU, matVM[vmType][CPU]);
				maxMEM = max(maxMEM, matVM[vmType][MEM]);

				arrayOpAdd* vmMsg = new arrayOpAdd{ matVM[vmType][CPU],matVM[vmType][MEM],matVM[vmType][NODE],vmID,0,opAddIndex };

				if (opAddIndex < opAdd.size()) {
					delete opAdd[opAddIndex];
					opAdd[opAddIndex] = vmMsg;
				}
				else {
					opAdd.push_back(vmMsg);

				}

				opAddIndex += 1;

			}
			else {
				is >> vmID >> _n;

				if (opDelIndex < opDel.size()) {
					opDel[opDelIndex] = vmID;
				}
				else {
					opDel.push_back(vmID);

				}

				opDelIndex += 1;

			}
		}


		//* 迁移

		int NumVmlive = vm2pm.size(), migCount = 0;
		int FailedMig = 0;
		int maxMigration = NumVmlive * 0.005;
		if (maxMigration > 0) { // MIGTHRESH
			for (int i = 0; i < pm2vm.size(); ++i) {
				pm2vm[i]->usedSizeAlg(); // 统计资源占用
			}
			quicksortPMUsed(pm2vm, 0, pm2vm.size() - 1); // 占用资源大小升序
			int minNeedSort = pm2vm.size();
			for (int i = 0; i < pm2vm.size(); ++i) {
				typedef array<int, 3> toDelVMMSG; // vmid node targetPM
				vector<toDelVMMSG> toDelVM;
				int todayDel = 0;
				for (int vmid : pm2vm[i]->vmset) {
					for (int j = pm2vm.size() - 1; j > i; --j) {
						if (find(opDel.begin(), opDel.end(), vmid) != opDel.end()) {
							todayDel += 1;
							continue;
						}
						if (migCount >= maxMigration) {
							break;
						}
						int whichNode = pm2vm[j]->addVM(vmid, vm2pm[vmid]);
						if (whichNode != -1) {
							toDelVMMSG vmMsg = { vmid ,whichNode, pm2vm[j]->index };
							toDelVM.push_back(vmMsg);

							arrayMigOutput mig = { vmid ,pm2vm[j]->index, whichNode };
							if (migCount < migOutput.size()) {
								migOutput[migCount] = mig;
							}
							else {
								migOutput.push_back(mig);
							}
							migCount += 1;
							quicksortPMUsed(pm2vm, j, pm2vm.size() - 1); // 每迁移一次虚拟机就排一次序 TODO:冒泡排序
							minNeedSort = min(j, minNeedSort);
							break;
						}
					}

				}
				if (toDelVM.size() + todayDel < pm2vm[i]->vmset.size()) {
					FailedMig += 1;
				}
				for (toDelVMMSG vmMsg : toDelVM) {
					int vmid = vmMsg[0];
					int whichNode = vmMsg[1];
					int pmid = vmMsg[2];
					pm2vm[i]->delVM(vmid, vm2pm[vmid]);
					vm2pm[vmid][NODE] = whichNode;
					vm2pm[vmid][PM_INDEX] = pmid;

				}
				//quicksortPMUsed(pm2vm, minNeedSort, pm2vm.size() - 1); // 每迁移一次物理机就排一次序
				if (FailedMig >= FAILEDLIM || migCount >= maxMigration) {
					break;
				}
			}
			quicksortPMIndex(pm2vm, 0, pm2vm.size() - 1); // index 大小升序
		}





		//* 部署
		// 计算需求的 vsize
		vsizeAlg(opAdd, opAddIndex);
		quicksortDOWN(opAdd, VSIZE, 0, opAddIndex - 1);
		int area20_num = 0, area45_num = 0, area60_num = 0, area70_num = 0, area90_num = 0; // 记录每天服务器的增量
		int targetV = ENLARGE * sumMEM / sumCPU;
		targetPM = choosePM(&matPM, targetV);


		if (matPM[targetPM][CPU] < maxCPU || matPM[targetPM][MEM] < maxMEM) {
			targetPM = defaultPM;
		}



		int unsortEnd = pm2vm.size() - 1;
		for (int i = 0; i < opAddIndex; ++i) {
			bool isSet = false;
			for (int i = 0; i < unsortEnd  + 1; ++i) {
				pm2vm[i]->unusedSizeAlg();
			}
			quicksortPMUnusedUP(pm2vm, 0, unsortEnd); // TODO:凹
			for (int j = 0; j < pm2vm.size(); ++j) { // 找到一个可以放下请求的物理机
				int whichNode = pm2vm[j]->addVM(*opAdd[i]); // 返回值为 -1 NODE_A NODE_b NODE_DOUBLE
				if (whichNode != -1) { // 
					isSet = true;
					arrayVM2PM vmMsg = { opAdd[i]->at(CPU),opAdd[i]->at(MEM),whichNode,pm2vm[j]->index };
					vm2pm[opAdd[i]->at(IDVM)] = vmMsg;
					unsortEnd = j;
					sumCPU -= opAdd[i]->at(CPU);
					sumMEM -= opAdd[i]->at(MEM);
					break;
				}
			}
			if (isSet == false) { // 买一台物理机

				auto newPM = buyPM(targetPM, &matPM, pm2vm.size());
				pm2vm.push_back(newPM);

				if (targetPM == area20_s) {
					area20_num += 1;
				}
				else if (targetPM == area45_s) {
					area45_num += 1;
				}
				else if (targetPM == area60_s) {
					area60_num += 1;
				}
				else if (targetPM == area70_s) {
					area70_num += 1;
				}
				else {
					area90_num += 1;
				}

				int whichNode = newPM->addVM(*opAdd[i]);
				int j = pm2vm.size() - 1;

				arrayVM2PM vmMsg = { opAdd[i]->at(CPU),opAdd[i]->at(MEM),whichNode,pm2vm[j]->index };
				vm2pm[opAdd[i]->at(IDVM)] = vmMsg;
				unsortEnd = pm2vm.size() - 1;
			}
		}
		quicksortPMIndex(pm2vm, 0, pm2vm.size() - 1);

		//* 删除
		for (int i = 0; i < opDelIndex; ++i) {
			int vmID = opDel[i];
			arrayVM2PM pmvmMsg = vm2pm[vmID];
			vm2pm.erase(vmID);
			int pmIndex = pmvmMsg[PM_INDEX];
			pm2vm[pmIndex]->delVM(vmID, pmvmMsg);
		}


		char outPMTypeAll[] = "(purchase, %d)\n";
		char outPMType[] = "(%s, %d)\n";
		char outMigration[] = "(migration, %d)\n";
		char outMigrationSingle[] = "(%d, %d, %s)\n";
		char outMigrationDouble[] = "(%d, %d)\n";
		char outDeploySingle[] = "(%d, %s)\n";
		char outDeployDouble[] = "(%d)\n";
		int needtoBuy = pm2vm.size() - numPM;
		if (needtoBuy == 0) {
			printf(outPMTypeAll, 0);
		}
		else {

			printf(outPMTypeAll, 1);
			if (area20_num != 0) {
				needtoBuy = area20_num;
			}
			else if (area45_num != 0) {
				needtoBuy = area45_num;
			}
			else if (area60_num != 0) {
				needtoBuy = area60_num;
			}
			else if (area70_num != 0) {
				needtoBuy = area70_num;
			}
			else {
				needtoBuy = area90_num;
			}

			printf(outPMType, targetPM.data(), needtoBuy);
		}
		numPM = pm2vm.size();

		printf(outMigration, migCount);
		for (int i = 0; i < migCount; ++i) {
			if (migOutput[i][NODE] == NODE_DOUBLE) {
				printf(outMigrationDouble, migOutput[i][MIG_VMID], migOutput[i][MIG_PMID]);
			}
			else {
				if (migOutput[i][NODE] == NODE_A) {
					printf(outMigrationSingle, migOutput[i][MIG_VMID], migOutput[i][MIG_PMID], "A");
				}
				else {
					printf(outMigrationSingle, migOutput[i][MIG_VMID], migOutput[i][MIG_PMID], "B");
				}
			}
		}

		quicksortUP(opAdd, ADD_INDEX, 0, opAddIndex - 1);
		for (int i = 0; i < opAddIndex; ++i) {
			int vmID = opAdd[i]->at(IDVM);
			arrayVM2PM pmvmMsg = vm2pm[vmID];
			if (pmvmMsg[NODE] == NODE_DOUBLE) {
				printf(outDeployDouble, pmvmMsg[PM_INDEX]);
			}
			else {
				if (pmvmMsg[NODE] == NODE_A) {
					printf(outDeploySingle, pmvmMsg[PM_INDEX], "A");
				}
				else {
					printf(outDeploySingle, pmvmMsg[PM_INDEX], "B");
				}
			}

		}

	}


	fflush(stdout);

	return 0;
}
