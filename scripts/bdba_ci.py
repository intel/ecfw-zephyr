""" BDBA Command Line scanner and reporter ReadMe
This script is intended to allow file scanning and reporting from command line,
and it was designed with the idea of continuously running it via a CI job during
development process instead of scanning a released application.
Note that the action of 'Scanning' for BDBA means to identify packages and
versions included in the uploaded application, as the matching against the list
of vulnerabilities happens on 'Report'. A 'Report' may change its result if a
new CVE is added to the database.
The 'scan' and 'report' functions are decoupled, times to scan files vary too
much and generates a lot of failed executions because the job cannot wait
forever or poll forever. Recommended usage is to schedule scans and reports via
cron and leave several hours before asking for the result, typical usage would
be: scan after business hours, report just before business hours. This way we
don't choke the BDBA server and we don't leave a CI agent/runner blocked until
the report is returned.

Requires:
  Python 3.6+
  requests
Usage:
<script> {scan,report} [parameters]

Common parameters to both scan and report:
  --username and --password, or --token (can be generated from BDBA interface)
  --host which defaults to bdba001.icloud.intel.com if not provided, do not
    include protocol (https://) here, it's not expected.

Scan parameters:
  --scan-file <path> or --scan-url <uri> are used to provide the file that will
    be uploaded to BDBA for Scanning
  --scan-group is the group ID number that you can see in the BDBA interface
    when browsing applications, for example:
      https://bdba001.icloud.intel.com/group/4/
    means you should use: --scan-group 4
  --save-id will write to disk a scan.id file that will save the application ID
    that has been generated. In CI, it's easier to capture this file as an
    artifact and pass it to a report stage

Scan returns:
  Application ID

Scan exit codes:
  0 <success>
  3 <scan could not be completed>

Report parameters:
  --scan-id or --scan-file can be used to set the target application we want to
    retrieve a report from. When using a file no argument is required as the
    name is expected to be scan.id.
  --threshold is the base score where a vulnerability will be considered. It
    defaults to 4 (excludes 'low'). Scores are based on CVSS V2 Ratings and
    further details can be found in the following link:
      https://nvd.nist.gov/vuln-metrics/cvss
    Mandatory requirement is to resolve all Critical or Major findings, see the
    following link (under "Details") for further reference:
      http://goto/sdl343
  --except-var or --except-url or --except-file give 3 different ways of
    providing an exclusion. Exclusions are important because they let us regain
    exit code 0, which will mark a CI job as successful once we have analyzed
    a vulnerability that may not apply for us, a patch has been scheduled, or
    other cases. This feature allows users of the report to focus on new issues
    that need attention by suppressing issues that have already been attended to
    The format of the exclusion JSON is shown below each vulnerability, and it's
    a strict matching, meaning that the library, the version, the cve and a hash
    made of all current paths have to match for the script to exclude the
    finding. If any of those values change, it will reappear in the report. If
    multiple exclusions are to be made, just append them inside the 'exceptions'
    tag of the JSON as another item.
    If you're using an environment variable, ensure that proper escaping is used
    or enclose it in single quotes.
    Lastly, keep in mind that while it will make this script pass, the status in
    BDBA will remain unaltered. This is not meant to circumvent fixing issues,
    but rather allow an easier monitoring of the package health.
    * Triaged components are excluded automatically.

Report returns:
  URL for direct linking with BDBA interface
  Status if no vulnerabilities were found, if BDBA asks for Verify or vulnerable
    components are present
  List of vulnerabilities with component name, version information, CVE details
    and the item you can use to exclude it from following reports
  List of components with NO version associated. When BDBA can't automatically
    detect the version number and the component has (or had) vulnerabilities it
    will be listed here and will impact the exit code

Report exit codes:
  0 <success>
  1 <BDBA requires manual attention due to vulnerabilities>
  3 <BDBA failed to scan the application, this script failed, or a there is a
    problem with a parameter>

Example of exceptions JSON, (if used in a variable, make it a signle line):
    {"exceptions": [
        {
        "cve": "CVE-2019-9740",
    	"lib": "python",
    	"vers": "2.7.16",
    	"hash": "0fb124626f4e83f43aff742624e1998487f5a797"
    	},{
    	"cve": "CVE-2018-20406",
    	"lib": "python",
    	"vers": "2.7.16",
    	"hash": "0fb124626f4e83f43aff742624e1998487f5a797"
    	},{
    	"cve": "CVE-2019-9947",
    	"lib": "python",
    	"vers": "2.7.16",
    	"hash": "0fb124626f4e83f43aff742624e1998487f5a797"
    	}
    ]}

Changelog:
  * Fri May 17 2019 Gabriel Brussa <gabriel.w.brussa@intel.com>
  - Improved error messages
  - Added timeout (15 minutes for scan, 1 for report)
  - Fixed a bug when report was successful
  * Fri May 05 2019 Gabriel Brussa <gabriel.w.brussa@intel.com>
  - Initial drop. This script is based on Gregory Junker snippet posted at
    https://gitlab.devtools.intel.com/snippets/88

"""

