#this is an attempt to improve the previous version using ML visualisation course
import matplotlib.pyplot as plt
import csv
import numpy as np
import pandas as pd


x=[]
y=[]
##Be careful as the first value of each row of csv are strings (headers) and they can cause issues if treated as normal values
#with open('c:/Users/abdel/Documents/GitHub/year4_project/microphone_test/test2_150kHz.csv','r') as csvfile:
#filename = 'c:/Users/abdel/Documents/GitHub/year4_project/microphone_test/newPItest.csv'
filename = 'newPItest3.csv'


df = pd.read_csv(filename)
#locate column of name 'values' and get all its values
audio= df.loc[:, ' value'].values #correct the c file later as there is a space before the , in the csv which causes problems
index= df.loc[:, 'index'].values
df.info()
print(type(audio))

plt.plot(index, audio, color='b', linestyle='-', label='audio sig')#label is used for legend

#plt.xticks(rotation = 90) #to rotate values in x axis
plt.xlabel('samples')
plt.ylabel('sound signal')
plt.legend(loc=(1.02,0))
plt.grid()
plt.show()

#     #next(df) #to skip header line or i can also slice the range of lines with: lines[1:5000] in the next line 
#     for row in df:
#         print(row)
#         x.append(row[0])
#         y.append(row[1])

# #section used to turn values to float and smooth with average
# for i in range(0,4999): #we avoid the 0 index as it is a header/string
#     y[i]=float(y[i])

# for i in range(1,4999):
#     if i==1:
#         print(y[i])
#     else:
#         y[i]=(y[i-1]+y[i])/2
#         print(y[i])

# #for t in y:
# #    print(t)

# plt.plot (x[0:5000],y[0:5000], color='g', linestyle='-')

# plt.ylim(0, 4000)
# plt.xlim(0,4999)
# #plt.axis([0, len(x), 0, 4095])
# plt.xlabel('samples')
# plt.ylabel('sound signal')
# plt.grid()
# plt.show()