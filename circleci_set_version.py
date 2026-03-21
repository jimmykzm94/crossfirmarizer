import os
import re
import sys
import subprocess

# Get build number based on commit count of release branch that is branching out from main
branch_name = os.environ.get('CIRCLE_BRANCH')
if not branch_name:
    branch_name = "HEAD"  # Fallback for local testing

try:
    build_num = subprocess.check_output(["git", "rev-list", "--count", branch_name, "^main"]).decode("utf-8").strip()
except subprocess.CalledProcessError as e:
    print(f"Warning: Failed to calculate commit count, defaulting to 0. Error: {e}")
    build_num = "0"

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