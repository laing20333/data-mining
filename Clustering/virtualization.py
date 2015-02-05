__author__ = 'Ben'

import pandas as pd
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt

# data preprocessing
df = pd.read_csv('data1_3_ma.txt', sep=',')
df = df.drop(['review/userId'], axis = 1)
df = df.fillna(0)
df = df.replace('?', 0)

for attr_name in df.columns:
    df[attr_name] = df[attr_name].astype(float)

# pca
pca = PCA(n_components=2)
vector = pca.fit(df).transform(df)

# visualization
plt.figure()
plt.scatter(vector[:, 0], vector[:, 1], c='b')
plt.show()
