# Set the path to your header file
import os 
WORKIN_DIR = os.path.abspath(os.path.join(os.getcwd(), "..", "..", ".."))
VERSION_HEADER = WORKIN_DIR+ '\\include\\adapter_config.h'

# Set the path to your manifest.json
MANIFEST_JSON = WORKIN_DIR+'\\manifest.json'

# Read the FW version from the header file
with open(VERSION_HEADER, 'r') as version_file:
    version_content = version_file.read()

# Extract the version number from the file content
import re
match = re.search(r"ADAPTER_FIRMWARE_VERSION\s*0x([0-9A-Fa-f]+)", version_content)
if not match:
    raise ValueError(f"Version not found in {VERSION_HEADER}")

version_hex_upper = match.group(1).upper()
version_int = int(version_hex_upper, 16)

# Read the existing JSON data from manifest.json
with open(MANIFEST_JSON, 'r') as json_file:
    manifest_json_content = json_file.read()

# Find and replace the "fw_version" key in JSON content
import json
manifest_json = json.loads(manifest_json_content)
manifest_json["fw_version"] = version_int

# Write the updated JSON content back to manifest.json
with open(MANIFEST_JSON, 'w') as json_file:
    json.dump(manifest_json, json_file, indent=2)

# Print the extracted version
print(f"Extracted version: {version_int}")