import argparse
import requests
import sys
import json
import urllib3 # To suppress ssl warnings
import hashlib
import os
import time

if sys.platform.lower() == "win32":
    os.system('color')

debug = False

def logger(text, color='default', details=None):
    switcher = {
        'default': '\033[99m',
        'red': '\033[91m',
        'green': '\033[92m',
        'yellow': '\033[93m'
    }
    sys.stdout.write(switcher.get(color) + str(text) + '\033[00m' + '\n')
    if (details and debug):
        sys.stdout.write(str(details) + '\n')
    sys.stdout.flush()

# Extend requests authentication methods to support 'bearer'
from requests.auth import AuthBase
class BearerAuth(AuthBase):
    def __init__(self, token):
        self.token = token

    def __call__(self, r):
         r.headers['Authorization'] = f'Bearer {self.token}'
         return r

def main():
    # Disable SSL warnings globally, required to run without errors on non-Intel lab machines
    urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

    # Create a parser instance
    parse = argparse.ArgumentParser()

    # Add functions subparsers
    subparse = parse.add_subparsers(title='functions', dest='function')
    subparse.required = True
    parse_scan = subparse.add_parser('scan', help='triggers a package scan')
    parse_report = subparse.add_parser('report', help='gets a report of a scan')
    parse_report_pdf = subparse.add_parser('report_pdf', help='gets a pdf report of a scan')

    # Username/Password or Token auth and instance
    # done this way to avoid code duplication (due to subparser and parameter ordering)
    for subparser in [parse_scan, parse_report, parse_report_pdf]:
        subparser_auth = subparser.add_argument_group('authentication')
        subparser_auth.required = True
        subparser_auth.add_argument('--username', help='authenticate via username, requires --password')
        subparser_auth.add_argument('--password', help='authentication password, used with --username')
        subparser_auth.add_argument('--token', help='authenticate via token')
        subparser.add_argument('--host', help='provide the service host, defaults to bdba001.icloud.intel.com. Do NOT include protocol (https://) as it is not expected', default="bdba001.icloud.intel.com")

    # Scan parameters
    parse_scan_source = parse_scan.add_mutually_exclusive_group()
    parse_scan_source.required = True
    parse_scan_source.add_argument('--scan-file', help='upload and scan a locally stored file')
    parse_scan_source.add_argument('--scan-url', help='scan a file hosted in a web server')
    parse_scan.add_argument('--scan-group', help='provide the target group id', required=True)
    parse_scan.add_argument('--save-id', help='saves the product id as scan.id', action='store_true')

    # Report parameters
    parse_report_id = parse_report.add_mutually_exclusive_group()
    parse_report_id.required = True
    parse_report_id.add_argument('--scan-id', help='provide a product id')
    parse_report_id.add_argument('--scan-file', help='use scan.id file', action='store_true')
    parse_report.add_argument('--threshold', help='provide a cvs score threshold, findings below the threshold are ignored, defaults to 4', type=int, default=4)
    parse_report_exclusion = parse_report.add_mutually_exclusive_group()
    parse_report_exclusion.add_argument('--except-var', help='except vulns via the provided environment variable name')
    parse_report_exclusion.add_argument('--except-url', help='except vulns via the provided url accessible JSON')
    parse_report_exclusion.add_argument('--except-file', help='except vulns via the provided locally stored JSON file')

    # PDF report parameters
    parse_report_pdf_id = parse_report_pdf.add_mutually_exclusive_group()
    parse_report_pdf_id.required = True
    parse_report_pdf_id.add_argument('--scan-file', help='use scan.id file', action='store_true')

    # Parse provided arguments
    args = parse.parse_args()
    if args.username and args.password is None:
        parse.error("--username requires --password")
    if args.password and args.username is None:
        parse.error("--password requires --username")
    if args.token and args.username:
        parse.error("--username cannot be used with --token")
    if args.username is None and args.token is None:
        parse.error("authentication has to be provided")
    # Generate auth header
    if args.token:
        auth_header = BearerAuth(args.token)
    else:
        auth_header = requests.auth.HTTPBasicAuth(args.username, args.password)


    if (args.function == "scan"):
        logger('Starting to do a scan...')
        if args.scan_file:
            try:
                scan_id = scan_file(args.host, auth_header, args.scan_file, args.scan_group)
            except Exception as error:
                logger('A fatal error happened while trying to scan the file, check parameters and BDBA console', 'red', error)
                exit(3)
        elif args.scan_url:
            try:
                scan_id = scan_url(args.host, auth_header, args.scan_url, args.scan_group)
            except Exception as error:
                logger('A fatal error happened while trying to scan the url, check parameters and BDBA console', 'red', error)
                exit(3)
        else:
            logger('Something went wrong', 'red')
            sys.exit(3)
        logger(f'Product ID: {scan_id}')
        if (args.save_id):
            # create the output file
            f = open('scan.id', "w")
            f.write(str(scan_id))
            f.close()

    elif (args.function == "report_pdf"):
        logger('Fetching PDF report...', 'green')
        if args.scan_file:
            scan_id = 0
            try:
                f = open('scan.id', "r")
                scan_id = f.read()
            except:
                logger('A fatal error happened while trying to open scan.id file', 'red')

            try:
                result = fetch_pdf_report(args.host, auth_header, scan_id)
            except Exception as error:
                logger('A fatal error happened while trying to retrieve the pdf report, check parameters and BDBA console', 'red', error)
                logger(f'Error:  {error.message}')
                sys.exit(3)
            f.close()

            report_name = "ksc_scan_{0}.pdf".format(scan_id)
            try:
                pdf = open(report_name, 'wb')
                pdf.write(result)
            except:
                logger('A fatal error happened while trying to create report', 'red')
            pdf.close()

        else:
            logger('Something went wrong', 'red')
            sys.exit(3)

    elif (args.function == "report"):
        logger('Fetching Exclusions (if any)...', 'green')
        exceptions = results_exceptions_load(args)
        logger('Fetching Report...', 'green')
        if args.scan_id:
            try:
                result = fetch_results(args.host, auth_header, args.scan_id)
            except Exception as error:
                logger('A fatal error happened while trying to read the report, check parameters and BDBA console', 'red', error)
                sys.exit(3)
        elif args.scan_file:
            try:
                f = open('scan.id', "r")
            except:
                logger('A fatal error happened while trying to open scan.id file', 'red')
            try:
                result = fetch_results(args.host, auth_header, f.read())
            except Exception as error:
                logger('A fatal error happened while trying to read the report, check parameters and BDBA console', 'red', error)
                sys.exit(3)
            f.close()
        else:
            logger('Something went wrong', 'red')
            sys.exit(3)

        if result['results']['status'] == 'B':
            logger('Results are not yet available, please retry later', 'yellow')
            sys.exit(3)
        elif result['results']['status'] == 'F':
            logger('Scan failed. Please refer to BDBA console', 'red')
            sys.exit(1)
        elif result['results']['status'] == 'R':
            logger('Parsing results...', 'green')
            # return vuln status
            status = results_status(result)
            # if vuln show details
            if (status != 0):
                detailed_status = results_parse(result, args.threshold, exceptions)
            else:
                detailed_status = 0
            # list components with no version
            noVersion = results_noVersion(result)
            if (noVersion == 1 or detailed_status != 0):
                sys.exit(1)
        else:
            logger('Could not parse results. Check BDBA console', 'red')
            sys.exit(3)

