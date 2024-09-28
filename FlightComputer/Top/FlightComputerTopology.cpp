#include "FlightComputer/Top/FppConstantsAc.hpp"
#include "Fw/Logger/Logger.hpp"

// Provides access to autocoded functions
#include <FlightComputer/Top/FlightComputerTopologyAc.hpp>
#include <FlightComputer/Top/FlightComputerTopologyDefs.hpp>
#include <FlightComputer/Top/FlightComputerTopology.hpp>

// Necessary project-specified types
#include <Fw/Types/MallocAllocator.hpp>
#include <Os/Console.hpp>
#include <Svc/FramingProtocol/FprimeProtocol.hpp>

// Used for 1Hz synthetic cycling
#include <Os/Mutex.hpp>
#include <cstdio>
#include <cstring>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace FlightComputer;

// Instantiate a system logger that will handle Fw::Logger::log calls
Os::Console logger;

// The reference topology uses a malloc-based allocator for components that need to allocate memory during the
// initialization phase.
Fw::MallocAllocator mallocator;

// The reference topology uses the F´ packet protocol when communicating with the ground and therefore uses the F´
// framing and deframing implementations.
Svc::FprimeFraming gdsFraming;
Svc::FprimeDeframing deframing;

// The reference topology divides the incoming clock signal (1Hz) into sub-signals: 1Hz, 1/2Hz, and 1/4Hz and
// zero offset for all the dividers
Svc::RateGroupDriver::DividerSet rateGroupDivisorsSet{{{1, 0}, {100, 0}, {10, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
NATIVE_INT_TYPE rateGroup1Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
NATIVE_INT_TYPE rateGroup2Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};
NATIVE_INT_TYPE rateGroup3Context[Svc::ActiveRateGroup::CONNECTION_COUNT_MAX] = {};

// A number of constants are needed for construction of the topology. These are specified here.
enum TopologyConstants {
    CMD_SEQ_BUFFER_SIZE = 5 * 1024,
    FILE_DOWNLINK_TIMEOUT = 1000,
    FILE_DOWNLINK_COOLDOWN = 1000,
    FILE_DOWNLINK_CYCLE_TIME = 1000,
    FILE_DOWNLINK_FILE_QUEUE_DEPTH = 10,
    HEALTH_WATCHDOG_CODE = 0x123,
    COMM_PRIORITY = 100,
    UPLINK_BUFFER_MANAGER_STORE_SIZE = 3000,
    UPLINK_BUFFER_MANAGER_QUEUE_SIZE = 30,
    UPLINK_BUFFER_MANAGER_ID = 200,
    DP_BUFFER_MANAGER_STORE_SIZE = 10000,
    DP_BUFFER_MANAGER_QUEUE_SIZE = 10,
    DP_BUFFER_MANAGER_ID = 300
};

// Ping entries are autocoded, however; this code is not properly exported. Thus, it is copied here.
Svc::Health::PingEntry pingEntries[] = {
    {PingEntries::FlightComputer_blockDrv::WARN, PingEntries::FlightComputer_blockDrv::FATAL, "blockDrv"},
    {PingEntries::FlightComputer_chanTlm::WARN, PingEntries::FlightComputer_chanTlm::FATAL, "chanTlm"},
    {PingEntries::FlightComputer_cmdDisp::WARN, PingEntries::FlightComputer_cmdDisp::FATAL, "cmdDisp"},
    {PingEntries::FlightComputer_cmdSeq::WARN, PingEntries::FlightComputer_cmdSeq::FATAL, "cmdSeq"},
    {PingEntries::FlightComputer_eventLogger::WARN, PingEntries::FlightComputer_eventLogger::FATAL, "eventLogger"},
    {PingEntries::FlightComputer_fileDownlink::WARN, PingEntries::FlightComputer_fileDownlink::FATAL, "fileDownlink"},
    {PingEntries::FlightComputer_fileManager::WARN, PingEntries::FlightComputer_fileManager::FATAL, "fileManager"},
    {PingEntries::FlightComputer_fileUplink::WARN, PingEntries::FlightComputer_fileUplink::FATAL, "fileUplink"},
    {PingEntries::FlightComputer_pingRcvr::WARN, PingEntries::FlightComputer_pingRcvr::FATAL, "pingRcvr"},
    {PingEntries::FlightComputer_prmDb::WARN, PingEntries::FlightComputer_prmDb::FATAL, "prmDb"},
    {PingEntries::FlightComputer_rateGroup1Comp::WARN, PingEntries::FlightComputer_rateGroup1Comp::FATAL, "rateGroup1Comp"},
    {PingEntries::FlightComputer_rateGroup2Comp::WARN, PingEntries::FlightComputer_rateGroup2Comp::FATAL, "rateGroup2Comp"},
    {PingEntries::FlightComputer_rateGroup3Comp::WARN, PingEntries::FlightComputer_rateGroup3Comp::FATAL, "rateGroup3Comp"},
};

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology() {
    // Command sequencer needs to allocate memory to hold contents of command sequences
    cmdSeq.allocateBuffer(0, mallocator, CMD_SEQ_BUFFER_SIZE);

    // Rate group driver needs a divisor list
    rateGroupDriverComp.configure(rateGroupDivisorsSet);

    // Rate groups require context arrays. Empty for FlightComputererence example.
    rateGroup1Comp.configure(rateGroup1Context, FW_NUM_ARRAY_ELEMENTS(rateGroup1Context));
    rateGroup2Comp.configure(rateGroup2Context, FW_NUM_ARRAY_ELEMENTS(rateGroup2Context));
    rateGroup3Comp.configure(rateGroup3Context, FW_NUM_ARRAY_ELEMENTS(rateGroup3Context));

    // File downlink requires some project-derived properties.
    fileDownlink.configure(FILE_DOWNLINK_TIMEOUT, FILE_DOWNLINK_COOLDOWN, FILE_DOWNLINK_CYCLE_TIME,
                           FILE_DOWNLINK_FILE_QUEUE_DEPTH);

    // Parameter database is configured with a database file name, and that file must be initially read.
    // prmDb.configure("PrmDb.dat");
    // prmDb.readParamFile();

    // Health is supplied a set of ping entires.
    health.setPingEntries(pingEntries, FW_NUM_ARRAY_ELEMENTS(pingEntries), HEALTH_WATCHDOG_CODE);

    // Buffer managers need a configured set of buckets and an allocator used to allocate memory for those buckets.
    Svc::BufferManager::BufferBins upBuffMgrBins;
    memset(&upBuffMgrBins, 0, sizeof(upBuffMgrBins));
    upBuffMgrBins.bins[0].bufferSize = UPLINK_BUFFER_MANAGER_STORE_SIZE;
    upBuffMgrBins.bins[0].numBuffers = UPLINK_BUFFER_MANAGER_QUEUE_SIZE;
    fileUplinkBufferManager.setup(UPLINK_BUFFER_MANAGER_ID, 0, mallocator, upBuffMgrBins);

    // Framer and Deframer components need to be passed a protocol handler
    gdsDownlink.setup(gdsFraming);
    uplink.setup(deframing);

    // Fw::FileNameString dpDir("./DpCat");

    // create the DP directory if it doesn't exist
    // Os::FileSystem::createDirectory(dpDir.toChar());

    // dpCat.configure(&dpDir,1,0,mallocator);
    // dpWriter.configure(dpDir);

    // Note: Uncomment when using Svc:TlmPacketizer
    //tlmSend.setPacketList(FlightComputerPacketsPkts, FlightComputerPacketsIgnore, 1);
}

// Public functions for use in main program are namespaced with deployment name FlightComputer
namespace FlightComputer {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded parameter loading. Function provided by autocoder.
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);
    // Startup TLM and Config verbosity for Versions
    version.config(true);
    // Initialize socket client communication if and only if there is a valid specification
    if (state.hostName != nullptr && state.uplinkPort != 0) {
        Os::TaskString name("ReceiveTask");
        // Uplink is configured for receive so a socket task is started
        comm.configure(state.hostName, state.uplinkPort);
        comm.start(name, true, COMM_PRIORITY, Default::stackSize);
    }

}

// Variables used for cycle simulation
Os::Mutex cycleLock;
volatile bool cycleFlag = true;

void startSimulatedCycle(Fw::Time interval) {
    cycleLock.lock();
    bool cycling = cycleFlag;
    cycleLock.unLock();

    // Main loop
    while (cycling) {
        FlightComputer::blockDrv.callIsr();
        Os::Task::delay(interval);

        cycleLock.lock();
        cycling = cycleFlag;
        cycleLock.unLock();
    }
}

void stopSimulatedCycle() {
    cycleLock.lock();
    cycleFlag = false;
    cycleLock.unLock();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);

    // Other task clean-up.
    comm.stop();
    (void)comm.join();

    // Resource deallocation
    cmdSeq.deallocateBuffer(mallocator);
    fileUplinkBufferManager.cleanup();
}
};  // namespace FlightComputer
