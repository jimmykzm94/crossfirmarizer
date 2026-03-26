from behave import given, when, then
import subprocess
import os

TEST_PATH = os.path.dirname(os.path.abspath(__file__)) + "/../"
# PACKET_PATH = os.path.dirname(os.path.abspath(__file__)) + "/../serial_packet/"

APPLICATION = {
        "C": "run_c",
        "Python": "run_py"
    }

@given('the buffer is "{data}"')
def step_given_payload(context, data):
    context.data = data
    context.results = {}

@when('I {mode} in "{language}"')
def step_when_encode(context, mode, language):
    try:
        make_command = ["make", 
                        "-f", 
                        f"{TEST_PATH}/serial_packet/crossfirmarizer_serial_test.mk",
                        f"{APPLICATION[language]}",
                        f"MODE={mode}",
                        f"DATA={context.data}"
                        ]

        result = subprocess.run(make_command, check=True, capture_output=True, text=True, timeout=5)
        context.results[language] = result.stdout.strip()
    except Exception as e:
        print("Error:\n", e.stdout, e.stderr)
        raise

@then('the processed buffer must be "{data}"')
def step_then_encoded_buffer(context, data):
    for language, result in context.results.items():
        assert result == data, f"Expected {data}, got {result} in {language}"
