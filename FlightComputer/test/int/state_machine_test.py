import time
from enum import Enum
from pathlib import Path

from fprime_gds.common.testing_fw import predicates


def test_is_pinging(fprime_test_api):
    """Test flight software is up and running

    Tests that the flight software is streaming by looking for 5 telemetry items in 10 seconds. Additionally,
    "FlightComputer.sendBuffComp.SendState" is verified to be SEND_IDLE.
    """
    results = fprime_test_api.assert_telemetry_count(5, timeout=10)
    for result in results:
        msg = "received channel {} update: {}".format(result.get_id(), result.get_str())
        print(msg)
    fprime_test_api.assert_telemetry(
        "FlightComputer.pingRcvr.PR_NumPings", timeout=3
    )

def assert_and_wait_for_condition(fprime_test_api, channel, condition, assert_timeout, condition_timeout):
    """
    Asserts that telemetry is received within assert_timeout, then waits for a condition to be true within condition_timeout.

    :param fprime_test_api: The fprime test API object
    :param channel: The telemetry channel to monitor
    :param condition: A lambda function that takes the telemetry value and returns a boolean
    :param assert_timeout: Timeout for receiving initial telemetry (in seconds)
    :param condition_timeout: Timeout for the condition to become true (in seconds)
    :return: The telemetry value that satisfied the condition
    """

    # Now, wait for the condition to be true, checking periodically
    start_time = time.time()
    tlm_time = None
    while time.time() - start_time < condition_timeout:
        tlm = fprime_test_api.assert_telemetry(channel, time_pred=tlm_time, timeout=assert_timeout)
        tlm_time = tlm.time
        fprime_test_api.log(f"Received initial telemetry: {tlm.get_val()} @ {tlm.time}")
        fprime_test_api.clear_histories()
        if tlm:
            telemetry_value = tlm.get_val()
            if condition(telemetry_value):
                fprime_test_api.log(f"Condition met: {telemetry_value}")
                return telemetry_value
            else:
                fprime_test_api.log(f"Condition not yet met: {telemetry_value}")
        time.sleep(0.1)  # Wait a short time before checking again

    # If we've reached here, the condition wasn't met within the timeout
    fprime_test_api.log(f"Condition not met within {condition_timeout} seconds")
    raise AssertionError(f"Condition not met within {condition_timeout} seconds")

def test_flight_state_machine(fprime_test_api):
    """Tests that the flight state machine transitions correctly"""

    repeat_num = 1
    test_repeat_delay_s = 0.5

    for i in range(repeat_num):
        # Verify initial condition
        assert_and_wait_for_condition(
            fprime_test_api,
            "FlightComputer.flightSequencer.flightStatus",
            lambda x: not x['isEngineOn'] and x['currentState'] == 'IDLE',
            assert_timeout=5,
            condition_timeout=10
        )

        # Send IGNITE signal
        # fprime_test_api.send_command("FlightComputer.flightSequencer.IGNITE")
        fprime_test_api.send_and_await_event(
            "FlightComputer.flightSequencer.IGNITE", timeout=5
        )

        # Validate desired conditions after IGNITE
        fprime_test_api.log(f"Validate desired conditions after IGNITE")
        assert_and_wait_for_condition(
            fprime_test_api,
            "FlightComputer.flightSequencer.flightStatus",
            lambda x: x['isEngineOn'] and x['currentState'] == 'FIRING',
            assert_timeout=3,
            condition_timeout=10
        )

        # Monitor until GLIDING state
        fprime_test_api.log(f"Monitor until GLIDING state")
        assert_and_wait_for_condition(
            fprime_test_api,
            "FlightComputer.flightSequencer.flightStatus",
            lambda x: x['currentState'] == 'GLIDING',
            assert_timeout=3,
            condition_timeout=10
        )

        # Monitor until IDLE state and altitude below 0
        fprime_test_api.log(f"Monitor until IDLE state and altitude below 0")
        final_state = assert_and_wait_for_condition(
            fprime_test_api,
            "FlightComputer.flightSequencer.flightStatus",
            lambda x: x['currentState'] == 'IDLE' and x['altitudeM'] < 0,
            assert_timeout=3,
            condition_timeout=30
        )

        fprime_test_api.log(f"Flight cycle {i+1} completed. Final state: {final_state}")
        time.sleep(test_repeat_delay_s)

    fprime_test_api.log("All flight cycles completed successfully.")
