import numpy as np

CPU = 0
MEM = 1
NODE = 2
IDVM = 3
VSIZE = 4
INDEX = 5
NODE_A = 0
NODE_B = 1
VMSET = 2

def vsizeAlg(opAdd):
    opAdd[:,4] = opAdd[:,0]/np.sum(opAdd[:,0]) + opAdd[:,1]/np.sum(opAdd[:,1])

def physizeAlg(cpu,mem):
    return cpu*mem


targetPM = None
def buyPM(matPM): # 记录一下每次买的型号
    return [[matPM[targetPM][CPU]/2,matPM[targetPM][MEM]/2],[matPM[targetPM][CPU]/2,matPM[targetPM][MEM]/2],set()]


with open("./training-3.txt", "r") as f:
    typePM = int(f.readline())
    matPM = dict()
    currMax = None
    for i in range(typePM):
        currLine = f.readline().strip("()\n").split(',')
        cpu, mem, price, maintain = int(currLine[1]), int(currLine[2]), int(currLine[3]), int(currLine[4])
        matPM[currLine[0]] = [cpu, mem, price, maintain]
        if currMax == None:
            currMax = [currLine[0],physizeAlg(cpu,mem)]
        elif currMax[1] < physizeAlg(cpu,mem):
            currMax = [currLine[0],physizeAlg(cpu,mem)]
    targetPM = currMax[0]

    print(matPM)

    typeVM = int(f.readline())
    matVM = dict()
    for i in range(typeVM):
        currLine = f.readline().strip("()\n").split(',')
        cpu, mem, node = int(currLine[1]), int(currLine[2]), int(currLine[3])
        matVM[currLine[0]] = [cpu, mem, node]
    print(matVM)

    days = int(f.readline())

    

    for i in range(days):
        todayReq = int(f.readline())
        opAdd = np.zeros((todayReq,6)) # cpu mem node vmID vsize index
        opAddIndex = 0
        opDel = np.zeros((todayReq,1))
        opDelIndex = 0
        for request in range(todayReq):
            currLine = f.readline().strip("()\n").split(',')
            op = currLine[0]
            if op == 'add':
                buyType, vmID = currLine[1].strip(), int(currLine[2])
                opAdd[opAddIndex] = matVM[buyType] + [vmID,0,opAddIndex]
                opAddIndex += 1

            else:
                opDel[opDelIndex] = int(currLine[1])
                opDelIndex += 1
        
        vsizeAlg(opAdd)
        opAdd = opAdd[np.argsort(-opAdd[:, VSIZE])]

        
        vm2pm = dict()
        pm2vm = []
        numPM = len(pm2vm)
        pm2vm.append(buyPM(matPM))
        for i in range(opAddIndex):
            for j,eachPM in enumerate(pm2vm) :
                if opAdd[i,NODE] == 0: # single node
                    whichNode = None
                    if physizeAlg(eachPM[NODE_A][CPU],eachPM[NODE_A][MEM]) <= physizeAlg(eachPM[NODE_B][CPU],eachPM[NODE_B][MEM]):
                        whichNode = 1
                    else:
                        whichNode = 0
                    if eachPM[whichNode][CPU] >= opAdd[i,CPU] and eachPM[whichNode][MEM] >= opAdd[i,MEM]:
                        eachPM[whichNode][CPU] -= opAdd[i,CPU]
                        eachPM[whichNode][MEM] -= opAdd[i,MEM]
                        eachPM[VMSET].add(opAdd[i,IDVM])
                        vm2pm[opAdd[i,IDVM]] = [j,whichNode]

                        break
                    else:
                        pass # 看下一个物理机
                else:
                    if eachPM[NODE_A][CPU] >= opAdd[i,CPU]/2 and eachPM[NODE_A][MEM] >= opAdd[i,MEM]/2 and \
                        eachPM[NODE_B][CPU] >= opAdd[i,CPU]/2 and eachPM[NODE_B][MEM] >= opAdd[i,MEM]/2:
                        eachPM[NODE_A][CPU] -= opAdd[i,CPU]/2
                        eachPM[NODE_A][MEM] -= opAdd[i,MEM]/2
                        eachPM[NODE_B][CPU] -= opAdd[i,CPU]/2
                        eachPM[NODE_B][MEM] -= opAdd[i,MEM]/2
                        eachPM[VMSET].add(opAdd[i,IDVM])
                        vm2pm[opAdd[i,IDVM]] = [j]
            else:
                pm2vm.append(buyPM(matPM)) # TODO 判断是否一定放得下
                j = len(pm2vm) - 1
                if opAdd[i,NODE] == 0: # single node
                    pm2vm[j][NODE_A][CPU] -= opAdd[i,CPU]
                    pm2vm[j][NODE_A][MEM] -= opAdd[i,MEM]
                    pm2vm[j][VMSET].add(opAdd[i,IDVM])
                    vm2pm[opAdd[i,IDVM]] = [j,NODE_A]
                else:
                    pm2vm[j][NODE_A][CPU] -= opAdd[i,CPU]/2
                    pm2vm[j][NODE_A][MEM] -= opAdd[i,MEM]/2
                    pm2vm[j][NODE_B][CPU] -= opAdd[i,CPU]/2
                    pm2vm[j][NODE_B][MEM] -= opAdd[i,MEM]/2
                    pm2vm[j][VMSET].add(opAdd[i,IDVM])
                    vm2pm[opAdd[i,IDVM]] = [j]
        
        '''
        (purchase, 2) 本日物理机购买数目
        (NV603, 1)
        (NV604, 1)
        (migration, 0) 迁移数目
        (0, A) 部署
        (0, B)
        '''
        outPMNum = '(purchase, %d)'
        outPMType = '(%s, %d)'
        outMigration = '(migration, %d)'
        outDeploySingle = '(%d, %s)'
        outDeployDouble = '(%d)'
        print(outPMNum % (len(pm2vm) - numPM))
        numPM = len(pm2vm)

        print(outPMType % (targetPM,len(pm2vm) - numPM))

        print(outMigration % 0)

        opAdd = opAdd[np.argsort(opAdd[:, IDVM])]
        for i in range(opAddIndex):
            if len(vm2pm[opAdd[i,IDVM]]) == 1:
                print(outDeployDouble % tuple(vm2pm[opAdd[i,IDVM]]))
            else:
                if vm2pm[opAdd[i,IDVM]][1] == NODE_A:
                    print(outDeploySingle % (vm2pm[opAdd[i,IDVM]][0],'A'))
                else:
                    print(outDeploySingle % (vm2pm[opAdd[i,IDVM]][0],'B'))














        i = 1

        




    

i = 0