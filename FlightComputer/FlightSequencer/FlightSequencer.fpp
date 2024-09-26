module FlightComputer {

  state machine FlightSM

  active component FlightSequencer {
    # Currently the generated F' code doesn't compile
    # add it when there are updates to F'
    # include "FlightSM.fpp"
    include "FlightSM.fppi"

    # state machine instance dev1: FlightSequencerSM priority 1 Drop
    # state machine instance dev2: FlightSequencerSM priority 2 Assert


    struct status {
        flightTimeS: F32
        isEngineOn: bool
        altitudeM: F32
        velocityMS: F32
        currentState: FlightSMStates
    }

    constant thrustN = 2000
    constant massKg = 100
    constant tBurnS = 30
    constant gravityMSS = 9.81

    # ----------------------------------------------------------------------
    # Special ports
    # ----------------------------------------------------------------------

    @ Command receive port
    command recv port CmdDisp

    @ Command registration port
    command reg port CmdReg

    @ Command response port
    command resp port CmdStatus

    @ Event
    event port eventOut

    @ Telemetry
    telemetry port tlmOut

    @ Port for getting the time necessary for the event and TM timestamps
    time get port Time

    @ Run port for running the simulation
    async input port run: Svc.Sched

    # ----------------------------------------------------------------------
    # Commands
    # ----------------------------------------------------------------------

    async command IGNITE
    async command TERMINATE

    # ----------------------------------------------------------------------
    # Telemetry
    # ----------------------------------------------------------------------

    telemetry flightStatus: status update on change
  }

}
