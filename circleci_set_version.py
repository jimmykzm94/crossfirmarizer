import os
import re
import crossfirmarizer_version

# Get version from crossfirmarizer_version.py
major_num = crossfirmarizer_version.VERSION_MAJOR
minor_num = crossfirmarizer_version.VERSION_MINOR

# Get build number based on pipeline number
build_num = os.environ.get('CIRCLE_PIPELINE_NUMBER')
if not build_num:
    build_num = "0"

# --- CONFIGURATION ---
# Update these paths to match your repository structure
C_FILE_PATH = "crossfirmarizer/Core/Inc/version.h"
TOML_FILE_PATH = "crossfirmarizer_client_python/pyproject.toml"

# --- 1. UPDATE C HEADER ---
with open(C_FILE_PATH, 'r') as f:
    c_data = f.read()

# Replaces: #define VERSION_MAJOR <any_number>
c_data = re.sub(
    r'(#define\s+VERSION_MAJOR\s+)\d+',
    rf'\g<1>{major_num}',
    c_data
)
# Replaces: #define VERSION_MINOR <any_number>
c_data = re.sub(
    r'(#define\s+VERSION_MINOR\s+)\d+',
    rf'\g<1>{minor_num}',
    c_data
)
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

# Replaces: version = "X.Y.Z"
toml_data = re.sub(
    r'(version\s*=\s*")\d+\.\d+\.\d+(")',
    rf'\g<1>{major_num}.{minor_num}.{build_num}\g<2>',
    toml_data
)

with open(TOML_FILE_PATH, 'w') as f:
    f.write(toml_data)
    
print(f"Successfully bumped version to {major_num}.{minor_num}.{build_num}.")