def getEnginePower(S, load, upperSoC = 0.5, lowerSoC = 0.15):
    if S.engine.PwrMax == 0 or S.state.landPwr > 0:
        return 0

    minLoadToProvide = 0
    if S.battery.DischargeLimitSoC > S.state.charge:
        minLoadToProvide = load;

    if S.state.charge > upperSoC:
        return minLoadToProvide
    if S.state.charge < lowerSoC:
        return S.engine.PwrMax
    if S.state.engPwr:
        return S.engine.PwrMax
    return minLoadToProvide

def MaxEngine(S, load):
    return S.engine.PwrMax
