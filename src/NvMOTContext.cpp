#include "Tracker.h"
#include "BYTETracker.h"
#include <fstream>

NvMOTContext::NvMOTContext(const NvMOTConfig &configIn, NvMOTConfigResponse &configResponse) {
    configResponse.summaryStatus = NvMOTConfigStatus_OK;
}

NvMOTStatus NvMOTContext::processFrame(const NvMOTProcessParams *params, NvMOTTrackedObjBatch *pTrackedObjectsBatch) {
    // FIX: Null checks for safety
    if (!params || !pTrackedObjectsBatch || !pTrackedObjectsBatch->list || !params->frameList) {
        return NvMOTStatus_Error;
    }

    for (uint streamIdx = 0; streamIdx < pTrackedObjectsBatch->numFilled; streamIdx++){
        NvMOTTrackedObjList   *trackedObjList = &pTrackedObjectsBatch->list[streamIdx];
        NvMOTFrame            *frame          = &params->frameList[streamIdx];
        // FIX: reserve() instead of size constructor to avoid duplicating elements with push_back
        std::vector<NvObject> nvObjects;
        nvObjects.reserve(frame->objectsIn.numFilled);
        for (uint32_t numObjects = 0; numObjects < frame->objectsIn.numFilled; numObjects++) {
            NvMOTObjToTrack *objectToTrack = &frame->objectsIn.list[numObjects];
            NvObject nvObject;
            nvObject.prob    = objectToTrack->confidence;
            nvObject.label   = objectToTrack->classId;
            nvObject.rect[0] = objectToTrack->bbox.x;
            nvObject.rect[1] = objectToTrack->bbox.y;
            nvObject.rect[2] = objectToTrack->bbox.width;
            nvObject.rect[3] = objectToTrack->bbox.height;
            nvObject.associatedObjectIn = objectToTrack;
            nvObjects.push_back(nvObject);
        }

        // Single-stream: create tracker on first use
        if (!byteTracker) {
            byteTracker = std::make_shared<BYTETracker>(15, 30);
        }

        std::vector<STrack> outputTracks = byteTracker->update(nvObjects);

        // FIX: Reuse existing buffer instead of allocating new one each frame
        if (trackedObjList->numAllocated != MAX_TARGETS_PER_STREAM) {
            delete[] trackedObjList->list;
            trackedObjList->list = new NvMOTTrackedObj[MAX_TARGETS_PER_STREAM];
            trackedObjList->numAllocated = MAX_TARGETS_PER_STREAM;
        }

        NvMOTTrackedObj *trackedObjs = trackedObjList->list;
        int filled = 0;

        for (STrack &sTrack: outputTracks) {
            if (filled >= MAX_TARGETS_PER_STREAM)
                break;
            std::vector<float> tlwh = sTrack.original_tlwh;
            NvMOTRect motRect{tlwh[0], tlwh[1], tlwh[2], tlwh[3]};
            // FIX: Use pointer to existing array element instead of new allocation
            NvMOTTrackedObj *trackedObj = &trackedObjs[filled];
            trackedObj->classId = 0;
            trackedObj->trackingId = (uint64_t) sTrack.track_id;
            trackedObj->bbox = motRect;
            trackedObj->confidence = 1;
            trackedObj->age = (uint32_t) sTrack.tracklet_len;
            trackedObj->associatedObjectIn = sTrack.associatedObjectIn;
            // FIX: Null check before dereference
            if (trackedObj->associatedObjectIn) {
                trackedObj->associatedObjectIn->doTracking = true;
            }
            filled++;
        }

        trackedObjList->streamID = frame->streamID;
        trackedObjList->frameNum = frame->frameNum;
        trackedObjList->valid = true;
        trackedObjList->numFilled = filled;
    }
    return NvMOTStatus_OK;
}

NvMOTStatus NvMOTContext::processFramePast(const NvMOTProcessParams *params,
                                           NvDsPastFrameObjBatch *pPastFrameObjectsBatch) {
    return NvMOTStatus_OK;
}

NvMOTStatus NvMOTContext::removeStream(const NvMOTStreamId streamIdMask) {
    std::cout << "Removing tracker for stream: " << streamIdMask << std::endl;
    byteTracker.reset();
    return NvMOTStatus_OK;
}
