from behave import given, when, then
import subprocess
import os

# FIXME the IO push everything 128 pins to output csv if can push only if being set thru pin mode

TEST_PATH = os.path.dirname(os.path.abspath(__file__)) + "/../"
BUILD_PATH = os.path.dirname(os.path.abspath(__file__)) + "/../build/"

PIN_MODE = {
    "INPUT": 0,
    "OUTPUT": 1,
    "ANALOG": 2,
    "INPUT_PULLUP": 3,
    "INPUT_PULLDOWN": 4
}

@given('the firmware simulation is running for "{mcu}"')
def step_given_firmware_running(context, mcu):
    context.mcu = mcu
    context.input_csv = "test_input.csv"
    context.output_csv = "test_output.csv"
    context.commands = []
    context.outputs = []

    if os.path.exists(context.input_csv):
        os.remove(context.input_csv)
    if os.path.exists(context.output_csv):
        os.remove(context.output_csv)

    try:
        subprocess.run(["make", "-f", f"{TEST_PATH}crossfirmarizer_test.mk", "build"], check=True)
    except subprocess.CalledProcessError as e:
        print("Error during make build:\n", e.stdout, e.stderr)
        raise

@given('set command with pin "{pin}" and mode "{mode}"')
def step_given_set_pin_mode(context, pin, mode):
    mode_val = PIN_MODE.get(mode.upper(), -1)
    context.commands.append(f"CMD_PIN_MODE,pin={pin},mode={mode_val}")

@given('set command with pin "{pin}" and value "{value}"')
def step_given_set_pin_value(context, pin, value):
    context.commands.append(f"CMD_SETTER,pin={pin},value={value}")

@when('I run the simulation')
def step_when_run_simulation(context):
    # 1. Set input file
    with open(BUILD_PATH + context.input_csv, 'w') as f:
        for cmd in context.commands:
            f.write(f"{cmd}\n")
    print(f"Created '{context.input_csv}' with commands:\n" + "\n".join(context.commands))

    # 2. Run simulation
    main_program = [f"{BUILD_PATH}crossfirmarizer_simulation", BUILD_PATH]
    print(f"Running build command: {' '.join(main_program)}")
    
    try:
        result = subprocess.run(main_program, check=True, capture_output=True, text=True, timeout=5)
    except subprocess.CalledProcessError as e:
        print("Error during make build:\n", e.stdout, e.stderr)
        raise
    print(f"Running simulation")
    print(result.stdout)
    print(result.stderr)

    # 3. Get values from simulation
    with open(BUILD_PATH + context.output_csv, 'r') as f:
        for line in f.readlines():
            context.outputs.append(line.strip())

@then('pin IO "{pin}" should be {value}')
def step_then_pin_state(context, pin, value):
    # Read IO
    data = context.outputs[int(pin)].split(",")
    expected_pin = int(pin)
    expected_value = int(value)
    actual_pin = int(data[1].strip('pin='))
    actual_value= int(data[2].strip('value='))
    assert expected_pin == actual_pin, f"Expected pin {expected_pin}, got {actual_pin}"
    assert expected_value == actual_value, f"Expected state {expected_value}, got {actual_value}"

@given('pin IO "{pin}" is "{value}"')
def step_given_pin_io(context, pin, value):
    context.commands.append(f"IO,pin={pin},value={value}")

@given('get command with pin "{pin}"')
def step_given_getter(context, pin):
    context.commands.append(f"CMD_GETTER,pin={pin}")

@then('getter response with pin "{pin}" should be {value}')
def step_then_response_with_pin(context, pin, value):
    # FIXME the index is not quite right, I only assume it will be at last line
    data = context.outputs[-1].split(",")
    expected_pin = int(pin)
    expected_value = int(value)
    actual_pin = int(data[1].strip('pin='))
    actual_value = int(data[2].strip('value='))
    assert actual_pin == expected_pin, f"Expected pin {expected_pin}, got {actual_pin}"
    assert actual_value == expected_value, f"Expected value {expected_value}, got {actual_value}"
