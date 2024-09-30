import time
from enum import Enum
from pathlib import Path

from fprime_gds.common.testing_fw import predicates
from fprime_gds.common.utils.event_severity import EventSeverity


"""
This enum is includes the values of EventSeverity that can be filtered by the ActiveLogger Component
"""
FilterSeverity = Enum(
    "FilterSeverity",
    "WARNING_HI WARNING_LO COMMAND ACTIVITY_HI ACTIVITY_LO DIAGNOSTIC",
)

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

# def test_bd_cycles_ascending(fprime_test_api):
#     """Test in-order block driver updates"""
#     length = 10
#     count_pred = predicates.greater_than(length - 1)
#     results = fprime_test_api.await_telemetry_count(
#         count_pred, "FlightComputer.blockDrv.BD_Cycles", timeout=length
#     )
#     last = None
#     reordered = False
#     ascending = True
#     for result in results:
#         if last is not None:
#             last_time = last.get_time()
#             result_time = result.get_time()
#             if result_time - last_time > 1.5:
#                 msg = "FSW didn't send an update between {} and {}".format(
#                     last_time.to_readable(), result_time.to_readable()
#                 )
#                 fprime_test_api.log(msg)
#             elif result_time < last_time:
#                 msg = "There is potential reorder error between {} and {}".format(
#                     last_time, result_time
#                 )
#                 fprime_test_api.log(msg)
#                 reordered = True

#             if not result.get_val() > last.get_val():
#                 msg = "Not all updates ascended: First ({}) Second ({})".format(
#                     last.get_val(), result.get_val()
#                 )
#                 fprime_test_api.log(msg)
#                 ascending = False

#         last = result

#     case = True
#     case &= fprime_test_api.test_assert(
#         ascending, "Expected all updates to ascend.", True
#     )
#     case &= fprime_test_api.test_assert(
#         not reordered, "Expected no updates to be dropped.", True
#     )
#     fprime_test_api.predicate_assert(
#         count_pred,
#         len(results) - 1,
#         "Expected >= {} updates".format(length - 1),
#         True,
#     )
#     fprime_test_api.assert_telemetry_count(0, "rateGroup1Comp.RgCycleSlips")
#     assert case, "Expected all checks to pass (ascending, reordering). See log."

def test_flight_state(fprime_test_api):
    """Test flight software is up and running

    Tests that the flight software is streaming by looking for 5 telemetry items in 10 seconds. Additionally,
    "FlightComputer.sendBuffComp.SendState" is verified to be SEND_IDLE.
    """
    """Test that commands may be sent

    Tests command send, dispatch, and receipt using send_and_assert command with a pair of NO-OP commands.
    """
    length = 10
    any_reordered = False
    dropped = False
    evr_seq = [
        "FlightComputer.cmdDisp.OpCodeDispatched",
        # "FlightComputer.cmdDisp.NoOpReceived",
        "FlightComputer.cmdDisp.OpCodeCompleted",
    ]
    for i in range(0, length):
        time.sleep(0.5)
        results = fprime_test_api.send_and_await_event(
            "FlightComputer.flightSequencer.IGNITE", timeout=5
        )
        msg = "Send and assert IGNITE Trial #{}".format(i)
        fprime_test_api.log("Results {}".format(results))
        fprime_test_api.assert_telemetry(
                "FlightComputer.FlightSequencer.status", timeout=3
        )

        pred = fprime_test_api.get_telemetry_pred("FlightComputer.FlightSequencer.status")
        fprime_test_api.log("pred found {}".format(pred))
        items = fprime_test_api.get_telemetry_test_history().retrieve_new()
        last = None
        reordered = False
        for item in items:
                fprime_test_api.log("items found {}".format(item))

        fprime_test_api.clear_histories()
