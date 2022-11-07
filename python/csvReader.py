import pandas as pd
import time

def readCSV(fname):
    data = pd.read_csv(fname, sep=',')
    print(data)
    return data


# old
#def readCSV(fname):
#    data = pd.read_csv(fname, sep='|')
#    data = data[['TimeStamp', 'SOG', 'LAT', 'LON', 'Power']]
#    data.columns = ['time', 'sog', 'lat', 'lon', 'load']
#    data['time'] = data['time'].apply(lambda x: toUnixT(x))
#    offset = data['time'].iloc[0]
#    data['time'] = data['time'].apply(lambda x: int(x - offset))
#    return data

#def toUnixT(timestamp):
#    t = time.strptime(timestamp, '%d-%b-%Y %H:%M:%S')
#    return int(time.mktime(t))

#def getCharge(datapt, gf):
#    inFence = (datapt.lat >= gf['minLat'] and
#    datapt.lat <= gf['maxLat'] and
#    datapt.lon >= gf['minLon'] and
#    datapt.lon <= gf['maxLon'])
#    return (inFence and datapt.sog<= 1e-4)
