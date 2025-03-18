import subprocess
import os, sys, re

exePath = "Tag_18_06_27_C/MV2/binaries/linux/MV2Host"
schemaXMLFile = "Tag_18_06_27_C/MV2/host/script/MV2ScriptSchema.xsd"
comPort = "/dev/ttyUSB0"
mxrFolderPath = "./mxr_files/"

def getDigitalBField(scriptXMLFile, schemaXMLFile=schemaXMLFile):
    result = subprocess.call([exePath, scriptXMLFile, schemaXMLFile, comPort, getNewMXRPath()])
    return result

def modifyScriptXML(scriptXMLFile, newScriptXMLFile, changes={}):
    with open(scriptXMLFile, 'r') as f:
        lines = f.readlines()
    for key, value in changes.items():
        modifyXMLsnippet(key,value,lines)
    with open(newScriptXMLFile, 'w') as f:
        f.writelines(lines)

def modifyXMLsnippet(type,value,lines):
    for i,line in enumerate(lines):
        m = re.search(f"<type>{type}</type>",line)
        if m:
            lines[i+1] = re.sub("<value>.*</value>",f"<value>{value}</value>",lines[i+1])

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
    scriptfile = "Tag_18_06_27_C/MV2/host/script/MV2DigitalScript.xml"
    newScriptfile = "Tag_18_06_27_C/MV2/host/script/MV2DigitalScript_new.xml"
    modifyScriptXML(scriptfile,newScriptfile,changes={"2C":"fish"})
