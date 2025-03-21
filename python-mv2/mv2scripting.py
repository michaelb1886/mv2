import subprocess
import os, sys, re

exePath = "Tag_18_06_27_C/MV2/binaries/linux/MV2Host"
schemaXMLFile = "Tag_18_06_27_C/MV2/host/script/MV2ScriptSchema.xsd"
comPort = "/dev/ttyACM0"
mxrFolderPath = "./mxr_files/"

def applyScriptMV2Host(scriptXMLFile, schemaXMLFile=schemaXMLFile):
    result = subprocess.check_output([exePath, scriptXMLFile, schemaXMLFile, comPort, getNewMXRPath()])
    return [int(x) for x in result.decode("utf-8").rstrip().split(',')]

def modifyScriptXML(scriptXMLFile, newScriptXMLFile, keys=[], values=[]):
    with open(scriptXMLFile, 'r') as f:
        lines = f.readlines()
    modifyXMLsnippet(keys,values,lines)
    with open(newScriptXMLFile, 'w') as f:
        f.writelines(lines)

def modifyXMLsnippet(types,values,lines):
    k = 0
    for i,line in enumerate(lines):
        m = re.search(f"<type>{types[k]}</type>",line)
        if m:
            lines[i+1] = re.sub("<value>.*</value>",f"<value>{values[k]}</value>",lines[i+1])
            k += 1
            if k == len(types):
                break

def makeDigitalRegister(settings):
    
    """
    measurement axis table
    bit 0 | bit 1 | option
    =======================
    0     | 0     |  Bx
    0     | 1     |  By
    1     | 0     |  Bz
    1     | 1     |  T
    """
    match settings.get("measurement_axis", "Bx"):
        case "Bx":
            register1 = 0b00
        case "By":
            register1 = 0b01
        case "Bz":
            register1 = 0b10
        case "T":
            register1 = 0b11
        case _:
            raise ValueError("Invalid measurement range")
    
    
    """
    sensing range table
    bit 0 | bit 1 | range | option
    ===============================
    0     | 0     | 0.1 T |   0
    0     | 1     | 0.3 T |   1
    1     | 0     | 1 T   |   2
    1     | 1     | 3 T   |   3
    """

    
    match settings.get("sensing_range", 0):
        case 0:
            register2 = 0b00
        case 1:
            register2 = 0b01
        case 2:
            register2 = 0b10
        case 3:
            register2 = 0b11
        case _:
            raise ValueError("Invalid sensing range")
    
    
    """
    resolution table
    bit 0 | bit 1 | num bit | refresh rate | option"
    ================================================
    0     | 0     | 14      |  3 kHz       | 0
    0     | 1     | 15      |  1.5 kHz     | 1
    1     | 0     | 16      |  0.75 kHz    | 2
    1     | 1     | 16      |  0.375 kHz   | 3
    """

    match settings.get("resolution", 0):
        case 0:
            register3 = 0b00
        case 1:
            register3 = 0b01
        case 2:
            register3 = 0b10
        case 3:
            register3 = 0b11
        case _:
            raise ValueError("Invalid resolution")
        
    """
    measurment axis table
    bit 0 | bit 1 | axis | option
    ===============================
    0     | 0     | 3    | 0
    0     | 1     | x    | 1
    1     | 0     | y    | 2
    1     | 1     | z    | 3
    """
    
    match settings.get("output", 3):
        case 3:
            register4 = 0b00
        case "x":
            register4 = 0b01
        case "y":
            register4 = 0b10
        case "z":
            register4 = 0b11
        case _:
            raise ValueError("Invalid measurement axis")
    
    return hex(register4<<6 | register3<<4 | register2<<2 | register1)
              

def getNewMXRPath(name=None,mxrFolderPath=mxrFolderPath):
    if name is None:
        name = "newMXR_0.mxr"
    if not os.path.exists(mxrFolderPath):
        os.makedirs(mxrFolderPath)
    if not os.path.exists(mxrFolderPath + name):
        return mxrFolderPath + name
    else:
        i = 1
        sanity = 1000
        while os.path.exists(mxrFolderPath + "newMXR_" + str(i) + ".mxr"):
            i += 1
            if i > sanity:
                raise Exception("Too many files in the folder")
        return mxrFolderPath + "newMXR_" + str(i) + ".mxr"

if __name__ == "__main__":
    for axis in ["Bx", "By", "Bz"]:
        print(f"axis: {axis}\thex:{makeDigitalRegister({"output":3,
                                   "sensing_range":0,
                                   "resolution":3,
                                   "measurement_axis":axis})}")

    # x = applyScriptMV2Host("python-mv2/MV2DigitalScript_new.xml")

    modifyScriptXML("python-mv2/MV2DigitalScript_new.xml","python-mv2/MV2DigitalScript_temp.xml",
                    keys=["2C"]*4,
                    values=[str(makeDigitalRegister({"output":3,
                                                    "sensing_range":0,
                                                    "resolution":3,
                                                    "measurement_axis":"Bx"}))[-2:],
                            str(makeDigitalRegister({"output":3,
                                                    "sensing_range":0,
                                                    "resolution":3,
                                                    "measurement_axis":"Bx"}))[-2:],
                            str(makeDigitalRegister({"output":3,
                                                    "sensing_range":0,
                                                    "resolution":3,
                                                    "measurement_axis":"By"}))[-2:],
                            str(makeDigitalRegister({"output":3,
                                                    "sensing_range":0,
                                                    "resolution":3,
                                                    "measurement_axis":"Bz"}))[-2:]])
