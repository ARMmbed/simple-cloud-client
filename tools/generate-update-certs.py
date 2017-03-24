from subprocess import Popen, PIPE
import os
import pprint
import sys
import re
import argparse
import uuid
import struct
from random import randint

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
certs_dir = os.path.realpath(os.path.join(tools_dir, "../..", "certs"))
update_cert_file = os.path.realpath(os.path.join(tools_dir, "../..", "update_default_resources.c"))

# arguments

parser = argparse.ArgumentParser()
parser.add_argument("-v", "--vendor-domain", required=True)
parser.add_argument("-m", "--device-model", required=True)
parser.add_argument("-d", "--device-id", required=False)
args = parser.parse_args()

print("\033[93mWARNING! This script generates *insecure* certificates. Do not deploy these certificates into production.\033[0m")

# sanity checking
if os.path.isdir(certs_dir):
    print("Certificate directory (%s) already exists" % certs_dir)
    sys.exit(1)

if os.path.isfile(update_cert_file):
    print("Update cert file (%s) already exists" % update_cert_file)
    sys.exit(1)

# 1. install the dependencies for manifest-tool...
print("[1/6] Installing manifest-tool dependencies")

out = run(["python", "setup.py", "install"], os.path.join(tools_dir, "manifest-tool"))

# 2. creating certificate
print("[2/6] Creating certificates")
os.mkdir(certs_dir)

out = run(["openssl", "ecparam", "-genkey", "-name", "prime256v1", "-out", "key.pem"], certs_dir)
out = run(["openssl", "req", "-new", "-sha256", "-key", "key.pem", "-out", "csr.csr", "-subj", "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com"], certs_dir)
out = run(["openssl", "req", "-x509", "-sha256", "-days", "365", "-key", "key.pem", "-in", "csr.csr", "-outform", "der", "-out", "certificate.der", "-subj", "/C=US/ST=Denial/L=Springfield/O=Dis/CN=www.example.com"], certs_dir)

# 3. verify certificate signature
print("[3/6] Verifying certificate")
out = run(["openssl", "req", "-in", "csr.csr", "-text", "-noout"], certs_dir)

algo = re.search("Signature Algorithm: ([\w-]+)", out)

if algo.group(1) != "ecdsa-with-SHA256":
    print("Certificate signature algorithm is not correct. Should by 'ecdsa-with-SHA256' but was '%s'" % algo.group(1))
    print("This is probably due to an outdated OpenSSL version...")
    sys.exit(1)

# 4. extracting fingerprint
print("[4/6] Generating update_default_resources.c")

fingerprint = run(["openssl", "x509", "-inform", "der", "-in", "certificate.der", "-noout", "-fingerprint", "-sha256"], certs_dir)
fingerprint_bytes = list(map(lambda x: "0x" + x, fingerprint.strip().split("=")[1].split(":")))

certificate_der = run(["xxd", "-i", "certificate.der"], certs_dir).replace("certificate_der", "arm_uc_default_certificate").replace("_len", "_size")

vendor_uuid = uuid.uuid5(uuid.NAMESPACE_DNS, args.vendor_domain)
class_uuid = uuid.uuid5(vendor_uuid, args.device_model)
if args.device_id:
    device_uuid = uuid.uuid5(class_uuid, args.device_id)
else:
    device_uuid = uuid.uuid4()

vendor_id = struct.unpack("BBBBBBBBBBBBBBBB", vendor_uuid.bytes)
class_id = struct.unpack("BBBBBBBBBBBBBBBB", class_uuid.bytes)
device_id = struct.unpack("BBBBBBBBBBBBBBBB", device_uuid.bytes)

update_default_resources = """
#ifdef MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#include MBED_CLOUD_CLIENT_USER_CONFIG_FILE
#endif

#include <stdint.h>

#ifdef MBED_CLOUD_CLIENT_UPDATE_ID
const uint8_t arm_uc_vendor_id[16] = { %s };
const uint16_t arm_uc_vendor_id_size = sizeof(arm_uc_vendor_id);

const uint8_t arm_uc_class_id[16]  = { %s };
const uint16_t arm_uc_class_id_size = sizeof(arm_uc_class_id);

const uint8_t arm_uc_device_id[16] = { %s };
const uint16_t arm_uc_device_id_size = sizeof(arm_uc_device_id);
#endif

#ifdef MBED_CLOUD_CLIENT_UPDATE_CERT
const uint8_t arm_uc_default_fingerprint[] = { %s };
const uint16_t arm_uc_default_fingerprint_size =
    sizeof(arm_uc_default_fingerprint);

%s
#endif
""" % (
    ", ".join(list(map(lambda x: ("0x%02x" % x), vendor_id))),
    ", ".join(list(map(lambda x: ("0x%02x" % x), class_id))),
    ", ".join(list(map(lambda x: ("0x%02x" % x), device_id))),
    ", ".join(fingerprint_bytes),
    certificate_der
)

with open(update_cert_file, 'w') as f:
    f.write(update_default_resources)

# 5. generate base manifest
print("[5/6] Generating base manifest")

base_manifest = """{
    "encryptionMode": "none-ecc-secp256r1-sha256",
    "vendorId": "%s",
    "classId":  "%s",
    "certificates": [
        { "file": "certificate.der" }
    ]
}""" % (
    "".join(list(map(lambda x: ("%02x" % x), vendor_id))),
    "".join(list(map(lambda x: ("%02x" % x), class_id))),
)

with open(os.path.join(certs_dir, "manifest-base.json"), 'w') as f:
    f.write(base_manifest)

# 6. OK!
print("[6/6] Finished")
