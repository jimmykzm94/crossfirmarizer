import os
import re
import sys

# Fetch the built-in CircleCI build number
build_num = os.environ.get('CIRCLE_BUILD_NUM')

if not build_num:
    print("Not running in CircleCI or CIRCLE_BUILD_NUM is missing. Exiting.")
    sys.exit(1)

# --- CONFIGURATION ---
# Update these paths to match your repository structure
C_FILE_PATH = "crossfirmarizer/Core/Inc/version.h" 
TOML_FILE_PATH = "crossfirmarizer_client_python/pyproject.toml"

# --- 1. UPDATE C HEADER ---
with open(C_FILE_PATH, 'r') as f:
    c_data = f.read()

# Replaces: #define VERSION_BUILD <any_number>
c_data = re.sub(
    r'(#define\s+VERSION_BUILD\s+)\d+', 
    rf'\g<1>{build_num}', 
    c_data
)

with open(C_FILE_PATH, 'w') as f:
    f.write(c_data)

# --- 2. UPDATE TOML ---
with open(TOML_FILE_PATH, 'r') as f:
    toml_data = f.read()

# Replaces: version = "X.Y.<any_number>"
toml_data = re.sub(
    r'(version\s*=\s*"\d+\.\d+\.)\d+(")', 
    rf'\g<1>{build_num}\g<2>', 
    toml_data
)

with open(TOML_FILE_PATH, 'w') as f:
    f.write(toml_data)

print(f"Successfully bumped build version to {build_num}.")