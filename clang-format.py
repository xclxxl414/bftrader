import os
import os.path
import subprocess

extList = set(["cpp","cc","h"])
cppDir = ["base","tools","allinone","ctpgateway","datafeed","ctaengine","dataframe","btgateway","assist"]

def clangformat(filePath):
    cmdline = ["clang-format.exe","-style","WebKit",filePath]
    child = subprocess.Popen(cmdline,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    output,error = child.communicate()
    if error:
        print error
        return
    with open(filePath,"wb") as f:
        f.write(output)
    
def formatDir(dirPath):
    for root,dirs,files in os.walk(dirPath):
        for name in files:
            if name.lower().endswith(".pb.cc"):
                continue
            if name.lower().endswith(".pb.h"):
                continue              
            ext = os.path.splitext(name)[1][1:]
            if ext.lower() in extList:
                cpp = os.path.join(root, name)
                print cpp
                clangformat(cpp)
        
if __name__ == '__main__':
    for path in cppDir:
        formatDir(path)
    