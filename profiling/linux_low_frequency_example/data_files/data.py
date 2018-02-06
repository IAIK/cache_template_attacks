import numpy as np

def outliers_modified_z_score(ys):
    threshold = 10

    median_y = np.median(ys)
    median_absolute_deviation_y = np.median([np.abs(y - median_y) for y in ys])
    if median_absolute_deviation_y == 0:
	median_absolute_deviation_y = 0.03
    modified_z_scores = [0.6745 * (y - median_y) / median_absolute_deviation_y for y in ys]
    return np.where(np.abs(modified_z_scores) > threshold)


def outliers_iqr(ys):
#    print ys
    if len(ys) <= 0:
	return []
    quartiles = np.percentile(ys, [25, 75])
    quartile_3 = quartiles[1]
    quartile_1 = quartiles[0]
    iqr = quartile_3 - quartile_1
    lower_bound = quartile_1 - (iqr * 1.5)
    upper_bound = quartile_3 + (iqr * 1.5)
    if upper_bound == 0:
	upper_bound = 0.1
    return np.where((ys > upper_bound))


f = open("temp.csv", "r")

l = []
for line in f:
	a = line.split(',')
	if a[0] == '':
		l.append(line)
		continue
	npar = np.array([float(j) for j in a[2:]])
	outliers = outliers_modified_z_score(npar)
	if len(outliers) > 0 and len(outliers[0]) > 0:
		#oar = [npar[i] for i in outliers]
		#if min(oar) <= min(npar):
		#	continue
		flag = 0
		for i in outliers[0]:
			line = line[:-1] +  "," + str(a[2+i]) + line[-1]
			l.append(line)

print ''.join(i for i in l)
