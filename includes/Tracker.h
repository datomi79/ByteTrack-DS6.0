//
// Created by Mayur Kulkarni on 11/11/21.
//

#ifndef DNSTARPROD_TRACKER_H
#define DNSTARPROD_TRACKER_H

#include "nvdstracker.h"
#include "BYTETracker.h"
#include <memory>
#include <map>

#define MAX_TARGETS_PER_STREAM 512

/**
* @brief Context for input video streams
*
* The stream context holds all necessary state to perform multi-object tracking
* within the stream. Simplified for single-stream operation.
*
*/
class NvMOTContext {
public:
    NvMOTContext(const NvMOTConfig &configIn, NvMOTConfigResponse &configResponse);

    ~NvMOTContext() {};

    /**
     * @brief Process a batch of frames
     *
     * Internal implementation of NvMOT_Process()
     *
     * @param [in] pParam Pointer to parameters for the frame to be processed
     * @param [out] pTrackedObjectsBatch Pointer to object tracks output
     */
    NvMOTStatus processFrame(const NvMOTProcessParams *params,
                             NvMOTTrackedObjBatch *pTrackedObjectsBatch);

    /**
     * @brief Output the past-frame data if there are
     *
     *  Internal implementation of NvMOT_ProcessPast()
     *
     * @param [in] pParam Pointer to parameters for the frame to be processed
     * @param [out] pPastFrameObjectsBatch Pointer to past frame object tracks output
     */
    NvMOTStatus processFramePast(const NvMOTProcessParams *params,
                                 NvDsPastFrameObjBatch *pPastFrameObjectsBatch);

    /**
     * @brief Terminate trackers and release resources for a stream when the stream is removed
     *
     *  Internal implementation of NvMOT_RemoveStreams()
     *
     * @param [in] streamIdMask removed stream ID
     */
    NvMOTStatus removeStream(const NvMOTStreamId streamIdMask);

protected:
    /**
     * Multi-stream tracker map: one BYTETracker instance per stream ID
     * Track IDs are globally unique across all streams via atomic counter
     */
    std::map<uint64_t, std::shared_ptr<BYTETracker>> byteTrackerMap;
};

#endif //DNSTARPROD_TRACKER_H
