# ByteTrack Integration with DeepStream 6.x (Multi-Stream with Global Track IDs)

This is an enhanced version of ByteTrack for DeepStream 6.x with memory leak fixes and multi-camera support.

## Features
- Compatible with DeepStream 6.0 and newer versions
- **Multi-stream support** with independent tracking per camera
- **Global track IDs** across all streams (thread-safe atomic counter)
- **Memory leak fixes** from the original implementation
- Based on multi-stream architecture from ByteTrack-DS6.4

## Memory Leak Fixes Applied

1. **NvMOTContext::processFrame()** - Fixed output buffer management
   - Reuses existing buffer instead of allocating new one each frame
   - Uses direct pointer to array element instead of `new` for each track

2. **lapjv()** - Fixed allocation size bug
   - Removed incorrect `sizeof()` multiplier that allocated 8x excess memory

3. **BYTETracker::update()** - Added cleanup for removed_stracks
   - Prevents unbounded growth of removed tracks vector

## Multi-Stream Architecture

Each camera stream gets its own independent BYTETracker instance:
- Automatic tracker creation per stream ID on first frame
- Track IDs are globally unique across all streams (atomic counter)
- Independent tracking state per camera (no cross-stream matching)
- Automatic cleanup when stream is removed

**Stream Lifecycle:**
1. New stream detected → Creates dedicated BYTETracker instance
2. Each frame processed independently per stream
3. Stream removed → Tracker automatically cleaned up

**Global Track IDs:**
- Uses thread-safe `std::atomic<int>` counter
- Ensures no ID collisions between streams
- Monotonically increasing across all cameras

## Build Instructions
```bash
mkdir build && cd build
cmake ..
make ByteTracker
```

This creates `./lib/libByteTracker.so` which can be used as a custom low-level tracker library in DeepStream.

Copy to DeepStream lib folder:
```bash
cp lib/libByteTracker.so /opt/nvidia/deepstream/deepstream/lib/
```

## Configuration

In your `deepstream_app_config.txt`:
```ini
[tracker]
enable=1
tracker-width=640
tracker-height=384
gpu-id=0
ll-lib-file=/opt/nvidia/deepstream/deepstream/lib/libByteTracker.so
enable-batch-process=1
```

## Testing for Memory Leaks

```bash
valgrind --leak-check=full ./deepstream-app -c your_config.txt
```

Or monitor memory usage during long runs:
```bash
watch -n 1 'ps -o rss,vsz,pid,cmd -p $(pgrep deepstream)'
```

## References
1. [How to Implement a Custom Low-Level Tracker Library in DeepStream](https://docs.nvidia.com/metropolis/deepstream/dev-guide/text/DS_plugin_gst-nvtracker.html#how-to-implement-a-custom-low-level-tracker-library)
2. [ByteTrack](https://github.com/ifzhang/ByteTrack)
