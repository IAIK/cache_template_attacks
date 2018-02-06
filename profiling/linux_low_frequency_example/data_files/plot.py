import matplotlib.pyplot as plt
import matplotlib.patches as mpatches

f = open('with_prftch.csv', 'r')
mat1 = []
for lines in f:
    mat1.append(lines.split(','))

f = open('no_prftch.csv', 'r')
mat2 = []
for lines in f:
    mat2.append(lines.split(','))

f = open('no_hw_prftch.csv', 'r')
mat3 = []
for lines in f:
    mat3.append(lines.split(','))

addrs = []

for row in mat1[1:]:
    addrs.append(row[0])

key = 0

values1 = []
values2 = []
values3 = []

for row in mat1[1:]:
    values1.append(float(row[key+1]))

for row in mat2[1:]:
    values2.append(float(row[key+1]))

for row in mat3[1:]:
    values3.append(float(row[key+1]))

plt.plot(range(1,3279), values1, 'r', label='With Prefetching')
plt.plot(range(1,3279), values2, 'b', label='Without any Prefetching')
plt.plot(range(1,3279), values3, 'g', label='Without Hardware Prefetching')
plt.title('Comparison of data for key "0"')
plt.xlabel('Addresses')
plt.ylabel('Ratio: #cache_hits / #key_pressed')
'''
red_patch = mpatches.Patch(color='red', label='With Prefetching')
blue_patch = mpatches.Patch(color='blue', label='Without any Prefetching')
green_patch = mpatches.Patch(color='green', label='Without Hardware Prefetching')
plt.legend(handles=[red_patch, blue_patch,green_patch])
'''

leg = plt.legend(loc='upper right', prop={'size':14})

for l in leg.get_lines():
	l.set_alpha(1)
	l.set_marker('.')
plt.show()
