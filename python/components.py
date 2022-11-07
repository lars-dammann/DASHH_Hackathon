from dataclasses import dataclass
from tabulate import tabulate

secsPerHour = 60 * 60
secsPerDay  = 60 * 60 * 24
secsPerYear = 60 * 60 * 24 * 365
eps = 1e-1

@dataclass
class SimResults:
    TotalTime: float = 0.0
    TimeShareBat: float = 0.0 # Time Share of Running only on Battery
    TimeShareEng: float = 0.0 # Time Share of Running on Battery and Engine
    TimeShareHybrid: float = 0.0 # Time Share of Running on Battery and Engine
    TimeShareLand: float = 0.0 # Time Share of Charging at Charge Point
    TimeShareRest: float = 0.0 # Time Share of (not Running and not charging)
    EnergyShareLand: float = 0.0 # Energy Share of Land Connection
    EnergyShareFuel: float = 0.0 # Energy Share of Fuel
    FuelConsumption: float = 0.0 # KWh of Fuel used
    InsufPwrT: float = 0.0 # Insufficient Power Duration in Seconds
    InsufPwrS1: float = 0.0 # Insufficient Power as Time Fraction of Job Time
    InsufPwrS2: float = 0.0 # Insufficient Power as Time Fraction of total Time
    CostFuel: float = 0.0
    CostElectricity: float = 0.0
    CostTotal: float = 0.0
    CO2Total: float = 0.0

    def display(self):
        time_table = str(tabulate([
        ['Total Time(d)', self.TotalTime / secsPerDay],
        ['Bat Share', self.TimeShareBat / self.TotalTime],
        ['Eng Share', self.TimeShareEng / self.TotalTime],
        ['Hybrid Share', self.TimeShareHybrid / self.TotalTime],
        ['Land Share', self.TimeShareLand / self.TotalTime],
        ['Idle Share', self.TimeShareRest / self.TotalTime]
        ], tablefmt='orgtbl')).splitlines()

        eva_table = str(tabulate([
        ['Total Cost', self.CostFuel + self.CostElectricity],
        ['Cost Electricity', self.CostElectricity],
        ['Cost Fuel', self.CostFuel],
        ['Insufficient Power(s)', self.InsufPwrT],
        ['Insufficient Power(%)', self.InsufPwrT / self.TotalTime]
        ], tablefmt='orgtbl')).splitlines()

        master_headers = ["Time", "Evaluation"]
        master_table = tabulate([list(item) for item in zip(time_table,eva_table)],
                            master_headers, tablefmt="simple")
        print(master_table)


@dataclass
class Record:
    time: int = 0
    load: float = 0.0 # load requirement over dt time units
    engPwr: float = 0.0 # power provided by engine over dt time units
    batPwr: float = 0.0 # power provided by battery over dt time units
    landPwr: float = 0.0 # power charged by the charge point over dt time units
    powerDeficit: float = 0.0  # power deficit over dt time units
    charge: float = 0.0 # SoC of battery in % of capacity
initalRecord = {
'time': 0,
'load': 0.0,
'engPwr': 0.0,
'batPwr': 0.0,
'landPwr': 0.0,
'powerDeficit': 0.0,
'charge': 0.0}

@dataclass
class ChargePoint:
    pluginDelay: int # Time required to start charging
    powerLimit: float # Chargespeed at chargepoint, KWh per second
    electricityPriceperKWh: float # in EUR
    electricityCO2EmissionperKWH: float # in
    # Coordinates in which ship is charging
    minLat: float
    maxLat: float
    minLon: float
    maxLon: float

    def display(self):
        print(tabulate([
        ['pluginDelay(s)', self.pluginDelay],
        ['Power Limit', self.powerLimit],
        ['Electricity Price per KWh', self.electricityPriceperKWh],
        ['Electricity CO2 per KWH', self.electricityCO2EmissionperKWH]
        ],
        headers=['Charge Point', ''], tablefmt='orgtbl'))

initialChargePoint = {
    'pluginDelay': 120,
    'powerLimit': 75,
    'electricityPriceperKWh': 0.08,
    'electricityCO2EmissionperKWH': 10,
    'minLat': 53.539,
    'maxLat': 53.541,
    'minLon': 9.878,
    'maxLon': 9.882}

@dataclass
class battery:
    capacity: float # Capacity of Battery in
    etaBat: float # Battery Efficiency (ignore for hackathon)
    BatteryCO2BackpackperKWH: float # CO2 emitted in Battery production per kWh Battery Capacity
    BatteryPricePerKWh: float # in EUR
    ChargeLimitSoc: float # Maximum allowed SoC
    DischargeLimitSoC: float # Minimum allowed SoC

    def display(self):
        print(tabulate([
        ['Capacity', self.capacity],
        ['Battery CO2Backpack per KWH', self.BatteryCO2BackpackperKWH],
        ['Battery Price Per KWh', self.BatteryPricePerKWh],
        ['Charge Limit Soc', self.ChargeLimitSoc],
        ['Discharge Limit SoC', self.DischargeLimitSoC]
        ],
        headers=['Battery', ''], tablefmt='orgtbl'))

initialbattery = {
    'capacity': 200,
    'etaBat': 0.95,
    'BatteryCO2BackpackperKWH': 123,
    'BatteryPricePerKWh': 280,
    'ChargeLimitSoc': 0.9,
    'DischargeLimitSoC': 0.1,
}

@dataclass
class engine:
    PwrMax: float # Maximum Engine Power
    FuelC02EmissionsperKWH: float # kg per kWh
    FuelPriceperKWH: float # kg CO2

    def display(self):
        print(tabulate([
        ['Power Maximum', self.PwrMax],
        ['Fuel C02 per KWH', self.FuelC02EmissionsperKWH],
        ['Fuel Price per KWH', self.FuelPriceperKWH]
        ],
        headers=['Engine', ''], tablefmt='orgtbl'))

initialengine = {
    'PwrMax':415,
    'FuelC02EmissionsperKWH': 0.25,
    'FuelPriceperKWH': 0.26
}

@dataclass
class ship:
    state: Record
    ChargePoint: ChargePoint
    battery: battery
    engine: engine
    def __post_init__(self):
        self.state = Record(**self.state)
        self.ChargePoint = ChargePoint(**self.ChargePoint)
        self.battery = battery(**self.battery)
        self.engine = engine(**self.engine)

    def display(self):
        print('##########################################')
        print('##########################################')
        print('Components Overview')
        print('##########################################')
        print('##########################################')
        self.battery.display()
        print('##########################################')
        self.engine.display()
        print('##########################################')
        self.ChargePoint.display()
        print('##########################################')
        print('##########################################')

initialship = {
'state':initalRecord,
'ChargePoint':initialChargePoint,
'battery':initialbattery,
'engine':initialengine}
