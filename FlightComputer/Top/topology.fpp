module FlightComputer {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

    enum Ports_RateGroups {
      rateGroup1
      rateGroup2
      rateGroup3
    }

    enum Ports_StaticMemory {
      gdsDownlink
      uplink
    }

  topology FlightComputer {

    # ----------------------------------------------------------------------
    # Instances used in the topology
    # ----------------------------------------------------------------------

    instance $health
    instance blockDrv
    instance gdsChanTlm
    instance cmdDisp
    instance cmdSeq
    instance comm
    instance gdsDownlink
    instance eventLogger
    instance fatalAdapter
    instance fatalHandler
    instance fileDownlink
    instance fileManager
    instance fileUplink
    instance fileUplinkBufferManager
    instance posixTime
    instance pingRcvr
    instance prmDb
    instance rateGroup1Comp
    instance rateGroup2Comp
    instance rateGroup3Comp
    instance rateGroupDriverComp
    instance staticMemory
    instance textLogger
    instance uplink
    instance systemResources
    instance flightSequencer
    instance version

    # ----------------------------------------------------------------------
    # Pattern graph specifiers
    # ----------------------------------------------------------------------

    command connections instance cmdDisp

    event connections instance eventLogger

    param connections instance prmDb

    telemetry connections instance gdsChanTlm

    text event connections instance textLogger

    time connections instance posixTime

    health connections instance $health

    # ----------------------------------------------------------------------
    # Direct graph specifiers
    # ----------------------------------------------------------------------

    connections GDSDownlink {

      gdsChanTlm.PktSend -> gdsDownlink.comIn
      eventLogger.PktSend -> gdsDownlink.comIn

      gdsDownlink.framedAllocate -> staticMemory.bufferAllocate[Ports_StaticMemory.gdsDownlink]
      gdsDownlink.framedOut -> comm.$send
      comm.deallocate -> staticMemory.bufferDeallocate[Ports_StaticMemory.gdsDownlink]

      fileDownlink.bufferSendOut -> gdsDownlink.bufferIn
      gdsDownlink.bufferDeallocate -> fileDownlink.bufferReturn

    }

    connections FaultProtection {
      eventLogger.FatalAnnounce -> fatalHandler.FatalReceive
    }

    connections RateGroups {

      # Block driver
      blockDrv.CycleOut -> rateGroupDriverComp.CycleIn

      # Rate group 1 (100Hz)
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup1] -> rateGroup1Comp.CycleIn

      # Rate group 2 (1Hz)
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup2] -> rateGroup2Comp.CycleIn
      rateGroup2Comp.RateGroupMemberOut[0] -> cmdSeq.schedIn
      rateGroup2Comp.RateGroupMemberOut[1] -> gdsChanTlm.Run
      rateGroup2Comp.RateGroupMemberOut[2] -> fileDownlink.Run
      rateGroup2Comp.RateGroupMemberOut[3] -> systemResources.run
      rateGroup2Comp.RateGroupMemberOut[4] -> blockDrv.Sched
      rateGroup2Comp.RateGroupMemberOut[5] -> fileUplinkBufferManager.schedIn
      rateGroup2Comp.RateGroupMemberOut[6] -> $health.Run

      # Rate group 3 (10Hz)
      rateGroupDriverComp.CycleOut[Ports_RateGroups.rateGroup3] -> rateGroup3Comp.CycleIn
      rateGroup1Comp.RateGroupMemberOut[0] -> flightSequencer.run
    }

    connections Uplink {

      comm.allocate -> staticMemory.bufferAllocate[Ports_StaticMemory.uplink]
      comm.$recv -> uplink.framedIn
      uplink.framedDeallocate -> staticMemory.bufferDeallocate[Ports_StaticMemory.uplink]

      uplink.comOut -> cmdDisp.seqCmdBuff
      cmdDisp.seqCmdStatus -> uplink.cmdResponseIn

      uplink.bufferAllocate -> fileUplinkBufferManager.bufferGetCallee
      uplink.bufferOut -> fileUplink.bufferSendIn
      uplink.bufferDeallocate -> fileUplinkBufferManager.bufferSendIn
      fileUplink.bufferSendOut -> fileUplinkBufferManager.bufferSendIn
    }
  }
}
