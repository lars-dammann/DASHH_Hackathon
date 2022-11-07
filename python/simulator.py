import pandas as pd
import controlStrategy
import components
import csvReader
import matplotlib.pyplot as plt

secsPerHour = 60 * 60
secsPerDay  = 60 * 60 * 24
secsPerYear = 60 * 60 * 24 * 365
eps = 1e-1

def simulate(ship, data):
    results = components.SimResults()

    # Defines a starting state
    r = components.Record()
    r.charge = 0.2
    ship.state = r

    # rvec is used to record results
    rvec = []

    # Defines variables and geofence
    timeSincePlugin = int(0)
    dt = int(10)
    time = int(0)
    gf = {'minLat':ship.ChargePoint.minLat,
        'maxLat':ship.ChargePoint.maxLat,
        'minLon':ship.ChargePoint.minLon,
        'maxLon':ship.ChargePoint.maxLon}

    # Simulation loop
    for index, row in data.iterrows():
        load = row['load']
        canCharge = row['charge']

        currentRecord = components.Record()
        currentRecord.load = load # Get current load requirement
        currentRecord.time = row['time'] # Get current time
        currentRecord.charge = ship.state.charge # Get current charge

        time += dt
        results.TotalTime += dt

        if canCharge:
            timeSincePlugin += dt
        else:
            timeSincePlugin = 0

        if timeSincePlugin >= ship.ChargePoint.pluginDelay:
            if ship.state.charge < ship.battery.ChargeLimitSoc:
                # If plugged in long enough and state of charge (SoC) is below charge limit, do charge
                addedCharge = ship.ChargePoint.powerLimit * dt / secsPerHour
                addedChargePercentage = addedCharge / ship.battery.capacity
                currentRecord.landPwr = ship.ChargePoint.powerLimit
                currentRecord.charge += addedChargePercentage
                if addedCharge > 0:
                    results.TimeShareLand += dt
                else:
                    results.TimeShareRest += dt
                results.EnergyShareLand += addedCharge
            else:
                results.TimeShareRest += dt
        elif timeSincePlugin > 0:
            results.TimeShareRest += dt


        #EnginePwr = controlStrategy.getEnginePower(ship, load) # Control Strategy is called
        EnginePwr = controlStrategy.MaxEngine(ship, load) # Control Strategy is called

        BatteryPwr = load - EnginePwr # Rest of load is covered by battery. Note: If engine provides more power than load, then the battery is charged

        currentRecord.engPwr = EnginePwr;
        if ship.state.charge <= ship.battery.DischargeLimitSoC and BatteryPwr > 0: # We cannot get power from battery if it is below discharg limit
            BatteryPwr = 0

        currentRecord.batPwr = BatteryPwr;

        # Everything that follows is update of record
        if not canCharge:
          if (BatteryPwr + EnginePwr) < load: # If we cannot cover load, then power deficit
            currentRecord.powerDeficit = load - BatteryPwr - EnginePwr
            results.InsufPwrT += dt
          else:
              engineRunning = False

              if EnginePwr >= eps:
                engineRunning = True

              batteryDischarging = False

              if BatteryPwr >= eps:
                batteryDischarging = True

              batteryCharging = False

              if BatteryPwr <= -eps:
                batteryCharging = True

              if engineRunning and batteryDischarging:
                  results.TimeShareHybrid  += dt
              if (not engineRunning) and batteryDischarging:
                  results.TimeShareBat     += dt
              if engineRunning and (not batteryDischarging):
                  results.TimeShareEng     += dt
              if (not engineRunning) and (not batteryDischarging):
                  results.TimeShareRest    += dt

        results.EnergyShareFuel += EnginePwr * dt/secsPerHour;
        results.FuelConsumption += EnginePwr * dt/secsPerHour
        chargeChange = (BatteryPwr * dt / secsPerHour) / ship.battery.capacity;
        currentRecord.charge -= chargeChange
        rvec.append(currentRecord)
        ship.state = currentRecord

    CostFuel: float = 0.0
    CostElectricity: float = 0.0
    CostTotal: float = 0.0
    CO2Fuel: float = 0.0
    CO2Electricity: float = 0.0
    CO2Total: float = 0.0

    results.CO2Fuel = results.FuelConsumption * ship.engine.FuelC02EmissionsperKWH
    results.CO2Electricity += results.EnergyShareLand * ship.ChargePoint.electricityCO2EmissionperKWH

    results.CostFuel = results.FuelConsumption * ship.engine.FuelPriceperKWH
    results.CostElectricity = results.EnergyShareLand * ship.ChargePoint.electricityPriceperKWh

    results.CostTotal = results.CostFuel + results.CostElectricity
    results.CO2Total = results.CO2Fuel + results. CO2Electricity

    rvec = pd.DataFrame(rvec)
    return {'results': results, 'record': rvec}


def writeRecord(record, fname):
    ofname = fname.split('data',1)[1].split(".",1)[0][1:] + '_record.csv'
    record.to_csv('../output/' + ofname, index=False)


#def writeResult(results, fname):
#    ofname = fname.split('data',1)[1].split(".",1)[0][1:] + '_results.csv'
#    rvec.to_csv('../output/' + ofname, index=False)
