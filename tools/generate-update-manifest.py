from subprocess import Popen, PIPE
import argparse
import pprint
import os
import sys
import json

def run(args, cwd):
    proc = Popen(args, stdout=PIPE, stderr=PIPE, cwd=cwd)
    out, err = proc.communicate()
    exitcode = proc.returncode

    if exitcode != 0:
        print("\n'%s' failed with code %d (in %s)" % (" ".join(args), exitcode, cwd))
        print(err.decode("ascii"))

        sys.exit(exitcode)

    return out.decode("ascii")

# vars
tools_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.realpath(os.path.join(tools_dir, "../.."))
certs_dir = os.path.realpath(os.path.join(root_dir, "certs"))

# parse args
parser = argparse.ArgumentParser()
parser.add_argument("firmware_url")
args = parser.parse_args()

# load base
if not os.path.isfile(os.path.join(certs_dir, "manifest-base.json")):
    print("Could not find manifest-base.json in certs directory. Did you not run 'generate-update-certs.py'?")
    sys.exit(1)

if not os.path.isfile(os.path.join(root_dir, "combined.bin")):
    print("Could not find combined.bin in '%s'. Did you compile the application and combine it with bootloader?" % root_dir)
    sys.exit(1)

print("[1/3] Generating manifest JSON file")

with open(os.path.join(certs_dir, "manifest-base.json")) as manifest_base:
    manifest = json.load(manifest_base)

manifest["payloadUri"] = args.firmware_url
manifest["certificates"][0]["file"] = "certs/certificate.der"

with open(os.path.join(root_dir, "combined-manifest.json"), "w") as manifest_file:
    json.dump(manifest, manifest_file)

print("[2/3] Creating manifest")

run(["manifest-tool", "create", "-i", "combined-manifest.json", "-o", "simple-cloud-client-example.manifest", "-k", "certs/key.pem"], root_dir)

os.remove(os.path.join(root_dir, "combined-manifest.json"))

print("[3/3] Finished. Your manifest is generated at '%s/simple-cloud-client-example.manifest'" % root_dir)
