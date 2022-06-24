import urllib, urllib2, json, sys, os.path, getpass, time
import ssl

def getToken(host, port, user) :
   ltoken = os.path.normpath(os.path.expanduser("~/.klocwork/ltoken"))
   ltokenFile = open(ltoken, 'r')
   for r in ltokenFile :
        rd = r.strip().split(';')
        #for debuging purposes
        print(rd)
        ltokenFile.close()
        return rd[3]
   ltokenFile.close()

class Issue(object) :
    def __init__(self, attrs) :
        self.id = attrs["id"]
        self.message = attrs["message"]
        self.file = attrs["file"]
        self.method = attrs["method"]
        self.code = attrs["code"]
        self.severity = attrs["severity"]
        self.severityCode = attrs["severityCode"]
        self.state = attrs["state"]
        self.status = attrs["status"]
        self.taxonomyName = attrs["taxonomyName"]
        self.url = attrs["url"]
        self.created=time.ctime(attrs["dateOriginated"]/1000)

    def __str__(self) :
        return "[%d] %s\n\t%s | %s\n\tCode %s | Severity: %s(%d) | State: %s | Status: %s | Taxonomy: %s | Created: %s\n\t%s" % (
        self.id, self.message, self.file, self.method, self.code, self.severity, self.severityCode, self.state,
        self.status, self.taxonomyName, self.created, self.url
       )

def from_json(json_object) :
    if 'id' in json_object :
        return Issue(json_object)
    return json_object

host = "hf2skwapp002.ed.cps.intel.com"
port = 8105
user = "sys_kwzephyr"
project = "basic-zephyr-ec"
url = "https://%s:%d/review/api" % (host, port)
values = {"project": project, "user": user, "action": "search"}

loginToken = getToken(host, port, user)
if loginToken is not None :
   values["ltoken"] = loginToken

# New KW server has a SSL certificate issue
# Until this is fixed, we need to disable verification
ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE

values["query"] = "status:Analyze -status:Not a problem module:app"
data = urllib.urlencode(values)
req = urllib2.Request(url, data)
response = urllib2.urlopen(req, context=ctx)

try:
    f = open("kwloaddb.log", "wb")
    for record in response:
        print(json.loads(record, object_hook=from_json))
        f.write(str(json.loads(record, object_hook=from_json)))
except Exception, e:
    print e.message

f.close()