def scan_file(api_host, api_auth, file_name, group_id):
    try:
        data = open(file_name, 'rb')
    except:
        logger('Could not open or find selected file', 'red')
        sys.exit(3)
    try:
        response = requests.put(
            f'https://{api_host}/api/upload/{file_name}',
            headers = {
                'Delete-Binary': 'true',
                'Force-Scan': 'true',
                'Group': str(group_id)
            },
            auth=api_auth,
            data=data,
            verify=False,
            timeout=900
        )
        response.raise_for_status()
    except requests.exceptions.HTTPError as error:
        logger (f'An http error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.RequestException as error:
        logger ('An unexpected error has occurred', 'red', error)
        sys.exit(3)
    except requests.exceptions.ConnectionError as error:
        logger (f'A connection error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.Timeout as error:
        logger (f'Request has timed out after 15 minutes', 'red', error)
        sys.exit(3)
    data.close()
    obj = json.loads(response.content)
    return obj['results']['product_id']

def scan_url(api_host, api_auth, url, group_id):
    try:
        response = requests.post(
            f'https://{api_host}/api/fetch/',
            headers = {
                'Url': str(url),
                'Force-Scan': 'true',
                'Group': str(group_id)
            },
            auth=api_auth,
            verify=False,
            timeout=900
        )
        response.raise_for_status()
    except requests.exceptions.HTTPError as error:
        logger (f'An http error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.RequestException as error:
        logger ('An unexpected error has occurred', 'red', error)
        sys.exit(3)
    except requests.exceptions.ConnectionError as error:
        logger (f'A connection error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.Timeout as error:
        logger (f'Request has timed out after 15 minutes', 'red', error)
        sys.exit(3)
    obj = json.loads(response.content)
    return obj['results']['product_id']

def fetch_pdf_report(api_host, api_auth, product_id):
    url = f'https://{api_host}/api/product/{product_id}/pdf-report'
    logger(f'Retrieving pdf from https://{api_host}/products/{product_id}/pdf-report', 'green')
    time.sleep(5)
    try:
        response = requests.get(
            url,
            auth=api_auth,
            verify=False,
            timeout=60
        )
        response.raise_for_status()
    except requests.exceptions.HTTPError as error:
        logger (f'An http error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.RequestException as error:
        logger ('An unexpected error has occurred', 'red', error)
        sys.exit(3)
    except requests.exceptions.ConnectionError as error:
        logger (f'A connection error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.Timeout as error:
        logger (f'Request has timed out after 1 minute', 'red', error)
        sys.exit(3)

    try:
        obj = json.loads(response.content)
        result = obj['results']['product_id']
        status = result['results']['status']
        logger(f'Retrieve status: ', 'green', status )
        if status == 'B':
            logger(f'Busy', 'red', error)
    except:
        pass

    return response.content


def fetch_results(api_host, api_auth, product_id):
    url = f'https://{api_host}/api/product/{product_id}'
    logger(f'API host : {api_host}')
    logger(f'API auth : {api_auth}')
    logger(f'PID : {product_id}')
    logger(f'Retrieving results for https://{api_host}/products/{product_id}', 'green')
    try:
        response = requests.get(
            url,
            auth=api_auth,
            verify=False,
            timeout=60
        )
        response.raise_for_status()
    except requests.exceptions.HTTPError as error:
        logger (f'An http error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.RequestException as error:
        logger ('An unexpected error has occurred', 'red', error)
        sys.exit(3)
    except requests.exceptions.ConnectionError as error:
        logger (f'A connection error has occurred: {error}', 'red')
        sys.exit(3)
    except requests.exceptions.Timeout as error:
        logger (f'Request has timed out after 1 minute', 'red', error)
        sys.exit(3)
    return json.loads(response.content)

def results_noVersion(result):
    title_shown = 0
    for component in result['results']['components']:
        if (component['version'] is None and component['vuln-count']['total'] != 0):
            if (title_shown == 0):
                logger('Components with NO version and Vulnerabilities:', 'green')
                title_shown = 1
            logger(component['lib'])
    if (title_shown == 1):
        logger('Please review BDBA console as there are vulnerable components with no associated version', 'red')
        return 1
    else:
        return 0

def results_status(result):
    if (result['results']['summary']['verdict']['short'] == 'Verify'):
        logger(result['results']['summary']['verdict']['detailed'], 'red')
        return 1
    elif (result['results']['summary']['verdict']['short'] == 'Vulns'):
        logger(result['results']['summary']['verdict']['detailed'], 'red')
        return 1
    elif (result['results']['summary']['verdict']['short'] == 'Pass'):
        logger(result['results']['summary']['verdict']['detailed'], 'green')
        return 0
    else:
        return 2

def results_parse(result, threshold, exceptions):
    return_code = 0
    for component in result['results']['components']:
        if (component['vuln-count']['total'] > 0 and component['vuln-count']['total'] != component['vuln-count']['historical']):
            title_shown = 0

            for vuln in component['vulns']:
                vuln_excepted = 0
                vuln_detail = vuln['vuln']

                if (vuln_detail['cvss'] >= threshold and 'triage' not in vuln and vuln['exact'] != 0):
                    if (title_shown == 0):
                        if ('latest_version' in component):
                            latest_version_tag = "latest_version"
                        else:
                            latest_version_tag = "latest-version"
                        logger(f"\nComponent: {component['lib']} {component['version']} (latest version: {component[latest_version_tag]})", "red")
                        title_shown = 1
                    for exception in exceptions['exceptions']:
                        # check if the library, cve and version match for the exception (strict checking).
                        if (component['lib'] == exception['lib'] and vuln_detail['cve'] == exception['cve'] and component['version'] == exception['vers']):
                            # this vuln is potentially excepted
                            # generate a hash of all paths to compare with the exception
                            if (exception['hash'] == results_paths_hash(component['extended-objects'])):
                                logger(f"    EXCEPTION: {vuln_detail['cve']}: {vuln_detail['summary']}", "green")
                                vuln_excepted = 1
                    if (vuln_excepted == 0):
                        logger(f"    {vuln_detail['cve']}: {vuln_detail['summary']}", "red")
                        mock_exception = results_exceptions_mock()

                        mock_exception['exceptions'][0]['cve'] = vuln_detail['cve']
                        mock_exception['exceptions'][0]['lib'] = component['lib']
                        mock_exception['exceptions'][0]['vers'] = component['version']
                        mock_exception['exceptions'][0]['hash'] = results_paths_hash(component['extended-objects'])

                        logger(f"Exclude this finding in all current paths using: \n {json.dumps(mock_exception)}")
                        return_code = 1
    return return_code

def results_exceptions_load(args):
    if args.except_var:
        return results_exceptions_var(args.except_var)
    elif args.except_url:
        return results_exceptions_url(args.except_url)
    elif args.except_file:
        return results_exceptions_file(args.except_file)
    else:
        return results_exceptions_mock()

def results_exceptions_file(filename):
    try:
        f = open(filename, "r")
        exceptions = json.loads(f.read())
        f.close()
        return exceptions
    except:
        logger("Fatal error while trying to get exceptions from file", "red")
        exit(3)

def results_exceptions_url(url):
    try:
        return requests.get(url).json()
    except:
        logger("Fatal error while trying to get exceptions from url", "red")
        exit(3)

def results_exceptions_var(varname):
    try:
        return json.loads(os.environ[varname])
    except:
        logger("Fatal error while trying to get exceptions from variable", "red")
        exit(3)

def results_exceptions_mock():
    return json.loads('{"exceptions":[{"cve": null,"lib": null,"vers": null,"hash": null}]}')

def results_paths_hash(extended_objects):
    string = "/"
    for extended_object in extended_objects:
        for path in extended_object['fullpath']:
            string = string + path
    hash_string = hashlib.sha1(string.encode('utf-8'))
    return hash_string.hexdigest()

main()
