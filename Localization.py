
import queue
from threading import Thread, Event
import matplotlib.pyplot as plt
import matplotlib.image as img
import math
import serial  # 引用pySerial模組
import numpy as np
import time
loc=[]
COM_PORT = 'COM3'    # 指定通訊埠名稱
BAUD_RATES = 115200    # 設定傳輸速率
ser = serial.Serial(COM_PORT, BAUD_RATES)   # 初始化序列通訊埠
#8.9 7.45 318
#
distABCD = [4,4,4,4]
# rectABCD = [[0.3,0.3],[8.5,0.3],[7.85,7.15],[1.5,7.15]] #318
rectABCD = [[1.9,0],[7.6,0],[7.8,7.34],[2.82,7.34]] #503


class location_thread(Thread):
    def __init__(self, q1,q2):
        Thread.__init__(self)
        self.q1 = q1
        self.q2 = q2
        self.stop_event = Event()
        self.count=0
    def run(self):
        print("thread join")
        qget = self.q1
        qput = self.q2
        while not self.stop_event.isSet():
            try:
                self.count+=1
                distance = qget.get(block=True, timeout=1)
                distABCDnew=compensation(distance)
                location=MinSigamaXY(rectABCD,distABCDnew)
                qput.put(location)
            except queue.Empty:
                print("empty queue")
                continue
            qget.task_done()
            qput.task_done()
        print ("thread end")
    def join(self, timeout=None):
        self.stop_event.set()
        Thread.join(self, timeout)

def Sigama(coord,rectABCD,distABCD):
    Ea=abs(math.sqrt(math.pow(coord[0]-rectABCD[0][0],2)+math.pow(coord[1]-rectABCD[0][1],2))-distABCD[0])
    Eb=abs(math.sqrt(math.pow(coord[0]-rectABCD[1][0],2)+math.pow(coord[1]-rectABCD[1][1],2))-distABCD[1])
    Ec=abs(math.sqrt(math.pow(coord[0]-rectABCD[2][0],2)+math.pow(coord[1]-rectABCD[2][1],2))-distABCD[2])
    Ed=abs(math.sqrt(math.pow(coord[0]-rectABCD[3][0],2)+math.pow(coord[1]-rectABCD[3][1],2))-distABCD[3])
    E=[Ea,Eb,Ec,Ed]
    zero=distABCD.count(0)
    if zero >= 2:
        sigama =1000
    elif zero==1:
        index=distABCD.index(0)
        E[index]=0
        sigama = sum(E)
    else:
        sigama = sum(E)
    
    return sigama
def goldratio(x1,x2):
    if x2>x1:
        k2=x1+0.618*(x2-x1)
        k1=x2-0.618*(x2-x1)
    else:
        k1=x1-0.618*(x1-x2)
        k2=x2+0.618*(x1-x2)
    return [k1,k2]
def MinSigamaXY(rectABCD,dist):
    width=[-5,10]
    length=[-5,10]
    times=0
    while times<15 :
        times+=1
        tempsig=[]
        w=goldratio(width[0],width[1])
        l=goldratio(length[0],length[1])
        for i in w:
            for j in l:
                tempsig.append(Sigama([i,j],rectABCD,dist))
        indexmin=tempsig.index(min(tempsig))
        #往左下
        if indexmin==0:
            width=[width[0],w[1]]#往左下縮小範圍
            length=[length[0],l[1]]
            location=[w[0],l[0]]
        #往左上
        elif indexmin==1:
            width=[width[0],w[1]]
            length=[l[0],length[1]]
            location=[w[0],l[1]]
        #往右下
        elif indexmin==2:
            width=[w[0],width[1]]
            length=[length[0],l[1]]
            location=[w[1],l[0]]
        #往右上
        elif indexmin==3:
            width=[w[0],width[1]]
            length=[l[0],length[1]]
            location=[w[1],l[1]]
        #tempmin = Sigama([i,j],rectABCD,dist)
    if min(tempsig)<1000:
        # print('location:',location)
        return location
    else:
        print("Doesn't have enough distance data")
        return [0,0]
def compensation(distance):
    dis=[]
    # height=1.3 #318
    height= 0.30 #503
    for x in distance:
        dis.append(x*(0.0005053*pow(x,3)-0.01344*pow(x,2)+0.1265*x+0.5114))
        # dis.append(x)
    for i,val in enumerate(dis):
        if val > height:
            dis[i]=math.sqrt(abs(val**2-height**2))
        # else:
        #     print(1)
        #     dis[i]=0
    print(dis)
    return dis
waitingdata = queue.Queue(5) #最多存放10筆
calculateddata =queue.Queue(5)
positioning_thread = location_thread(waitingdata,calculateddata)
positioning_thread.start()
count=0
plt.ion()
fig = plt.figure(figsize=(5,5))
fig.canvas.manager.window.wm_geometry('+500+30')
plt.xlim(0,7.8,0.1)
plt.ylim(0,7.3,0.1)
bgimg = img.imread('./background503.jpg')
ax=fig.add_subplot(111)
ax.imshow(bgimg, extent=[0, 7.8, 0,7.26])
loclist=[]
try:
    while True:
        while ser.in_waiting:          # 若收到序列資料…
            data_raw = ser.readline()  # 讀取一行
            data = data_raw.decode(errors='ignore')   # 用預設的UTF-8解碼
            datas=data.split(',')
            try:
                # differance=[abs(i-float(j))<5 for i,j in zip(distABCD,datas)]
                differance=[1,1,1,1]
                if sum(differance) >=3 :
                    for i,value in enumerate(differance):
                        if value:
                            datas[i]=datas[i].replace('\r','')
                            datas[i]=datas[i].replace('\n','')
                            distABCD[i]=float(datas[i])
                        else: 
                            distABCD[i]=0
            except ValueError:
                continue     
            print('origin data:',distABCD)
            waitingdata.put(distABCD)
            locs=calculateddata.get()
            print(locs)
            if locs != [0,0]:
                ser.write(str.encode(str(round(locs[0],1))+'/'+str(round(locs[1],1))+'\n'))   
            
            # print(locs)
            if len(loclist) <10:
                loclist.append(locs)
            else:
                loclist.pop(0)
                loclist.append(locs)
            ax.scatter([each[0] for each in loclist],[each[1] for each in loclist],c='red',marker='o')
            fig.canvas.draw()
            fig.canvas.flush_events()
            count+=1
            if count % 10 == 0:
                plt.clf()
                ax=fig.add_subplot(111)
                ax.imshow(bgimg, extent=[0, 7.8, 0, 7.26])
                plt.xlim(0,7.8,0.1)
                plt.ylim(0,7.3,0.1)
            # time.sleep(0.5)
except AttributeError:
    print('Not get the distance data.')
except KeyboardInterrupt:
    ser.close()    # 清除序列通訊物件
    print('再見！')


calculateddata.join()     
waitingdata.join() # block until all tasks are done
positioning_thread.join()
